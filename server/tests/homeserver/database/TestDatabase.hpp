#pragma once
#include "../../common.hpp"
#include "../../../homeserver/database/Database.hpp"

#define HOME_DB "test_home.sqlite3"

static Ref<server::Database> database = nullptr;

//! Cleanup
BOOST_AUTO_TEST_CASE(cleanup)
{
	//! Delete old database
	if (remove(HOME_DB) != 0)
	{
		if (errno != ENOENT)
			BOOST_REQUIRE_MESSAGE(false, "Remove " HOME_DB);
	}
}

//! Create database
BOOST_AUTO_TEST_CASE(test_database_creation)
{
	//! Create database
	database = server::Database::Create(HOME_DB);
	BOOST_REQUIRE_MESSAGE(database != nullptr, "Create server::Database instance");

	//! Test connection
	BOOST_REQUIRE_MESSAGE(database->GetConnection_TEST() != nullptr, "Test sqlite connection");
}