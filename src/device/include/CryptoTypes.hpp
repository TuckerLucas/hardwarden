#pragma once

#include <cassert>
#include <string>
#include <unordered_map>

constexpr uint16_t ED25519_MAX_MSG_SIZE = 4096;

enum class Curve
{
    P256 = 1,
    Ed25519 = 2,
};

inline std::string curveToString(Curve curve)
{
    static const std::unordered_map<Curve, std::string> curve_map = {
        {Curve::Ed25519, "ed25519"},
        {Curve::P256, "p256"},
    };

    auto it = curve_map.find(curve);
    assert(it != curve_map.end() && "Unhandled curve value in curveToString()");
    return (it != curve_map.end()) ? it->second : "unknown";
}
