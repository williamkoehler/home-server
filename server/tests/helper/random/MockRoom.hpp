#pragma once
#include "../../../homeserver/home/room.hpp"
#include "../../common.hpp"

Ref<server::Room> MockRoom(identifier_t id = 1, const std::string& name = "unknown room",
                           const std::string& type = "no type")
{
    return server::Room::Create(name, id, type);
}