#include "Database.hpp"
#include "../home/Room.hpp"
#include "../home/DeviceController.hpp"
#include "../home/Device.hpp"

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
	Ref<Database> Database::Create()
	{
		if (!instanceDatabase.expired())
			return Ref<Database>(instanceDatabase);

		Ref<Database> database = boost::make_shared<Database>();
		if (database == nullptr)
			return nullptr;

		instanceDatabase = database;

		// Open / Create database
		if (sqlite3_open_v2("home.sqlite3", &database->connection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK)
		{
			LOG_ERROR("Failing to open/create database.\n{0}", sqlite3_errcode(database->connection));
			return nullptr;
		}

		// Create necessary tables
		char* err = nullptr;

		// Home
		{
			// Rooms table
			if (sqlite3_exec(database->connection,
							 R"(create table if not exists rooms)"
							 R"((id integer not null primary key, name text not null, type text not null))",
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
							 R"((id integer not null primary key, name text not null, pluginid integer not null, roomid integer not null, data text not null, foreign key(roomid) references rooms(id)))",
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
							 R"((id integer not null primary key, name text not null, pluginid integer not null, controllerid integer not null, roomid integer not null, data text not null, foreign key (controllerid) references devicecontrollers (id), foreign key (roomid) references rooms (id)))",
							 nullptr,
							 nullptr,
							 &err) != SQLITE_OK)
			{
				LOG_ERROR("Failing to create 'devices' table.\n{0}", err);
				return nullptr;
			}
		}

		return database;
	}
	Ref<Database> Database::GetInstance()
	{
		return Ref<Database>(instanceDatabase);
	}

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
			std::string name = (const char*)sqlite3_column_text(statement, 1);
			std::string type = (const char*)sqlite3_column_text(statement, 2);

			callback(roomID, name, type);
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
							   R"("unknown room", "no type"))", 165,
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
	bool Database::UpdateRoomPropName(Ref<Room> room, std::string& value, const std::string& newValue)
	{
		// Lock
		boost::lock(mutex, room->mutex);
		boost::lock_guard lock(mutex, boost::adopt_lock);
		boost::lock_guard lock2(room->mutex, boost::adopt_lock);

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

		// Update value
		value = newValue;

		return true;
	}
	bool Database::UpdateRoomPropType(Ref<Room> room, std::string& value, const std::string& newValue)
	{
		// Lock
		boost::lock(mutex, room->mutex);
		boost::lock_guard lock(mutex, boost::adopt_lock);
		boost::lock_guard lock2(room->mutex, boost::adopt_lock);

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

		// Update value
		value = newValue;

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
			std::string name = (const char*)sqlite3_column_text(statement, 1);
			identifier_t pluginID = sqlite3_column_int64(statement, 2);
			identifier_t roomID = sqlite3_column_int64(statement, 3);
			std::string data = (const char*)sqlite3_column_text(statement, 4);

			callback(controllerID, name, pluginID, roomID, data);
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
							   R"("unknown devicecontroller", 0, null, "{}"))", 215,
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

		identifier_t controllerID = sqlite3_last_insert_rowid(connection);

		sqlite3_finalize(statement);
		return controllerID;
	}
	bool Database::UpdateDeviceController(Ref<DeviceController> controller)
	{
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
	bool Database::UpdateDeviceControllerPropName(Ref<DeviceController> controller, std::string& value, const std::string& newValue)
	{
		// Lock
		boost::lock(mutex, controller->mutex);
		boost::lock_guard lock(mutex, boost::adopt_lock);
		boost::lock_guard lock2(controller->mutex, boost::adopt_lock);

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

		// Update value
		value = newValue;

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
			LOG_ERROR("Failing to delete device from 'devicecontrollers' table.\n{0}", sqlite3_errmsg(connection));
			sqlite3_finalize(statement);
			return false;
		}

		sqlite3_finalize(statement);
		return true;
	}

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
			std::string name = (const char*)sqlite3_column_text(statement, 1);
			identifier_t pluginID = sqlite3_column_int64(statement, 2);
			identifier_t controllerID = sqlite3_column_int64(statement, 3);
			identifier_t roomID = sqlite3_column_int64(statement, 4);
			std::string data = (const char*)sqlite3_column_text(statement, 5);

			callback(deviceID, name, pluginID, controllerID, roomID, data);
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
							   R"("unknown device", 0, null, null, "{}"))", 181,
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

		identifier_t deviceID = sqlite3_last_insert_rowid(connection);

		sqlite3_finalize(statement);
		return deviceID;
	}
	bool Database::UpdateDevice(Ref<Device> device)
	{
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
	bool Database::UpdateDevicePropName(Ref<Device> device, std::string& value, const std::string& newValue)
	{
		// Lock
		boost::lock(mutex, device->mutex);
		boost::lock_guard lock(mutex, boost::adopt_lock);
		boost::lock_guard lock2(device->mutex, boost::adopt_lock);

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

		// Update value
		value = newValue;

		return true;
	}
	bool Database::UpdateDevicePropDeviceController(Ref<Device> device, Ref<DeviceController>& value, Ref<DeviceController> newValue)
	{
		// Lock
		boost::lock(mutex, device->mutex);
		boost::lock_guard lock(mutex, boost::adopt_lock);
		boost::lock_guard lock2(device->mutex, boost::adopt_lock);

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

		// Update value
		value = newValue;

		return true;
	}
	bool Database::UpdateDevicePropRoom(Ref<Device> device, Ref<Room>& value, Ref<Room> newValue)
	{
		// Lock
		boost::lock(mutex, device->mutex);
		boost::lock_guard lock(mutex, boost::adopt_lock);
		boost::lock_guard lock2(device->mutex, boost::adopt_lock);

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

		// Update value
		value = newValue;

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
}