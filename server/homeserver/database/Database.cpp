#include "Database.hpp"
#include "../scripting/ScriptSource.hpp"
#include "../home/Room.hpp"
#include "../home/DeviceController.hpp"
#include "../home/Device.hpp"
#include "../home/Action.hpp"
#include "../user/User.hpp"
#include <cppcodec/base64_rfc4648.hpp>

namespace server
{
	boost::weak_ptr<Database> instanceDatabase;

	Database::Database()
		: connection(nullptr)
	{
	}
	Database::~Database()
	{
		if (connection != nullptr)
		{
			sqlite3_close(connection);
			connection != nullptr;
		}
	}
	Ref<Database> Database::Create(const std::string& db)
	{
		if (!instanceDatabase.expired())
			return Ref<Database>(instanceDatabase);

		Ref<Database> database = boost::make_shared<Database>();
		if (database == nullptr)
			return nullptr;

		instanceDatabase = database;

		// Open / Create database
		if (sqlite3_open_v2(db.c_str(), &database->connection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to open/create database.\n{0}", sqlite3_errcode(database->connection));
			return nullptr;
		}

		// Create necessary tables
		char* err = nullptr;

		// Scripting
		{
			// Script Source Table
			if (sqlite3_exec(database->connection,
				R"(create table if not exists scriptsources)"
				R"((id integer not null primary key, name text, usage text not null, language text not null, data blob not null))",
				nullptr,
				nullptr,
				&err) != SQLITE_OK)
			{
				LOG_ERROR("Failing to create 'scriptsources' table.\n{0}", err);
				return nullptr;
			}
		}

		// Home
		{
			// Room table
			if (sqlite3_exec(database->connection,
				R"(create table if not exists rooms)"
				R"((id integer not null primary key, name text, type text not null))",
				nullptr,
				nullptr,
				&err) != SQLITE_OK)
			{
				LOG_ERROR("Failing to create 'rooms' table.\n{0}", err);
				return nullptr;
			}

			// Device Controller Table
			if (sqlite3_exec(database->connection,
				R"(create table if not exists devicecontrollers)"
				R"((id integer not null primary key, name text, pluginid integer not null, roomid integer, data text not null, foreign key(roomid) references rooms(id)))",
				nullptr,
				nullptr,
				&err) != SQLITE_OK)
			{
				LOG_ERROR("Failing to create 'devicecontrollers' table.\n{0}", err);
				return nullptr;
			}

			// Device Table
			if (sqlite3_exec(database->connection,
				R"(create table if not exists devices)"
				R"((id integer not null primary key, name text, pluginid integer not null, controllerid integer, roomid integer, data text not null, foreign key (controllerid) references devicecontrollers (id), foreign key (roomid) references rooms (id)))",
				nullptr,
				nullptr,
				&err) != SQLITE_OK)
			{
				LOG_ERROR("Failing to create 'devices' table.\n{0}", err);
				return nullptr;
			}

			// Action Table
			if (sqlite3_exec(database->connection,
				R"(create table if not exists actions)"
				R"((id integer not null primary key, name text, sourceid integer not null, roomid integer, data text not null, foreign key (roomid) references rooms (id)))",
				nullptr,
				nullptr,
				&err) != SQLITE_OK)
			{
				LOG_ERROR("Failing to create 'actions' table.\n{0}", err);
				return nullptr;
			}
		}

		// User
		{
			// User Table
			if (sqlite3_exec(database->connection,
				R"(create table if not exists users)"
				R"((id integer not null primary key, name text, hash text not null, salt text not null, accesslevel text not null, data text not null))",
				nullptr,
				nullptr,
				&err) != SQLITE_OK)
			{
				LOG_ERROR("Failing to create 'users' table.\n{0}", err);
				return nullptr;
			}
		}

		return database;
	}
	Ref<Database> Database::GetInstance()
	{
		return Ref<Database>(instanceDatabase);
	}

	//! ScriptSource
	bool Database::LoadScriptSources(const boost::function<void(identifier_t sourceID, const std::string& name, ScriptUsage usage, ScriptLanguage language, const std::string_view& data)>& callback)
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(select id, name, usage, language, data from scriptsources)", 57,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			identifier_t sourceID = sqlite3_column_int64(statement, 0);

			// Check for null
			if (sqlite3_column_type(statement, 1) == SQLITE_NULL ||
				sqlite3_column_type(statement, 2) == SQLITE_NULL ||
				sqlite3_column_type(statement, 3) == SQLITE_NULL ||
				sqlite3_column_type(statement, 4) == SQLITE_NULL)
			{
				LOG_ERROR("Failing to load invalid room {0}");
				continue;
			}

			const unsigned char* name = sqlite3_column_text(statement, 1);
			size_t nameSize = sqlite3_column_bytes(statement, 1);

			const unsigned char* usage = sqlite3_column_text(statement, 2);
			size_t usageSize = sqlite3_column_bytes(statement, 2);

			const unsigned char* language = sqlite3_column_text(statement, 3);
			size_t languageSize = sqlite3_column_bytes(statement, 3);

			const void* data = sqlite3_column_text(statement, 4);
			size_t dataSize = sqlite3_column_bytes(statement, 4);

			// Check values
			if (name == nullptr ||
				usage == nullptr ||
				language == nullptr ||
				data == nullptr)
			{
				LOG_ERROR("Failing to load invalid script source properties {0}", sourceID);
				continue;
			}

			callback(
				sourceID, 
				std::string((const char*)name, nameSize), 
				ParseScriptUsage(std::string((const char*)usage, usageSize)), 
				ParseScriptLanguage(std::string((const char*)language, languageSize)), 
				std::string_view((const char*)data, dataSize));
		}

		sqlite3_finalize(statement);
		return true;
	}

	identifier_t Database::ReserveScriptSource()
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(insert into scriptsources values)"
			R"(((select ifnull((select id+1 from scriptsources where (id+1) not in (select id from scriptsources) order by id asc limit 1), 1)),)"
			R"(null, "unknown", "unknown", ""))", 192,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to insert user into 'scriptsources' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		identifier_t sourceID = sqlite3_last_insert_rowid(connection);

		sqlite3_finalize(statement);
		return sourceID;
	}
	bool Database::UpdateScriptSource(Ref<ScriptSource> scriptSource)
	{
		assert(scriptSource != nullptr);

		// Lock
		boost::lock(mutex, scriptSource->mutex);
		boost::lock_guard lock(mutex, boost::adopt_lock);
		boost::lock_guard lock2(scriptSource->mutex, boost::adopt_lock);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(replace into scriptsources values (?, ?, ?, ?, ?))", 49,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		std::string usage = StringifyScriptUsage(scriptSource->usage);

		std::string language = StringifyScriptLanguage(scriptSource->language);

		if (sqlite3_bind_int64(statement, 1, scriptSource->sourceID) != SQLITE_OK ||
			sqlite3_bind_text(statement, 2, scriptSource->name.data(), scriptSource->name.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_text(statement, 3, usage.data(), usage.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_text(statement, 4, language.data(), language.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_blob(statement, 5, scriptSource->data.data(), scriptSource->data.size(), nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to insert user into 'scriptsources' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);
		return true;
	}
	bool Database::UpdateScriptSourcePropName(Ref<ScriptSource> scriptSource, const std::string& value, const std::string& newValue)
	{
		assert(scriptSource != nullptr);

		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(update scriptsources set name = ? where id = ?)", 46,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_text(statement, 1, newValue.data(), newValue.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 2, scriptSource->sourceID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to update script source name.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);

		return true;
	}
	bool Database::UpdateScriptSourcePropData(Ref<ScriptSource> scriptSource, const std::string_view& newValue)
	{
		assert(scriptSource != nullptr);

		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(update scriptsources set data = ? where id = ?)", 46,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_blob(statement, 1, newValue.data(), newValue.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 2, scriptSource->sourceID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to update script source data.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);

		return true;
	}
	bool Database::RemoveScriptSource(identifier_t sourceID)
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(delete from scriptsources where id = ?)", 38,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_int64(statement, 1, sourceID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to delete script source from 'scriptsources' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);
		return true;
	}

	size_t Database::GetScriptSourceCount()
	{
		return size_t();
	}

	//! Room
	bool Database::LoadRooms(const boost::function<void(identifier_t roomID, const std::string& name, const std::string& type)>& callback)
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(select id, name, type from rooms)", 32,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			identifier_t roomID = sqlite3_column_int64(statement, 0);

			// Check for null
			if (sqlite3_column_type(statement, 1) == SQLITE_NULL ||
				sqlite3_column_type(statement, 2) == SQLITE_NULL)
			{
				LOG_ERROR("Failing to load invalid room {0}", roomID);
				continue;
			}

			const unsigned char* name = sqlite3_column_text(statement, 1);
			size_t nameSize = sqlite3_column_bytes(statement, 1);

			const unsigned char* type = sqlite3_column_text(statement, 2);
			size_t typeSize = sqlite3_column_bytes(statement, 2);

			// Check values
			if (name == nullptr ||
				type == nullptr)
			{
				LOG_ERROR("Failing to load invalid room properties {0}", roomID);
				continue;
			}

			callback(
				roomID,
				std::string((const char*)name, nameSize),
				std::string((const char*)type, typeSize));
		}

		sqlite3_finalize(statement);
		return true;
	}

	identifier_t Database::ReserveRoom()
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(insert into rooms values)"
			R"(((select ifnull((select (id+1) from rooms where (id+1) not in (select id from rooms) order by id asc limit 1), 1)),)"
			R"(null, "no type"))", 155,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to delete room from 'rooms' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		identifier_t roomID = sqlite3_last_insert_rowid(connection);

		sqlite3_finalize(statement);
		return roomID;
	}
	bool Database::UpdateRoom(Ref<Room> room)
	{
		assert(room != nullptr);

		// Lock
		boost::lock(mutex, room->mutex);
		boost::lock_guard lock(mutex, boost::adopt_lock);
		boost::lock_guard lock2(room->mutex, boost::adopt_lock);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(replace into rooms values (?, ?, ?))", 48,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_int64(statement, 1, room->roomID) != SQLITE_OK ||
			sqlite3_bind_text(statement, 2, room->name.data(), room->name.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_text(statement, 3, room->type.data(), room->type.size(), nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to delete room from 'rooms' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);
		return true;
	}
	bool Database::UpdateRoomPropName(Ref<Room> room, const std::string& value, const std::string& newValue)
	{
		assert(room != nullptr);

		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(update rooms set name = ? where id = ?)", 40,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_text(statement, 1, newValue.data(), newValue.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 2, room->roomID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to update room name.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);

		return true;
	}
	bool Database::UpdateRoomPropType(Ref<Room> room, const std::string& value, const std::string& newValue)
	{
		assert(room != nullptr);

		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(update rooms set type = ? where id = ?)", 40,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_text(statement, 1, newValue.data(), newValue.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 2, room->roomID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to update room type.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);

		return true;
	}
	bool Database::RemoveRoom(identifier_t roomID)
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(delete from rooms where id = ?)", 30,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_int64(statement, 1, roomID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to delete room from 'rooms' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);
		return true;
	}

	size_t Database::GetRoomCount()
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(select count(*) from rooms)", 26,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		if (sqlite3_step(statement) != SQLITE_ROW)
		{
			LOG_ERROR("Failing to count rooms.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		size_t roomCount = sqlite3_column_int64(statement, 0);

		sqlite3_finalize(statement);
		return roomCount;
	}

	//! DeviceController
	bool Database::LoadDeviceControllers(const boost::function<void(identifier_t controllerID, const std::string& name, identifier_t pluginID, identifier_t roomID, const std::string& data)>& callback)
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(select id, name, pluginid, roomid, data from devicecontrollers)", 62,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			identifier_t controllerID = sqlite3_column_int64(statement, 0);

			// Check values
			if (sqlite3_column_type(statement, 1) == SQLITE_NULL ||
				sqlite3_column_type(statement, 4) == SQLITE_NULL)
			{
				LOG_ERROR("Failing to load invalid device controller {0}", controllerID);
				continue;
			}

			const unsigned char* name = sqlite3_column_text(statement, 1);
			size_t nameSize = sqlite3_column_bytes(statement, 1);

			identifier_t pluginID = sqlite3_column_int64(statement, 2);

			identifier_t roomID = sqlite3_column_int64(statement, 3);

			const unsigned char* data = sqlite3_column_text(statement, 4);
			size_t dataSize = sqlite3_column_bytes(statement, 4);

			// Check values
			if (name == nullptr ||
				data == nullptr)
			{
				LOG_ERROR("Failing to load invalid device controller properties {0}", controllerID);
				continue;
			}

			callback(controllerID,
				std::string((const char*)name, nameSize),
				pluginID,
				roomID,
				std::string((const char*)data, dataSize));
		}

		sqlite3_finalize(statement);
		return true;
	}

	identifier_t Database::ReserveDeviceController()
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(insert into devicecontrollers values)"
			R"(((select ifnull((select id+1 from devicecontrollers where (id+1) not in (select id from devicecontrollers) order by id asc limit 1), 1)),)"
			R"(null, 0, null, "{}"))", 193,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to insert devicecontroller into 'devicecontrollers' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		identifier_t controllerID = sqlite3_last_insert_rowid(connection);

		sqlite3_finalize(statement);
		return controllerID;
	}
	bool Database::UpdateDeviceController(Ref<DeviceController> controller)
	{
		assert(controller != nullptr);

		// Lock
		boost::lock(mutex, controller->mutex);
		boost::lock_guard lock(mutex, boost::adopt_lock);
		boost::lock_guard lock2(controller->mutex, boost::adopt_lock);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(replace into devicecontrollers values (?, ?, ?, ?, "{}"))", 56,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_int64(statement, 1, controller->controllerID) != SQLITE_OK ||
			sqlite3_bind_text(statement, 2, controller->name.data(), controller->name.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 3, controller->plugin->GetPluginID()) != SQLITE_OK ||
			(controller->room != nullptr ?
				sqlite3_bind_int64(statement, 4, controller->room->GetRoomID()) :
				sqlite3_bind_null(statement, 4)) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to delete devicecontroller from 'devicecontrollers' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);
		return true;
	}
	bool Database::UpdateDeviceControllerPropName(Ref<DeviceController> controller, const std::string& value, const std::string& newValue)
	{
		assert(controller != nullptr);

		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(update devicecontrollers set name = ? where id = ?)", 50,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_text(statement, 1, newValue.data(), newValue.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 2, controller->controllerID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to update device controller name.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);

		return true;
	}
	bool Database::UpdateDeviceControllerPropRoom(Ref<DeviceController> controller, Ref<Room> value, Ref<Room> newValue)
	{
		assert(controller != nullptr);

		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(update devicecontrollers set roomid = ? where id = ?)", 52,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if ((newValue != nullptr ?
			sqlite3_bind_int64(statement, 1, newValue->GetRoomID()) :
			sqlite3_bind_null(statement, 1)) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 2, controller->controllerID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to update device room.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);

		return true;
	}
	bool Database::RemoveDeviceController(identifier_t controllerID)
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(delete from devicecontrollers where id = ?)", 42,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_int64(statement, 1, controllerID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to delete devicecontroller from 'devicecontrollers' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);
		return true;
	}

	size_t Database::GetDeviceControllerCount()
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(select count(*) from devicecontrollers)", 38,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		if (sqlite3_step(statement) != SQLITE_ROW)
		{
			LOG_ERROR("Failing to count users.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		size_t deviceControllerCount = sqlite3_column_int64(statement, 0);

		sqlite3_finalize(statement);
		return deviceControllerCount;
	}

	//! Device
	bool Database::LoadDevices(const boost::function<void(identifier_t deviceID, const std::string& name, identifier_t pluginID, identifier_t controllerID, identifier_t roomID, const std::string& data)>& callback)
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(select id, name, pluginid, controllerid, roomid, data from devices)", 66,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			identifier_t deviceID = sqlite3_column_int64(statement, 0);

			// Check values
			if (sqlite3_column_type(statement, 1) == SQLITE_NULL ||
				sqlite3_column_type(statement, 5) == SQLITE_NULL)
			{
				LOG_ERROR("Failing to load invalid device {0}", deviceID);
				continue;
			}

			const unsigned char* name = sqlite3_column_text(statement, 1);
			size_t nameSize = sqlite3_column_bytes(statement, 1);

			identifier_t pluginID = sqlite3_column_int64(statement, 2);

			identifier_t controllerID = sqlite3_column_int64(statement, 3);

			identifier_t roomID = sqlite3_column_int64(statement, 4);

			const unsigned char* data = sqlite3_column_text(statement, 5);
			size_t dataSize = sqlite3_column_bytes(statement, 5);

			// Check values
			if (name == nullptr ||
				data == nullptr)
			{
				LOG_ERROR("Failing to load invalid device properties {0}", deviceID);
				continue;
			}

			callback(
				deviceID,
				std::string((const char*)name, nameSize),
				pluginID,
				controllerID,
				roomID,
				std::string((const char*)data, dataSize));
		}

		sqlite3_finalize(statement);
		return true;
	}

	identifier_t Database::ReserveDevice()
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(insert into devices values)"
			R"(((select ifnull((select id+1 from devices where (id+1) not in (select id from devices) order by id asc limit 1), 1)),)"
			R"(null, 0, null, null, "{}"))", 179,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to insert device into 'devices' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		identifier_t deviceID = sqlite3_last_insert_rowid(connection);

		sqlite3_finalize(statement);
		return deviceID;
	}
	bool Database::UpdateDevice(Ref<Device> device)
	{
		assert(device != nullptr);

		// Lock
		boost::lock(mutex, device->mutex);
		boost::lock_guard lock(mutex, boost::adopt_lock);
		boost::lock_guard lock2(device->mutex, boost::adopt_lock);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(replace into devices values (?, ?, ?, ?, ?, "{}"))", 49,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_int64(statement, 1, device->deviceID) != SQLITE_OK ||
			sqlite3_bind_text(statement, 2, device->name.data(), device->name.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 3, device->plugin->GetPluginID()) != SQLITE_OK ||
			(device->controller != nullptr ?
				sqlite3_bind_int64(statement, 4, device->controller->GetDeviceControllerID()) :
				sqlite3_bind_null(statement, 4)) != SQLITE_OK ||
			(device->room != nullptr ?
				sqlite3_bind_int64(statement, 5, device->room->GetRoomID()) :
				sqlite3_bind_null(statement, 5)) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to insert device into 'devices' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);
		return true;
	}
	bool Database::UpdateDevicePropName(Ref<Device> device, const std::string& value, const std::string& newValue)
	{
		assert(device != nullptr);

		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(update devices set name = ? where id = ?)", 40,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_text(statement, 1, newValue.data(), newValue.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 2, device->deviceID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to update device name.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);

		return true;
	}
	bool Database::UpdateDevicePropDeviceController(Ref<Device> device, Ref<DeviceController> value, Ref<DeviceController> newValue)
	{
		assert(device != nullptr);

		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(update devices set controllerid = ? where id = ?)", 48,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if ((newValue != nullptr ?
			sqlite3_bind_int64(statement, 1, newValue->GetDeviceControllerID()) :
			sqlite3_bind_null(statement, 1)) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 2, device->deviceID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to update device device controller.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);

		return true;
	}
	bool Database::UpdateDevicePropRoom(Ref<Device> device, Ref<Room> value, Ref<Room> newValue)
	{
		assert(device != nullptr);

		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(update devices set roomid = ? where id = ?)", 42,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if ((newValue != nullptr ?
			sqlite3_bind_int64(statement, 1, newValue->GetRoomID()) :
			sqlite3_bind_null(statement, 1)) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 2, device->deviceID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to update device room.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);

		return true;
	}
	bool Database::RemoveDevice(identifier_t deviceID)
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(delete from devices where id = ?)", 32,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_int64(statement, 1, deviceID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to delete device from 'devices' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);
		return true;
	}

	size_t Database::GetDeviceCount()
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(select count(*) from devices)", 28,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		if (sqlite3_step(statement) != SQLITE_ROW)
		{
			LOG_ERROR("Failing to count devices.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		size_t deviceCount = sqlite3_column_int64(statement, 0);

		sqlite3_finalize(statement);
		return deviceCount;
	}

	//! Action
	bool Database::LoadActions(const boost::function<void(identifier_t actionID, const std::string& name, identifier_t sourceID, identifier_t roomID, const std::string& data)>& callback)
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(select id, name, sourceid, roomid, data from actions)", 52,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			identifier_t actionID = sqlite3_column_int64(statement, 0);

			// Check values
			if (sqlite3_column_type(statement, 1) == SQLITE_NULL ||
				sqlite3_column_type(statement, 4) == SQLITE_NULL)
			{
				LOG_ERROR("Failing to load invalid action {0}", actionID);
				continue;
			}

			const unsigned char* name = sqlite3_column_text(statement, 1);
			size_t nameSize = sqlite3_column_bytes(statement, 1);

			identifier_t sourceID = sqlite3_column_int64(statement, 2);

			identifier_t roomID = sqlite3_column_int64(statement, 3);

			const unsigned char* data = sqlite3_column_text(statement, 4);
			size_t dataSize = sqlite3_column_bytes(statement, 4);

			// Check values
			if (name == nullptr ||
				data == nullptr)
			{
				LOG_ERROR("Failing to load invalid action properties {0}", actionID);
				continue;
			}

			callback(
				actionID, 
				std::string((const char*)name, nameSize), 
				sourceID, 
				roomID, 
				std::string((const char*)data, dataSize));
		}

		sqlite3_finalize(statement);
		return true;
	}

	identifier_t Database::ReserveAction()
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(insert into actions values)"
			R"(((select ifnull((select id+1 from actions where (id+1) not in (select id from actions) order by id asc limit 1), 1)),)"
			R"(null, 0, null, "{}"))", 163,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to insert action into 'actions' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		identifier_t actionID = sqlite3_last_insert_rowid(connection);

		sqlite3_finalize(statement);
		return actionID;
	}
	bool Database::UpdateAction(Ref<Action> action)
	{
		assert(action != nullptr);

		// Lock
		boost::lock(mutex, action->mutex);
		boost::lock_guard lock(mutex, boost::adopt_lock);
		boost::lock_guard lock2(action->mutex, boost::adopt_lock);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(replace into actions values (?, ?, ?, ?, "{}"))", 46,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_int64(statement, 1, action->actionID) != SQLITE_OK ||
			sqlite3_bind_text(statement, 2, action->name.data(), action->name.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 3, action->script->GetSourceID()) != SQLITE_OK ||
			(action->room != nullptr ?
				sqlite3_bind_int64(statement, 4, action->room->GetRoomID()) :
				sqlite3_bind_null(statement, 4)) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to insert action into 'actions' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);
		return true;
	}
	bool Database::UpdateActionPropName(Ref<Action> action, const std::string& value, const std::string& newValue)
	{
		assert(action != nullptr);

		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(update actions set name = ? where id = ?)", 40,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_text(statement, 1, newValue.data(), newValue.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 2, action->actionID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to update action name.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);

		return true;
	}
	bool Database::UpdateActionPropRoom(Ref<Action> action, Ref<Room> value, Ref<Room> newValue)
	{
		assert(action != nullptr);

		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(update actions set roomid = ? where id = ?)", 42,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if ((newValue != nullptr ?
			sqlite3_bind_int64(statement, 1, newValue->GetRoomID()) :
			sqlite3_bind_null(statement, 1)) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 2, action->actionID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to update device room.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);

		return true;
	}
	bool Database::RemoveAction(identifier_t actionID)
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(delete from actions where id = ?)", 32,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_int64(statement, 1, actionID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to delete action from 'actions' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);
		return true;
	}

	size_t Database::GetActionCount()
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(select count(*) from actions)", 28,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		if (sqlite3_step(statement) != SQLITE_ROW)
		{
			LOG_ERROR("Failing to count actions.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		size_t actionCount = sqlite3_column_int64(statement, 0);

		sqlite3_finalize(statement);
		return actionCount;
	}

	//! User
	bool Database::LoadUsers(const boost::function<void(identifier_t userID, const std::string& name, uint8_t hash[SHA256_DIGEST_LENGTH], uint8_t salt[SALT_LENGTH], UserAccessLevel accessLevel)>& callback)
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(select id, name, hash, salt, accesslevel, data from users)", 57,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			identifier_t userID = sqlite3_column_int64(statement, 0);

			// Check values
			if (sqlite3_column_type(statement, 1) == SQLITE_NULL ||
				sqlite3_column_type(statement, 2) == SQLITE_NULL ||
				sqlite3_column_type(statement, 3) == SQLITE_NULL ||
				sqlite3_column_type(statement, 4) == SQLITE_NULL ||
				sqlite3_column_type(statement, 5) == SQLITE_NULL)
			{
				LOG_ERROR("Failing to load invalid user {0}", userID);
				continue;
			}

			const unsigned char* name = sqlite3_column_text(statement, 1);
			size_t nameSize = sqlite3_column_bytes(statement, 1);

			const unsigned char* hash = sqlite3_column_text(statement, 2);
			size_t hashSize = sqlite3_column_bytes(statement, 2);

			const unsigned char* salt = sqlite3_column_text(statement, 3);
			size_t saltSize = sqlite3_column_bytes(statement, 3);

			const unsigned char* accessLevel = sqlite3_column_text(statement, 4);
			size_t accessLevelSize = sqlite3_column_bytes(statement, 4);

			const unsigned char* data = sqlite3_column_text(statement, 5);
			size_t dataSize = sqlite3_column_bytes(statement, 5);

			// Check values
			if (name == nullptr ||
				data == nullptr)
			{
				LOG_ERROR("Failing to load invalid user properties {0}", userID);
				continue;
			}

			// Decode hash
			std::vector<uint8_t> hashBuffer = cppcodec::base64_rfc4648::decode((const char*)hash, hashSize);
			if (hashBuffer.size() != SHA256_DIGEST_LENGTH)
			{
				LOG_ERROR("Failing to load invalid user hash {0}", userID);
				continue;
			}

			// Decode salt
			std::vector<uint8_t> saltBuffer = cppcodec::base64_rfc4648::decode((const char*)salt, saltSize);
			if (saltBuffer.size() != SALT_LENGTH)
			{
				LOG_ERROR("Failing to load invalid user salt {0}", userID);
				continue;
			}

			callback(
				userID, 
				std::string((const char*)name, nameSize), 
				hashBuffer.data(), 
				saltBuffer.data(), 
				ParseUserAccessLevel(std::string((const char*)accessLevel, accessLevelSize)));
		}

		sqlite3_finalize(statement);
		return true;
	}

	identifier_t Database::ReserveUser()
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(insert into users values)"
			R"(((select ifnull((select id+1 from users where (id+1) not in (select id from users) order by id asc limit 1), 1)),)"
			R"(null, "", "", "restricted", "{}"))", 170,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to insert user into 'users' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		identifier_t deviceID = sqlite3_last_insert_rowid(connection);

		sqlite3_finalize(statement);
		return deviceID;
	}
	bool Database::UpdateUser(Ref<User> user)
	{
		assert(user != nullptr);

		// Lock
		boost::lock(mutex, user->mutex);
		boost::lock_guard lock(mutex, boost::adopt_lock);
		boost::lock_guard lock2(user->mutex, boost::adopt_lock);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(replace into users values (?, ?, ?, ?, ?, "{}"))", 47,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		std::string hash = cppcodec::base64_rfc4648::encode(user->hash, SHA256_DIGEST_LENGTH);
		std::string salt = cppcodec::base64_rfc4648::encode(user->salt, SALT_LENGTH);

		std::string accessLevel = server::StringifyUserAccessLevel(user->accessLevel);

		if (sqlite3_bind_int64(statement, 1, user->userID) != SQLITE_OK ||
			sqlite3_bind_text(statement, 2, user->name.data(), user->name.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_text(statement, 3, hash.data(), hash.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_text(statement, 4, salt.data(), salt.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_text(statement, 5, accessLevel.data(), accessLevel.size(), nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to insert user into 'users' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);
		return true;
	}
	bool Database::UpdateUserPropName(Ref<User> user, const std::string& value, const std::string& newValue)
	{
		assert(user != nullptr);

		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(update users set name = ? where id = ?)", 40,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_text(statement, 1, newValue.data(), newValue.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 2, user->userID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to update user name.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);

		return true;
	}
	bool Database::UpdateUserPropAccessLevel(Ref<User> user, UserAccessLevel value, UserAccessLevel newValue)
	{
		assert(user != nullptr);

		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(update users set accesslevel = ? where id = ?)", 45,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		std::string accessLevel = StringifyUserAccessLevel(newValue);

		if (sqlite3_bind_text(statement, 1, accessLevel.data(), accessLevel.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 2, user->userID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to update user accesslevel.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);

		return true;
	}
	bool Database::UpdateUserPropHash(Ref<User> user, uint8_t newValue[SHA256_DIGEST_LENGTH])
	{
		assert(user != nullptr);

		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(update users set hash = ? where id = ?)", 38,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		std::string hash = cppcodec::base64_rfc4648::encode(newValue, SHA256_DIGEST_LENGTH);

		if (sqlite3_bind_text(statement, 1, hash.data(), hash.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 2, user->userID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to update user hash.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);

		return true;
	}
	bool Database::UpdateUserPropSalt(Ref<User> user, uint8_t newValue[SALT_LENGTH])
	{
		assert(user != nullptr);

		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(update users set salt = ? where id = ?)", 38,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		std::string salt = cppcodec::base64_rfc4648::encode(newValue, SALT_LENGTH);

		if (sqlite3_bind_text(statement, 1, salt.data(), salt.size(), nullptr) != SQLITE_OK ||
			sqlite3_bind_int64(statement, 2, user->userID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to update user salt.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);

		return true;
	}
	bool Database::RemoveUser(identifier_t userID)
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(delete from users where id = ?)", 30,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_bind_int64(statement, 1, userID) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		if (sqlite3_step(statement) != SQLITE_DONE)
		{
			LOG_ERROR("Failing to delete user from 'users' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);
		return true;
	}

	size_t Database::GetUserCount()
	{
		// Lock
		boost::lock_guard lock(mutex);

		// Insert into database
		sqlite3_stmt* statement;

		if (sqlite3_prepare_v2(connection,
			R"(select count(*) from users)", 26,
			&statement, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		if (sqlite3_step(statement) != SQLITE_ROW)
		{
			LOG_ERROR("Failing to count users.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return 0;
		}

		size_t userCount = sqlite3_column_int64(statement, 0);

		sqlite3_finalize(statement);
		return userCount;
	}
}