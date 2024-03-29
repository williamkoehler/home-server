#pragma once
#include "../../../homeserver/user/user.hpp"
#include "../../common.hpp"

Ref<server::User> MockUser(identifier_t id = 1, const std::string& name = "unknown user", uint8_t* hash = nullptr,
                           uint8_t* salt = nullptr,
                           server::UserAccessLevel accessLevel = server::UserAccessLevel::kRestrictedUserAccessLevel)
{
    static uint8_t HASH[SHA256_DIGEST_LENGTH] = {0xb7, 0x6d, 0xfc, 0x71, 0x99, 0xb6, 0x17, 0x85, 0xaa, 0x18, 0x4b,
                                                 0xa9, 0x55, 0x7b, 0x95, 0x1e, 0x2d, 0x75, 0x6c, 0x0d, 0x33, 0x09,
                                                 0xb9, 0xcb, 0xb3, 0x80, 0x38, 0xb8, 0xbc, 0x99, 0x40, 0xc5};
    static uint8_t SALT[SALT_LENGTH] = {0xb7, 0x6d, 0xfc, 0x71, 0x17, 0x85, 0xaa, 0xa9,
                                        0x55, 0x7b, 0x95, 0x1e, 0x0d, 0x38, 0xb8, 0xbc};

    if (hash == nullptr)
        hash = HASH;

    if (salt == nullptr)
        salt = SALT;

    return server::User::Create(name, id, hash, salt, accessLevel);
}