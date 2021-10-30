#include "../common.hpp"

namespace server
{
	class Room;
	class DeviceController;
	class Device;

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

		//! Home

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
		bool UpdateRoomPropName(Ref<Room> room, const std::string& value, const std::string& newValue);
		bool UpdateRoomPropType(Ref<Room> room, const std::string& value, const std::string& newValue);
		bool RemoveRoom(identifier_t roomID);

		size_t GetRoomCount();

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
		bool UpdateDeviceControllerPropName(Ref<DeviceController> controller, const std::string& value, const std::string& newValue);
		bool UpdateDeviceControllerPropRoom(Ref<DeviceController> controller, Ref<Room> value, Ref<Room> newValue);
		bool RemoveDeviceController(identifier_t controllerID);

		size_t GetDeviceControllerCount();

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
		bool UpdateDevicePropName(Ref<Device> device, const std::string& value, const std::string& newValue);
		bool UpdateDevicePropDeviceController(Ref<Device> device, Ref<DeviceController> value, Ref<DeviceController> newValue);
		bool UpdateDevicePropRoom(Ref<Device> device, Ref<Room> value, Ref<Room> newValue);
		bool RemoveDevice(identifier_t deviceID);

		size_t GetDeviceCount();

		//! Users

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
		bool UpdateUserPropName(Ref<User> user, const std::string& value, const std::string& newValue);
		bool UpdateUserPropAccessLevel(Ref<User> user, UserAccessLevel value, UserAccessLevel newValue);
		bool UpdateUserPropHash(Ref<User> user, uint8_t value[SHA256_DIGEST_LENGTH], uint8_t newValue[SHA256_DIGEST_LENGTH]);
		bool RemoveUser(identifier_t userID);

		size_t GetUserCount();
	};
}