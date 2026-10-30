#pragma once

const std::string valid_pubkey_32_bytes =
    "714255fc32c686b12996e22e72ad972d768df450adda40b5eb0c88779dd9155b";

const std::string valid_signature =
    "d0fc58b11758d70d6f11824fffc5a3e19cceaa9f15956cd48d235bcf169ab37519ccfb51e15dc53face471a61e212e"
    "73df0fee8750f2c04269fff982ae7df202";

const std::string invalid_signature_64_bytes =
    "d0fc58b11758d70d6f11824fffc5a3e19cceaa9f15956cd48d235bcf169ab37519ccfb51e15dc53face471a61e212e"
    "73df0fee8750f2c04269fff982ae7df2zz";

const std::string invalid_signature_65_bytes =
    "d0fc58b11758d70d6f11824fffc5a3e19cceaa9f15956cd48d235bcf169ab37519ccfb51e15dc53face471a61e212e"
    "73df0fee8750f2c04269fff982ae7df202aa";