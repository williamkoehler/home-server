#include "TestRooms.hpp"
#include "../mock/MockString.hpp"
#include "Testdatabase.hpp"

BOOST_AUTO_TEST_CASE(check_initial_state)
{
    BOOST_REQUIRE(database != nullptr);

    // Check initial table state
    TestTableSize(0);
    TestTableContent(nullptr, 0);
}

//! Test reserve
BOOST_AUTO_TEST_CASE(test_reserve_room)
{
    // Insert 10 different rooms
    database->ReserveRoom();
    database->ReserveRoom();
    database->ReserveRoom();
    database->ReserveRoom();
    database->ReserveRoom();

    database->ReserveRoom();
    database->ReserveRoom();
    database->ReserveRoom();
    database->ReserveRoom();
    database->ReserveRoom();

    // Check table
    TestTableContent(nullptr, 0);

    // Check table size
    TestTableSize(10);
}

//! Test update
BOOST_AUTO_TEST_CASE(test_partial_update_room_1)
{
    static const RoomCache* rooms[] = {
        &room01, &room02, &room04, &room05, &room09,
    };

    // Update 5 rooms
    UpdateRoom(&room01, "mooR 10", "bathroom");
    UpdateRoom(&room02, "mooR 20", "kitchen");
    UpdateRoom(&room04, "mooR 40", "bedroom");
    UpdateRoom(&room05, "mooR 50", "bathroom");
    UpdateRoom(&room09, "mooR 90", "saloon");

    // Check table
    TestTableContent(rooms, sizeof(rooms) / sizeof(RoomCache*));

    // Check table size
    TestTableSize(10);
}
BOOST_AUTO_TEST_CASE(test_partial_update_room_2)
{
    static const RoomCache* rooms[] = {
        &room01, &room02, &room03, &room04, &room05,

        &room06, &room07, &room08, &room09, &room10,
    };

    // Update 5 rooms
    UpdateRoom(&room03, "Room 03", "bathroom");
    UpdateRoom(&room06, "Room 06", "bathroom");
    UpdateRoom(&room07, "Room 07", "bathroom");
    UpdateRoom(&room08, "Room 08", "bathroom");
    UpdateRoom(&room10, "Room 10", "bathroom");

    // Check table
    TestTableContent(rooms, sizeof(rooms) / sizeof(RoomCache*));

    // Re-Update 5 Rooms
    UpdateRoom(&room01, "Room 01", "bedroom");
    UpdateRoom(&room02, "Room 02", "bedroom");
    UpdateRoom(&room04, "Room 04", "bathroom");
    UpdateRoom(&room05, "Room 05", "kitchen");
    UpdateRoom(&room09, "Room 09", "kitchen");

    // Check table
    TestTableContent(rooms, sizeof(rooms) / sizeof(RoomCache*));

    // Check table size
    TestTableSize(10);
}

//! Test update prop
BOOST_AUTO_TEST_CASE(test_update_room_prop)
{
    static const RoomCache* rooms[] = {
        &room01, &room02, &room03, &room04, &room05,

        &room06, &room07, &room08, &room09, &room10,
    };

    UpdateRoomPropName(&room01, "Updated-Room 1");
    UpdateRoomPropName(&room02, "Updated-Room 2");
    UpdateRoomPropName(&room05, "This is just a long room name to check if very very long text is working correctly");
    UpdateRoomPropName(&room02, "Room 2 is a very good name");

    UpdateRoomPropType(&room09, "bedroom");
    UpdateRoomPropType(&room07, "bedroom");
    UpdateRoomPropType(&room05, "this is a very looooooooong room type that is not very correct, but is only here to "
                                "test if long strings are allowed");
    UpdateRoomPropType(&room02, "this is a very looooooooong room type that is not very correct, but is only here to "
                                "test if long strings are allowed");

    // Check table
    TestTableContent(rooms, sizeof(rooms) / sizeof(RoomCache*));

    // Check table size
    TestTableSize(10);
}

//! Test update 2
BOOST_AUTO_TEST_CASE(test_update_room)
{
    static const RoomCache* rooms[] = {
        &room01, &room02, &room03, &room04, &room05,

        &room06, &room07, &room08, &room09, &room10,
    };

    // Update all rooms
    UpdateRoom(&room01, "Room 01", "kitchen");
    UpdateRoom(&room02, "Room 02", "kitchen");
    UpdateRoom(&room03, "Room 03", "kitchen");
    UpdateRoom(&room04, "Room 04", "kitchen");
    UpdateRoom(&room05, "Room 05", "kitchen");
    UpdateRoom(&room06, "Room 06", "kitchen");
    UpdateRoom(&room07, "Room 07", "kitchen");
    UpdateRoom(&room08, "Room 08", "kitchen");
    UpdateRoom(&room09, "Room 09", "kitchen");
    UpdateRoom(&room10, "Room 10", "kitchen");

    // Check table
    TestTableContent(rooms, sizeof(rooms) / sizeof(RoomCache*));

    // Check table size
    TestTableSize(10);
}

//! Test remove
BOOST_AUTO_TEST_CASE(test_partial_remove_room_1)
{
    static const RoomCache* rooms[] = {
        &room03, &room06, &room07, &room08, &room10,
    };

    // Remove 5 rooms
    BOOST_CHECK_MESSAGE(database->RemoveRoom(4) == true, "Remove room  4");
    BOOST_CHECK_MESSAGE(database->RemoveRoom(2) == true, "Remove room  2");
    BOOST_CHECK_MESSAGE(database->RemoveRoom(1) == true, "Remove room  1");
    BOOST_CHECK_MESSAGE(database->RemoveRoom(5) == true, "Remove room  5");
    BOOST_CHECK_MESSAGE(database->RemoveRoom(9) == true, "Remove room  9");

    // Remove 2 rooms twice
    BOOST_CHECK_MESSAGE(database->RemoveRoom(2) == true, "Remove room  2 twice");
    BOOST_CHECK_MESSAGE(database->RemoveRoom(4) == true, "Remove room  4 twice");

    // Check table
    TestTableContent(rooms, sizeof(rooms) / sizeof(RoomCache*));

    // Check table size
    TestTableSize(5);
}
BOOST_AUTO_TEST_CASE(test_partial_remove_room_2)
{
    // Remove 5 rooms
    BOOST_CHECK_MESSAGE(database->RemoveRoom(3) == true, "Remove room  3");
    BOOST_CHECK_MESSAGE(database->RemoveRoom(6) == true, "Remove room  6");
    BOOST_CHECK_MESSAGE(database->RemoveRoom(7) == true, "Remove room  7");
    BOOST_CHECK_MESSAGE(database->RemoveRoom(8) == true, "Remove room  8");
    BOOST_CHECK_MESSAGE(database->RemoveRoom(10) == true, "Remove room 10");

    // Check table
    TestTableContent(nullptr, 0);

    // Remove 3 rooms twice
    BOOST_CHECK_MESSAGE(database->RemoveRoom(7) == true, "Remove room  7 twice");
    BOOST_CHECK_MESSAGE(database->RemoveRoom(6) == true, "Remove room  6 twice");
    BOOST_CHECK_MESSAGE(database->RemoveRoom(3) == true, "Remove room  3 twice");

    // Check table
    TestTableContent(nullptr, 0);

    // Check table size
    TestTableSize(0);
}

#define MAGIC_NUMBER (0x4DA45D1C76F4A969)

//! Big scale test
BOOST_AUTO_TEST_CASE(test_big_scale)
{
    static const size_t testSize = 2000;

    RoomCache roomList[testSize];
    RoomCache* rooms[testSize];

    //! Fill database
    for (size_t index = 0; index < testSize; index++)
    {
        RoomCache& roomCache = roomList[index];
        roomCache.id = database->ReserveRoom();
        roomCache.name = GenerateString(index % 256, index * 10);
        roomCache.type = GenerateString(index % 128, index * 10);

        Ref<server::Room> room = MockRoom(roomCache.id, roomCache.name, roomCache.type);
        database->UpdateRoom(room);

        rooms[index] = &roomCache;
    }

    //! Verify database

    // Check table
    TestTableContent((const RoomCache**)rooms, testSize);

    // Check table size
    TestTableSize(testSize);
}