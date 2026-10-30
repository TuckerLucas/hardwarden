#include "Pkcs11Hsm.hpp"

#include <dlfcn.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>

#include <cstring>
#include <vector>

Pkcs11Hsm::Pkcs11Hsm(std::string modulePath) : m_modulePath(std::move(modulePath))
{
}

Pkcs11Hsm::~Pkcs11Hsm()
{
    deinit();
}

CK_RV Pkcs11Hsm::findObjects(CK_OBJECT_CLASS objectClass, std::optional<uint8_t> slot,
                             std::vector<CK_OBJECT_HANDLE>& found)
{
    // What to look for: objects of this class and, if a slot is given,
    // whose CKA_ID is {slot}.
    // Always include CKA_CLASS: without it, this module also searches its
    // user-data storage, where the ID means something else.
    CK_BYTE id = slot.value_or(0);
    std::vector<CK_ATTRIBUTE> searchTemplate = {
        {CKA_CLASS, &objectClass, sizeof(objectClass)},
    };

    if (slot.has_value())
    {
        searchTemplate.push_back({CKA_ID, &id, sizeof(id)});
    }

    CK_RV rv = m_fn->C_FindObjectsInit(m_session, searchTemplate.data(), searchTemplate.size());

    if (rv != CKR_OK)
    {
        return rv;
    }

    found.clear();
    CK_OBJECT_HANDLE batch[16];
    CK_ULONG count = 0;

    // Collect results until the module returns none
    do
    {
        rv = m_fn->C_FindObjects(m_session, batch, 16, &count);

        if (rv != CKR_OK)
        {
            m_fn->C_FindObjectsFinal(m_session);
            return rv;
        }

        found.insert(found.end(), batch, batch + count);
    } while (count > 0);

    return m_fn->C_FindObjectsFinal(m_session);
}

CK_RV Pkcs11Hsm::readPublicKey(CK_OBJECT_HANDLE object, std::vector<uint8_t>& pubKey)
{
    CK_BYTE ecPoint[67];  // big enough for either curve
    CK_ATTRIBUTE attr = {CKA_EC_POINT, ecPoint, sizeof(ecPoint)};

    CK_RV rv = m_fn->C_GetAttributeValue(m_session, object, &attr, 1);

    if (rv != CKR_OK)
    {
        return rv;
    }

    // P-256: 04 41 | 04 X Y  ->  X Y
    if (attr.ulValueLen == 67 && ecPoint[0] == 0x04 && ecPoint[1] == 65 && ecPoint[2] == 0x04)
    {
        pubKey.assign(ecPoint + 3, ecPoint + 67);
        return CKR_OK;
    }

    // Ed25519: 04 20 | key  ->  key
    if (attr.ulValueLen == 34 && ecPoint[0] == 0x04 && ecPoint[1] == 32)
    {
        pubKey.assign(ecPoint + 2, ecPoint + 34);
        return CKR_OK;
    }

    return CKR_GENERAL_ERROR;  // unexpected format
}

SystemStatus Pkcs11Hsm::init()
{
    // 1. Load the module (.so) into this process
    m_library = dlopen(m_modulePath.c_str(), RTLD_NOW | RTLD_LOCAL);

    if (m_library == nullptr)
    {
        return SystemStatus::HSM_ERROR_INIT;
    }

    // 2. Ask the module for its table of PKCS#11 functions
    auto getFunctionList =
        reinterpret_cast<CK_C_GetFunctionList>(dlsym(m_library, "C_GetFunctionList"));

    if (getFunctionList == nullptr || getFunctionList(&m_fn) != CKR_OK)
    {
        deinit();
        return SystemStatus::HSM_ERROR_INIT;
    }

    // 3. Initialise the library
    if (m_fn->C_Initialize(nullptr) != CKR_OK)
    {
        deinit();
        return SystemStatus::HSM_ERROR_INIT;
    }

    m_initialized = true;

    // 4. Find the first slot with a token present (first call gets the count,
    //    second call fills the list)
    CK_ULONG count = 0;

    if (m_fn->C_GetSlotList(CK_TRUE, nullptr, &count) != CKR_OK || count == 0)
    {
        deinit();
        return SystemStatus::HSM_ERROR_INIT;
    }

    std::vector<CK_SLOT_ID> slots(count);

    if (m_fn->C_GetSlotList(CK_TRUE, slots.data(), &count) != CKR_OK)
    {
        deinit();
        return SystemStatus::HSM_ERROR_INIT;
    }

    m_slotId = slots[0];

    // 5. Open a read/write session on that token
    if (m_fn->C_OpenSession(m_slotId, CKF_SERIAL_SESSION | CKF_RW_SESSION, nullptr, nullptr,
                            &m_session) != CKR_OK)
    {
        m_session = CK_INVALID_HANDLE;
        deinit();
        return SystemStatus::HSM_ERROR_INIT;
    }

    return SystemStatus::OK;
}

SystemStatus Pkcs11Hsm::status()
{
    if (m_fn == nullptr)
    {
        return SystemStatus::HSM_ERROR_STATUS;
    }

    CK_TOKEN_INFO info;

    if (m_fn->C_GetTokenInfo(m_slotId, &info) != CKR_OK)
    {
        deinit();
        return SystemStatus::HSM_ERROR_STATUS;
    }

    return SystemStatus::OK;
}

// TODO: Think if erasing a key from an empty slot should succeed
SystemStatus Pkcs11Hsm::eraseKey(uint8_t slot)
{
    if (m_fn == nullptr)
    {
        return SystemStatus::HSM_ERROR_ERASE_KEY;
    }

    // 1. Find both halves of the key pair in this slot
    std::vector<CK_OBJECT_HANDLE> privateKeys;
    std::vector<CK_OBJECT_HANDLE> publicKeys;

    if (findObjects(CKO_PRIVATE_KEY, slot, privateKeys) != CKR_OK ||
        findObjects(CKO_PUBLIC_KEY, slot, publicKeys) != CKR_OK)
    {
        deinit();
        return SystemStatus::HSM_ERROR_ERASE_KEY;
    }

    std::vector<CK_OBJECT_HANDLE> objects = privateKeys;
    objects.insert(objects.end(), publicKeys.begin(), publicKeys.end());

    // 2. Destroy each one. An empty slot finds nothing, which counts as success.
    for (CK_OBJECT_HANDLE object : objects)
    {
        CK_RV rv = m_fn->C_DestroyObject(m_session, object);

        // On TROPIC01, destroying one half erases the whole slot, so the
        // other half may already be gone. That's fine.
        if (rv != CKR_OK && rv != CKR_OBJECT_HANDLE_INVALID)
        {
            deinit();
            return SystemStatus::HSM_ERROR_ERASE_KEY;
        }
    }

    return SystemStatus::OK;
}

SystemStatus Pkcs11Hsm::generateKey(uint8_t slot, Curve curve)
{
    if (m_fn == nullptr)
    {
        return SystemStatus::HSM_ERROR_GENERATE_KEY_HW_ERROR;
    }

    // 1. Refuse if the slot already holds a key
    std::vector<CK_OBJECT_HANDLE> privateKeys;
    std::vector<CK_OBJECT_HANDLE> publicKeys;

    if (findObjects(CKO_PRIVATE_KEY, slot, privateKeys) != CKR_OK ||
        findObjects(CKO_PUBLIC_KEY, slot, publicKeys) != CKR_OK)
    {
        deinit();
        return SystemStatus::HSM_ERROR_GENERATE_KEY_HW_ERROR;
    }

    if (!privateKeys.empty() || !publicKeys.empty())
    {
        deinit();
        return SystemStatus::HSM_ERROR_GENERATE_KEY_SLOT_OCCUPIED;
    }

    // 2. Pick the mechanism and curve OID
    std::vector<CK_BYTE> ecParams;
    CK_MECHANISM mechanism = {0, nullptr, 0};

    if (curve == Curve::P256)
    {
        mechanism.mechanism = CKM_EC_KEY_PAIR_GEN;
        ecParams = {0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07};
    }
    else
    {
        mechanism.mechanism = CKM_EC_EDWARDS_KEY_PAIR_GEN;
        ecParams = {0x06, 0x03, 0x2B, 0x65, 0x70};
    }

    // 3. Describe the two objects we want created
    CK_BBOOL yes = CK_TRUE;
    CK_BYTE id = slot;

    // CKA_ID must be in the public template: this module ignores it
    // in the private template.
    CK_ATTRIBUTE publicTemplate[] = {
        {CKA_TOKEN, &yes, sizeof(yes)},   // store on the device
        {CKA_VERIFY, &yes, sizeof(yes)},  // can verify signatures
        {CKA_EC_PARAMS, ecParams.data(), ecParams.size()},
        {CKA_ID, &id, sizeof(id)},
    };

    CK_ATTRIBUTE privateTemplate[] = {
        {CKA_TOKEN, &yes, sizeof(yes)},      // store on the device
        {CKA_PRIVATE, &yes, sizeof(yes)},    // needs login, where supported
        {CKA_SENSITIVE, &yes, sizeof(yes)},  // never readable
        {CKA_SIGN, &yes, sizeof(yes)},       // can sign
        {CKA_ID, &id, sizeof(id)},
    };

    // 4. Generate
    CK_OBJECT_HANDLE publicKey = CK_INVALID_HANDLE;
    CK_OBJECT_HANDLE privateKey = CK_INVALID_HANDLE;

    CK_RV rv = m_fn->C_GenerateKeyPair(m_session, &mechanism, publicTemplate, 4, privateTemplate, 5,
                                       &publicKey, &privateKey);

    if (rv != CKR_OK)
    {
        deinit();
        return SystemStatus::HSM_ERROR_GENERATE_KEY_HW_ERROR;
    }

    return SystemStatus::OK;
}

SystemStatus Pkcs11Hsm::readKey(uint8_t slot, std::vector<uint8_t>& pubKey)
{
    if (m_fn == nullptr)
    {
        return SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR;
    }

    std::vector<CK_OBJECT_HANDLE> found;

    if (findObjects(CKO_PUBLIC_KEY, slot, found) != CKR_OK)
    {
        deinit();
        return SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR;
    }

    // No public key object with this ID means the slot is empty
    if (found.empty())
    {
        deinit();
        return SystemStatus::HSM_ERROR_READ_KEY_EMPTY_SLOT;
    }

    if (readPublicKey(found[0], pubKey) != CKR_OK)
    {
        deinit();
        return SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR;
    }

    return SystemStatus::OK;
}

SystemStatus Pkcs11Hsm::listKeys(std::vector<std::vector<uint8_t>>& pubKeys)
{
    if (m_fn == nullptr)
    {
        return SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR;
    }

    // 1. Find every public key on the token (no slot filter)
    std::vector<CK_OBJECT_HANDLE> found;

    if (findObjects(CKO_PUBLIC_KEY, std::nullopt, found) != CKR_OK)
    {
        deinit();
        return SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR;
    }

    // 2. One entry per slot, empty unless a key is found there
    pubKeys.assign(32, {});

    for (CK_OBJECT_HANDLE object : found)
    {
        // 3. Read CKA_ID to learn which slot this key is in
        CK_BYTE id[16];
        CK_ATTRIBUTE idAttr = {CKA_ID, id, sizeof(id)};

        if (m_fn->C_GetAttributeValue(m_session, object, &idAttr, 1) != CKR_OK)
        {
            deinit();
            return SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR;
        }

        // hardwarden slots are a single byte, 0-31. Skip anything else.
        if (idAttr.ulValueLen != 1 || id[0] >= 32)
        {
            continue;
        }

        // 4. Read the key into its slot's entry
        if (readPublicKey(object, pubKeys[id[0]]) != CKR_OK)
        {
            deinit();
            return SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR;
        }
    }

    return SystemStatus::OK;
}

SystemStatus Pkcs11Hsm::sign(uint8_t slot, Curve curve, std::vector<uint8_t> payload,
                             std::vector<uint8_t>& signature)
{
    if (m_fn == nullptr)
    {
        return SystemStatus::HSM_ERROR_SIGN;
    }

    // 1. Find the private key in this slot
    std::vector<CK_OBJECT_HANDLE> found;

    if (findObjects(CKO_PRIVATE_KEY, slot, found) != CKR_OK || found.empty())
    {
        deinit();
        return SystemStatus::HSM_ERROR_SIGN;
    }

    // 2. Pick the mechanism and the data to sign
    CK_MECHANISM mechanism = {CKM_EDDSA, nullptr, 0};
    std::vector<uint8_t> data = payload;

    if (curve == Curve::P256)
    {
        // CKM_ECDSA signs a hash, not the message: compute SHA-256 here
        std::vector<uint8_t> hash(32);
        unsigned int hashLen = 0;

        if (EVP_Digest(payload.data(), payload.size(), hash.data(), &hashLen, EVP_sha256(),
                       nullptr) != 1)
        {
            deinit();
            return SystemStatus::HSM_ERROR_SIGN;
        }

        data = hash;
        mechanism.mechanism = CKM_ECDSA;
    }

    // 3. Sign: init with key and mechanism, then sign the data
    if (m_fn->C_SignInit(m_session, &mechanism, found[0]) != CKR_OK)
    {
        deinit();
        return SystemStatus::HSM_ERROR_SIGN;
    }

    std::vector<uint8_t> out(64);  // R || S, 64 bytes for both curves
    CK_ULONG outLen = out.size();

    if (m_fn->C_Sign(m_session, data.data(), data.size(), out.data(), &outLen) != CKR_OK ||
        outLen != 64)
    {
        deinit();
        return SystemStatus::HSM_ERROR_SIGN;
    }

    signature = out;
    return SystemStatus::OK;
}

SystemStatus Pkcs11Hsm::verify(std::vector<uint8_t> pubKey, std::vector<uint8_t> payload,
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

SystemStatus Pkcs11Hsm::verifyEd25519(std::vector<uint8_t> pubKey, std::vector<uint8_t> payload,
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

SystemStatus Pkcs11Hsm::verifyP256(std::vector<uint8_t> pubKey, std::vector<uint8_t> payload,
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
std::vector<uint8_t> Pkcs11Hsm::rawSignatureToDer(const std::vector<uint8_t>& raw_sig)
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

SystemStatus Pkcs11Hsm::deinit()
{
    CK_RV rv = CKR_OK;

    if (m_session != CK_INVALID_HANDLE)
    {
        rv = m_fn->C_CloseSession(m_session);
        m_session = CK_INVALID_HANDLE;
    }

    if (m_initialized)
    {
        CK_RV finalizeRv = m_fn->C_Finalize(nullptr);
        rv = (rv == CKR_OK) ? finalizeRv : rv;
        m_initialized = false;
    }

    if (m_library != nullptr)
    {
        dlclose(m_library);
        m_library = nullptr;
    }

    m_fn = nullptr;

    return (rv == CKR_OK) ? SystemStatus::OK : SystemStatus::HSM_ERROR_DEINIT;
}