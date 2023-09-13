#pragma once
#include "../../common.hpp"
#include "../mock/Mockroom.hpp"

struct RoomCache
{
    identifier_t id;
    std::string name;
    std::string type;
};
void TestTableSize(size_t tableSize)
{
    //! Check table size
    BOOST_CHECK_MESSAGE(database->GetRoomCount() == tableSize, "Room table has too many or too few elements");
}
void TestTableContent(const RoomCache** rooms, size_t roomCount)
{
    size_t counter = 0;

    // Iterate over every element in the database
    BOOST_REQUIRE_MESSAGE(database->LoadRooms([&rooms, &roomCount, &counter](identifier_t id, const std::string& name,
                                                                             const std::string& type) -> void {
        if (counter < roomCount)
        {
            const RoomCache* room = rooms[counter];

            //! Check id
            BOOST_CHECK_MESSAGE(id == room->id, "ID is not correct: '" << id << "' != '" << room->id << "'");

            //! Check name
            BOOST_CHECK_MESSAGE(name == room->name, "Name is not correct: '" << name << "' != '" << room->name << "'");

            //! Check type
            BOOST_CHECK_MESSAGE(type == room->type, "Type is not correct: '" << type << "' != '" << room->type << "'");
        }

        counter++;
    }),
                          "Load rooms from database");

    //! Compaire element count
    BOOST_CHECK_MESSAGE(counter <= roomCount, "Room table has too many elements");
    BOOST_CHECK_MESSAGE(counter >= roomCount, "Room table has too few elements");
}

#define UNKNOWN_ROOM "unknown room"

static RoomCache room01 = {1, UNKNOWN_ROOM, ""};
static RoomCache room02 = {2, UNKNOWN_ROOM, ""};
static RoomCache room03 = {3, UNKNOWN_ROOM, ""};
static RoomCache room04 = {4, UNKNOWN_ROOM, ""};
static RoomCache room05 = {5, UNKNOWN_ROOM, ""};
static RoomCache room06 = {6, UNKNOWN_ROOM, ""};
static RoomCache room07 = {7, UNKNOWN_ROOM, ""};
static RoomCache room08 = {8, UNKNOWN_ROOM, ""};
static RoomCache room09 = {9, UNKNOWN_ROOM, ""};
static RoomCache room10 = {10, UNKNOWN_ROOM, ""};

void UpdateRoom(RoomCache* roomCache, const std::string& name, const std::string& type)
{
    //! Update cache
    roomCache->name = name;
    roomCache->type = type;

    //! Create instance
    Ref<server::Room> room = MockRoom(roomCache->id, roomCache->name, roomCache->type);
    BOOST_REQUIRE_MESSAGE(room != nullptr, "Create server::Room instance");

    BOOST_CHECK_MESSAGE(database->UpdateRoom(room) == true, "Update room '" << name << "'");
}
void UpdateRoomPropName(RoomCache* roomCache, const std::string& name)
{
    //! Update name
    roomCache->name = name;

    //! Create instance
    Ref<server::Room> room = MockRoom(roomCache->id, roomCache->name);
    BOOST_REQUIRE_MESSAGE(room != nullptr, "Create server::Room instance");

    BOOST_CHECK_MESSAGE(database->UpdateRoomPropName(room, "old name", name) == true, "Update room prop name");
}
void UpdateRoomPropType(RoomCache* roomCache, const std::string& type)
{
    //! Update room
    roomCache->type = type;

    //! Create instance
    Ref<server::Room> room = MockRoom(roomCache->id, roomCache->name);
    BOOST_REQUIRE_MESSAGE(room != nullptr, "Create server::Room instance");

    BOOST_CHECK_MESSAGE(database->UpdateRoomPropType(room, "old type", type) == true, "Update room prop type");
}