#include "LibtropicSecureElement.hpp"

#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>

#include <cstring>

#include "libtropic.h"
#include "libtropic_common.h"
#include "libtropic_mbedtls_v4.h"
#include "libtropic_port_linux_spi.h"
#include "psa/crypto.h"

#define PING_MSG "ping from hsmctl"
#define PING_MSG_SIZE 17

const uint8_t lt_sh0priv_prod0[] = {
    0x28, 0x3f, 0x5a, 0x0f, 0xfc, 0x41, 0xcf, 0x50, 0x98, 0xa8, 0xe1, 0x7d, 0xb6, 0x37, 0x2c, 0x3c,
    0xaa, 0xd1, 0xee, 0xee, 0xdf, 0x0f, 0x75, 0xbc, 0x3f, 0xbf, 0xcd, 0x9c, 0xab, 0x3d, 0xe9, 0x72};

const uint8_t lt_sh0pub_prod0[] = {0xf9, 0x75, 0xeb, 0x3c, 0x2f, 0xd7, 0x90, 0xc9, 0x6f, 0x29, 0x4f,
                                   0x15, 0x57, 0xa5, 0x03, 0x17, 0x80, 0xc9, 0xaa, 0xfa, 0x14, 0x0d,
                                   0xa2, 0x8f, 0x55, 0xe7, 0x51, 0x57, 0x37, 0xb2, 0x50, 0x2c};

struct LibtropicSecureElement::Impl
{
    lt_handle_t handle = {0};
    lt_dev_linux_spi_t device = {0};
    lt_ctx_mbedtls_v4_t crypto_ctx = {0};
};

LibtropicSecureElement::LibtropicSecureElement() : m_impl(std::make_unique<Impl>())
{
}

LibtropicSecureElement::~LibtropicSecureElement() = default;

SystemStatus LibtropicSecureElement::init()
{
    if (psa_crypto_init() != PSA_SUCCESS)
    {
        return SystemStatus::HSM_ERROR_INIT;
    }

    snprintf(m_impl->device.spi_dev, sizeof(m_impl->device.spi_dev), "/dev/spidev0.0");
    snprintf(m_impl->device.gpio_dev, sizeof(m_impl->device.gpio_dev), "/dev/gpiochip0");
    m_impl->device.spi_speed = 5000000;
    m_impl->device.gpio_cs_num = 25;

    m_impl->handle.l2.device = &m_impl->device;
    m_impl->handle.l3.crypto_ctx = &m_impl->crypto_ctx;

    if (lt_init(&m_impl->handle) != LT_OK)
    {
        return SystemStatus::HSM_ERROR_INIT;
    }

    if (lt_reboot(&m_impl->handle, TR01_REBOOT) != LT_OK)
    {
        lt_deinit(&m_impl->handle);
        mbedtls_psa_crypto_free();
        return SystemStatus::HSM_ERROR_INIT;
    }

    if (lt_verify_chip_and_start_secure_session(&m_impl->handle, lt_sh0priv_prod0, lt_sh0pub_prod0,
                                                TR01_PAIRING_KEY_SLOT_INDEX_0) != LT_OK)
    {
        lt_deinit(&m_impl->handle);
        mbedtls_psa_crypto_free();
        return SystemStatus::HSM_ERROR_INIT;
    }

    return SystemStatus::OK;
}

SystemStatus LibtropicSecureElement::status()
{
    uint8_t recv_buf[PING_MSG_SIZE];

    if (lt_ping(&m_impl->handle, (const uint8_t*)PING_MSG, recv_buf, PING_MSG_SIZE) != LT_OK)
    {
        deinit();
        return SystemStatus::HSM_ERROR_STATUS;
    }

    return SystemStatus::OK;
}

SystemStatus LibtropicSecureElement::eraseKey(uint8_t slot)
{
    if (lt_ecc_key_erase(&m_impl->handle, static_cast<lt_ecc_slot_t>(slot)) != LT_OK)
    {
        deinit();
        return SystemStatus::HSM_ERROR_ERASE_KEY;
    }

    return SystemStatus::OK;
}

SystemStatus LibtropicSecureElement::generateKey(uint8_t slot, Curve curve)
{
    lt_ecc_curve_type_t lt_curve = (curve == Curve::P256) ? TR01_CURVE_P256 : TR01_CURVE_ED25519;

    lt_ret_t ret = lt_ecc_key_generate(&m_impl->handle, static_cast<lt_ecc_slot_t>(slot), lt_curve);

    if (ret != LT_OK)
    {
        deinit();
        return (ret == LT_L3_FAIL) ? SystemStatus::HSM_ERROR_GENERATE_KEY_SLOT_OCCUPIED
                                   : SystemStatus::HSM_ERROR_GENERATE_KEY_HW_ERROR;
    }

    return SystemStatus::OK;
}

SystemStatus LibtropicSecureElement::readKey(uint8_t slot, std::vector<uint8_t>& pubKey)
{
    uint8_t raw_key[64];
    lt_ecc_curve_type_t curve;
    lt_ecc_key_origin_t origin;

    lt_ret_t ret = lt_ecc_key_read(&m_impl->handle, static_cast<lt_ecc_slot_t>(slot), raw_key,
                                   sizeof(raw_key), &curve, &origin);

    if (ret != LT_OK)
    {
        deinit();
        return (ret == LT_L3_INVALID_KEY) ? SystemStatus::HSM_ERROR_READ_KEY_EMPTY_SLOT
                                          : SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR;
    }

    int key_len = (curve == TR01_CURVE_ED25519) ? 32 : 64;

    pubKey.assign(raw_key, raw_key + key_len);

    return SystemStatus::OK;
}

SystemStatus LibtropicSecureElement::listKeys(std::vector<std::vector<uint8_t>>& pubKeys)
{
    lt_ret_t ret;
    int key_len;
    pubKeys.resize(32);

    for (uint8_t i = 0; i < 32; i++)
    {
        uint8_t raw_key[64] = {};
        lt_ecc_curve_type_t curve;
        lt_ecc_key_origin_t origin;

        ret = lt_ecc_key_read(&m_impl->handle, static_cast<lt_ecc_slot_t>(i), raw_key,
                              sizeof(raw_key), &curve, &origin);

        if (ret != LT_OK)
        {
            if (ret == LT_L3_INVALID_KEY)
            {
                continue;
            }
            else
            {
                deinit();
                return SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR;
            }
        }

        key_len = (curve == TR01_CURVE_ED25519) ? 32 : 64;

        pubKeys.at(i).assign(raw_key, raw_key + key_len);
    }

    return SystemStatus::OK;
}

SystemStatus LibtropicSecureElement::sign(uint8_t slot, Curve curve, std::vector<uint8_t> payload,
                                          std::vector<uint8_t>& signature)
{
    SystemStatus result;

    if (curve == Curve::Ed25519)
    {
        result = signEd25519(slot, payload, signature);
    }
    else if (curve == Curve::P256)
    {
        result = signP256(slot, payload, signature);
    }
    else
    {
        assert(false && "Unhandled curve value in LibtropicSecureElement::sign()");
        return SystemStatus::HSM_ERROR_SIGN;
    }

    return result;
}

SystemStatus LibtropicSecureElement::signEd25519(uint8_t slot, std::vector<uint8_t> payload,
                                                 std::vector<uint8_t>& signature)
{
    uint8_t rs[64];

    if (lt_ecc_eddsa_sign(&m_impl->handle, static_cast<lt_ecc_slot_t>(slot), payload.data(),
                          payload.size(), rs) != LT_OK)
    {
        deinit();
        return SystemStatus::HSM_ERROR_SIGN;
    }

    signature.assign(rs, rs + 64);

    return SystemStatus::OK;
}

SystemStatus LibtropicSecureElement::signP256(uint8_t slot, std::vector<uint8_t> payload,
                                              std::vector<uint8_t>& signature)
{
    uint8_t hash[32];
    size_t hash_length;

    // Hash the payload first
    psa_status_t status = psa_hash_compute(PSA_ALG_SHA_256, payload.data(), payload.size(), hash,
                                           sizeof(hash), &hash_length);

    if (status != PSA_SUCCESS)
    {
        deinit();
        return SystemStatus::HSM_ERROR_SIGN;
    }

    uint8_t rs[64];

    if (lt_ecc_ecdsa_sign(&m_impl->handle, static_cast<lt_ecc_slot_t>(slot), hash, hash_length,
                          rs) != LT_OK)
    {
        deinit();
        return SystemStatus::HSM_ERROR_SIGN;
    }

    signature.assign(rs, rs + 64);

    return SystemStatus::OK;
}

SystemStatus LibtropicSecureElement::verify(std::vector<uint8_t> pubKey,
                                            std::vector<uint8_t> payload,
                                            std::vector<uint8_t> signature)
{
    if (pubKey.size() == 32)
    {
        return verifyEd25519(pubKey, payload, signature);
    }
    else if (pubKey.size() == 64)
    {
        return verifyP256(pubKey, payload, signature);
    }
    else
    {
        // This should never happen if parser validation is correct
        assert(false && "Invalid public key size in LibtropicSecureElement::verify()");
        return SystemStatus::HSM_ERROR_VERIFY;
    }
}

SystemStatus LibtropicSecureElement::verifyEd25519(std::vector<uint8_t> pubKey,
                                                   std::vector<uint8_t> payload,
                                                   std::vector<uint8_t> signature)
{
    // Create EVP_PKEY from raw Ed25519 public key
    EVP_PKEY* pkey =
        EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, nullptr, pubKey.data(), pubKey.size());

    if (pkey == nullptr)
    {
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    // Create message digest context
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();

    if (mdctx == nullptr)
    {
        EVP_PKEY_free(pkey);
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    // Initialize verification with NULL digest (Ed25519 is PureEdDSA, no pre-hashing)
    if (EVP_DigestVerifyInit(mdctx, nullptr, nullptr, nullptr, pkey) <= 0)
    {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    // Verify the signature (one-shot)
    int result =
        EVP_DigestVerify(mdctx, signature.data(), signature.size(), payload.data(), payload.size());

    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);

    return (result == 1) ? SystemStatus::OK : SystemStatus::HSM_ERROR_VERIFY_INVALID_SIGNATURE;
}

SystemStatus LibtropicSecureElement::verifyP256(std::vector<uint8_t> pubKey,
                                                std::vector<uint8_t> payload,
                                                std::vector<uint8_t> signature)
{
    // Hash the payload with SHA-256
    unsigned char hash[32];
    unsigned int hash_len;

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();

    if (mdctx == nullptr)
    {
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr) <= 0)
    {
        EVP_MD_CTX_free(mdctx);
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    if (EVP_DigestUpdate(mdctx, payload.data(), payload.size()) <= 0)
    {
        EVP_MD_CTX_free(mdctx);
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) <= 0)
    {
        EVP_MD_CTX_free(mdctx);
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    EVP_MD_CTX_free(mdctx);

    // Create P256 EVP_PKEY from raw public key (64 bytes: X || Y)
    // Prepend 0x04 to make uncompressed point format for OpenSSL
    unsigned char uncompressed_point[65];
    uncompressed_point[0] = 0x04;
    std::memcpy(uncompressed_point + 1, pubKey.data(), 64);

    // Create EC_KEY from uncompressed point
    EC_KEY* ec_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);

    if (ec_key == nullptr)
    {
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    const EC_GROUP* group = EC_KEY_get0_group(ec_key);
    EC_POINT* ec_point = EC_POINT_new(group);

    if (ec_point == nullptr)
    {
        EC_KEY_free(ec_key);
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    BN_CTX* bn_ctx = BN_CTX_new();

    if (bn_ctx == nullptr)
    {
        EC_POINT_free(ec_point);
        EC_KEY_free(ec_key);
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    if (EC_POINT_oct2point(group, ec_point, uncompressed_point, 65, bn_ctx) <= 0)
    {
        BN_CTX_free(bn_ctx);
        EC_POINT_free(ec_point);
        EC_KEY_free(ec_key);
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    if (EC_KEY_set_public_key(ec_key, ec_point) <= 0)
    {
        BN_CTX_free(bn_ctx);
        EC_POINT_free(ec_point);
        EC_KEY_free(ec_key);
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    BN_CTX_free(bn_ctx);
    EC_POINT_free(ec_point);

    // Create EVP_PKEY from EC_KEY
    EVP_PKEY* pkey = EVP_PKEY_new();

    if (pkey == nullptr)
    {
        EC_KEY_free(ec_key);
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    if (EVP_PKEY_assign_EC_KEY(pkey, ec_key) <= 0)
    {
        EVP_PKEY_free(pkey);
        EC_KEY_free(ec_key);
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    // Verify the signature using EVP_PKEY_verify
    EVP_PKEY_CTX* pkey_ctx = EVP_PKEY_CTX_new(pkey, nullptr);

    if (pkey_ctx == nullptr)
    {
        EVP_PKEY_free(pkey);
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    if (EVP_PKEY_verify_init(pkey_ctx) <= 0)
    {
        EVP_PKEY_CTX_free(pkey_ctx);
        EVP_PKEY_free(pkey);
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    std::vector<uint8_t> der_signature = rawSignatureToDer(signature);

    if (der_signature.empty())
    {
        EVP_PKEY_CTX_free(pkey_ctx);
        EVP_PKEY_free(pkey);
        return SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;
    }

    int result =
        EVP_PKEY_verify(pkey_ctx, der_signature.data(), der_signature.size(), hash, hash_len);

    EVP_PKEY_CTX_free(pkey_ctx);
    EVP_PKEY_free(pkey);

    return (result == 1) ? SystemStatus::OK : SystemStatus::HSM_ERROR_VERIFY_INVALID_SIGNATURE;
}

// TODO: Consider refactoring this function somewhere else
std::vector<uint8_t> LibtropicSecureElement::rawSignatureToDer(const std::vector<uint8_t>& raw_sig)
{
    // raw_sig is 64 bytes: r (32 bytes) || s (32 bytes)
    if (raw_sig.size() != 64)
    {
        return {};
    }

    BIGNUM* r = BN_bin2bn(raw_sig.data(), 32, nullptr);
    BIGNUM* s = BN_bin2bn(raw_sig.data() + 32, 32, nullptr);

    if (r == nullptr || s == nullptr)
    {
        BN_free(r);
        BN_free(s);
        return {};
    }

    ECDSA_SIG* ecdsa_sig = ECDSA_SIG_new();

    if (ecdsa_sig == nullptr)
    {
        BN_free(r);
        BN_free(s);
        return {};
    }

    ECDSA_SIG_set0(ecdsa_sig, r, s);

    unsigned char* der = nullptr;
    int der_len = i2d_ECDSA_SIG(ecdsa_sig, &der);

    ECDSA_SIG_free(ecdsa_sig);

    if (der_len <= 0)
    {
        OPENSSL_free(der);
        return {};
    }

    std::vector<uint8_t> result(der, der + der_len);
    OPENSSL_free(der);

    return result;
}

SystemStatus LibtropicSecureElement::deinit()
{
    if (lt_session_abort(&m_impl->handle) != LT_OK)
    {
        lt_deinit(&m_impl->handle);
        mbedtls_psa_crypto_free();
        return SystemStatus::HSM_ERROR_DEINIT;
    }

    if (lt_deinit(&m_impl->handle) != LT_OK)
    {
        mbedtls_psa_crypto_free();
        return SystemStatus::HSM_ERROR_DEINIT;
    }

    mbedtls_psa_crypto_free();

    return SystemStatus::OK;
}