#pragma once
#include "../../common.hpp"
#include "../../../homeserver/home/Room.hpp"

Ref<server::Room> MockRoom(identifier_t id = 1, const std::string& name = "unknown room", const std::string& type = "no type")
{
	return server::Room::Create(name, id, type);
}