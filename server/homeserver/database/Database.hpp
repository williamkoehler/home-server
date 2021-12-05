#include "../common.hpp"
#include "../scripting/ScriptSource.hpp"

namespace server
{
	class Room;
	class DeviceController;
	class Device;
	class Action;

	enum class UserAccessLevel;
	class User;

	class Database : public boost::enable_shared_from_this<Database>
	{
	private:
		boost::mutex mutex;
		sqlite3* connection;

	public:
		Database();
		~Database();
		static Ref<Database> Create();
		static Ref<Database> GetInstance();

		//! ScriptSource

		/// @brief Load scripts from database
		/// @param callback Callback for each user
		/// @return Successfulness
		bool LoadScriptSources(const boost::function<void(identifier_t sourceID, const std::string& name, ScriptUsage usage, ScriptLanguage language, const std::string_view& data)>& callback);

		/// @brief Reserves new script source entry in database
		/// @return Entry identifier
		identifier_t ReserveScriptSource();

		/// @brief Update script source without pushing record
		/// @param user Script source to update
		/// @return Successfulness
		bool UpdateScriptSource(Ref<ScriptSource> scriptSource);

		/// @brief Update script source name 
		/// @param scriptSource Script source to update
		/// @param value Old name (for record)
		/// @param newValue New name
		/// @return Successfulness
		bool UpdateScriptSourcePropName(Ref<ScriptSource> scriptSource, const std::string& value, const std::string& newValue);

		/// @brief Update script source data 
		/// @param scriptSource Script source to update
		/// @param newValue New data
		/// @return Successfulness
		bool UpdateScriptSourcePropData(Ref<ScriptSource> scriptSource, const std::string_view& newValue);
		bool RemoveScriptSource(identifier_t sourceID);

		size_t GetScriptSourceCount();

		//! Room

		/// @brief Load rooms from database
		/// @param callback Callback for each room
		/// @return Successfulness
		bool LoadRooms(const boost::function<void(identifier_t roomID, const std::string& name, const std::string& type)>& callback);

		/// @brief Reserves new room entry in database
		/// @return Entry identifier
		identifier_t ReserveRoom();

		/// @brief Update room without pushing record
		/// @param room Room to update
		/// @return Successfulness
		bool UpdateRoom(Ref<Room> room);

		/// @brief Update room name 
		/// @param room Room to update
		/// @param value Old name (for record)
		/// @param newValue New name
		/// @return Successfulness
		bool UpdateRoomPropName(Ref<Room> room, const std::string& value, const std::string& newValue);
		bool UpdateRoomPropType(Ref<Room> room, const std::string& value, const std::string& newValue);
		bool RemoveRoom(identifier_t roomID);

		size_t GetRoomCount();

		//! DeviceController

		/// @brief Load device controllers from database
		/// @param callback Callback for each device controller
		/// @return Successfulness
		bool LoadDeviceControllers(const boost::function<void(identifier_t controllerID, const std::string& name, identifier_t pluginID, identifier_t roomID, const std::string& data)>& callback);

		/// @brief Reserves new device controller entry in database
		/// @return Entry identifier
		identifier_t ReserveDeviceController();

		/// @brief Update device controller without pushing record
		/// @param room Device controller to update
		/// @return Successfulness
		bool UpdateDeviceController(Ref<DeviceController> controller);

		/// @brief Update device controller name 
		/// @param controller Device controller to update
		/// @param value Old name (for record)
		/// @param newValue New name
		/// @return Successfulness
		bool UpdateDeviceControllerPropName(Ref<DeviceController> controller, const std::string& value, const std::string& newValue);
		bool UpdateDeviceControllerPropRoom(Ref<DeviceController> controller, Ref<Room> value, Ref<Room> newValue);
		bool RemoveDeviceController(identifier_t controllerID);

		size_t GetDeviceControllerCount();

		//! Device

		/// @brief Load device from database
		/// @param callback Callback for each device
		/// @return Successfulness
		bool LoadDevices(const boost::function<void(identifier_t deviceID, const std::string& name, identifier_t pluginID, identifier_t controllerID, identifier_t roomID, const std::string& data)>& callback);

		/// @brief Reserves new device entry in database
		/// @return Entry identifier
		identifier_t ReserveDevice();

		/// @brief Update device without pushing record
		/// @param room Device to update
		/// @return Successfulness
		bool UpdateDevice(Ref<Device> device);

		/// @brief Update device name 
		/// @param device Device to update
		/// @param value Old name (for record)
		/// @param newValue New name
		/// @return Successfulness
		bool UpdateDevicePropName(Ref<Device> device, const std::string& value, const std::string& newValue);
		bool UpdateDevicePropDeviceController(Ref<Device> device, Ref<DeviceController> value, Ref<DeviceController> newValue);
		bool UpdateDevicePropRoom(Ref<Device> device, Ref<Room> value, Ref<Room> newValue);
		bool RemoveDevice(identifier_t deviceID);

		size_t GetDeviceCount();

		//! Action

		/// @brief Load action from database
		/// @param callback Callback for each action
		/// @return Successfulness
		bool LoadActions(const boost::function<void(identifier_t actionID, const std::string& name, identifier_t sourceID, identifier_t roomID, const std::string& data)>& callback);

		/// @brief Reserves new action entry in database
		/// @return Entry identifier
		identifier_t ReserveAction();

		/// @brief Update action without pushing record
		/// @param room Action to update
		/// @return Successfulness
		bool UpdateAction(Ref<Action> action);

		/// @brief Update action name 
		/// @param action Action to update
		/// @param value Old name (for record)
		/// @param newValue New name
		/// @return Successfulness
		bool UpdateActionPropName(Ref<Action> action, const std::string& value, const std::string& newValue);
		bool UpdateActionPropRoom(Ref<Action> action, Ref<Room> value, Ref<Room> newValue);
		bool RemoveAction(identifier_t actionID);

		size_t GetActionCount();

		//! User

		/// @brief Load users from database
		/// @param callback Callback for each user
		/// @return Successfulness
		bool LoadUsers(const boost::function<void(identifier_t userID, const std::string& name, uint8_t hash[SHA256_DIGEST_LENGTH], uint8_t salt[SALT_LENGTH], UserAccessLevel accessLevel)>& callback);

		/// @brief Reserves new user entry in database
		/// @return Entry identifier
		identifier_t ReserveUser();

		/// @brief Update user without pushing record
		/// @param user User to update
		/// @return Successfulness
		bool UpdateUser(Ref<User> user);

		/// @brief Update user name 
		/// @param script User to update
		/// @param value Old name (for record)
		/// @param newValue New name
		/// @return Successfulness
		bool UpdateUserPropName(Ref<User> user, const std::string& value, const std::string& newValue);
		bool UpdateUserPropAccessLevel(Ref<User> user, UserAccessLevel value, UserAccessLevel newValue);
		bool UpdateUserPropHash(Ref<User> user, uint8_t value[SHA256_DIGEST_LENGTH], uint8_t newValue[SHA256_DIGEST_LENGTH]);
		bool RemoveUser(identifier_t userID);

		size_t GetUserCount();
	};
}