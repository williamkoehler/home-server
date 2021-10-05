#include "modbus.hpp"
#include <cassert>
#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#endif

#define MAX_MSG_LENGTH 512

enum ModbusFunctions
{
	ReadCoils = 0x01,
	ReadInputBits = 0x02,
	ReadRegisters = 0x03,
	ReadInputRegisters = 0x04,
	WriteCoil = 0x05,
	WriteRegister = 0x06,
	WriteCoils = 0x0F,
	WriteRegisters = 0x10,
};

Modbus::Modbus()
	: host("127.0.0.1"), port(502), sock(0), msgID(0), slaveID(1)
{
}
Modbus::~Modbus()
{
	Close();
}

void Modbus::SetEndpoint(std::string h, uint16_t p)
{
	assert(!h.empty() && p != 0);
	host = h;
	port = p;
}

bool Modbus::Connect() {
	assert(!host.empty() && port != 0);

	Close();

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		sock = 0;
		return false;
	}

	//Set socket to non-blocking
#ifdef _WIN32
	size_t mode = 1;
	if (ioctlsocket(sock, FIONBIO, (u_long*)&mode) < 0)
#else
	int opt = fcntl(sock, F_GETFL, nullptr);
	if (opt < 0 ||
		fcntl(sock, F_SETFL, opt | O_NONBLOCK))
#endif
	{
		Close();
		return false;
	}

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(host.c_str());
	address.sin_port = htons(port);

	if (connect(sock, (struct sockaddr*) & address, sizeof(address)) < 0 &&
#ifdef _WIN32
		WSAGetLastError() != WSAEWOULDBLOCK)
#else
		errno != EINPROGRESS)
#endif
	{
		Close();
		return false;
	}

#ifdef _WIN32
	mode = 0;
	if (ioctlsocket(sock, FIONBIO, (u_long*)&mode) < 0)
#else
	if (fcntl(sock, F_SETFL, opt) < 0)
#endif
	{
		Close();
		return false;
	}

	fd_set set;
	FD_ZERO(&set);
	FD_SET(sock, &set);

	timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	if (select(sock + 1, nullptr, &set, nullptr, &timeout) <= 0)
	{
		Close();
		return false;
	}

	msgID = 0;

	return true;
}
void Modbus::SetSlaveID(uint8_t id)
{
	slaveID = id;
}

const char* Modbus::GetException(uint8_t exception)
{
	static const char* exceptions[] =
	{
		"No error",
		"Illegal function",
		"Illegal address",
		"Illegal balue",
		"Server failure",
		"Acknowledge",
		"Server or slave busy",
		"Negative acknowledge",
		"Memory party error",
		"No connection",
		"Gateway path unavailable",
		"Gateway target device failed to respond",
	};
	if (exception < 12)
		return exceptions[exception];
	else
		return exceptions[0];
}

void Modbus::BuildRequest(uint8_t* msg, uint16_t address, uint8_t function) {
	msg[0] = static_cast<uint8_t>(msgID >> 8);
	msg[1] = static_cast<uint8_t>(msgID & 0x00FF);
	msg[2] = 0;
	msg[3] = 0;
	msg[4] = 0;
	msg[6] = slaveID;
	msg[7] = function;
	msg[8] = static_cast<uint8_t>(address >> 8);
	msg[9] = static_cast<uint8_t>(address & 0x00FF);
}

uint16_t Modbus::SendReadRequest(uint16_t address, uint16_t amount, uint8_t function)
{
	if (msgID < UINT16_MAX)
		msgID++;
	else
		msgID = 1;

	uint8_t msg[12];
	BuildRequest(msg, address, function);
	msg[5] = 6;
	msg[10] = static_cast<uint8_t>(amount >> 8);
	msg[11] = static_cast<uint8_t>(amount & 0x00FF);
	if (SendRequest(msg, 12) <= 0)
		return 0;

	return msgID;
}
uint16_t Modbus::SendWriteRequest(uint16_t address, uint16_t amount, uint8_t function, uint16_t* value)
{
	if (msgID < UINT16_MAX)
		msgID++;
	else
		msgID = 1;

	switch (function)
	{
	case ModbusFunctions::WriteCoil:
	case ModbusFunctions::WriteRegister:
	{
		uint8_t msg[12];
		BuildRequest(msg, address, function);
		msg[5] = 6;
		msg[10] = static_cast<uint8_t>(value[0] >> 8);
		msg[11] = static_cast<uint8_t>(value[0] & 0x00FF);

		SendRequest(msg, 12);
	}break;
	case ModbusFunctions::WriteRegisters:
	{
		uint8_t msg[MAX_MSG_LENGTH];
		BuildRequest(msg, address, ModbusFunctions::WriteRegisters);
		msg[5] = static_cast<uint8_t>(5 + 2 * amount);
		msg[10] = static_cast<uint8_t>(amount >> 8);
		msg[11] = static_cast<uint8_t>(amount & 0x00FF);
		msg[12] = static_cast<uint8_t>(2 * amount);

		for (int i = 0; i < amount; i++)
		{
			msg[13 + 2 * i] = static_cast<uint8_t>(value[i] >> 8);
			msg[14 + 2 * i] = static_cast<uint8_t>(value[i] & 0x00FF);
		}

		SendRequest(msg, 13 + 2 * amount);
	}break;
	case ModbusFunctions::WriteCoils:
	{
		uint8_t msg[MAX_MSG_LENGTH];
		BuildRequest(msg, address, ModbusFunctions::WriteCoils);
		msg[5] = static_cast<uint8_t>(7 + (amount - 1) / 8);
		msg[10] = static_cast<uint8_t>(amount >> 8);
		msg[11] = static_cast<uint8_t>(amount >> 8);
		msg[12] = static_cast<uint8_t>((amount + 7) / 8);

		for (int i = 0; i < amount; i++)
			msg[13 + (i - 1) / 8] += static_cast<uint8_t>(value[i] << (i % 8));

		SendRequest(msg, 14 + (amount - 1) / 8);
	}break;
	}

	return msgID;
}

uint8_t Modbus::ReadCoils(uint16_t address, uint16_t amount, bool* buffer)
{
	if (sock == 0)
		return NoConnection;

	uint16_t transactionID = SendReadRequest(address, amount, ModbusFunctions::ReadCoils);
	if (!transactionID)
		return NoConnection;

	uint8_t msg[MAX_MSG_LENGTH];
	ssize_t receivedBytes = ReceiveResponse(msg, transactionID);

	if (receivedBytes <= 0)
		return NoConnection;

	uint8_t exception = HandleError(msg, ModbusFunctions::ReadCoils);
	if (exception)
		return exception;

	uint8_t* data = msg + 9;

	for (int i = 0; i < amount; i++)
		buffer[i] = static_cast<bool>((data[i / 8] >> (i % 8)) & 0x01);

	return 0;
}
uint8_t Modbus::ReadInputBits(uint16_t address, uint16_t amount, bool* buffer)
{
	if (sock == 0)
		return NoConnection;

	uint16_t transactionID = SendReadRequest(address, amount, ModbusFunctions::ReadInputBits);
	if (!transactionID)
		return NoConnection;

	uint8_t msg[MAX_MSG_LENGTH];
	ssize_t receivedBytes = ReceiveResponse(msg, transactionID);

	if (receivedBytes <= 0)
		return NoConnection;

	uint8_t exception = HandleError(msg, ModbusFunctions::ReadInputBits);
	if (exception)
		return exception;

	uint8_t* data = msg + 9;

	for (int i = 0; i < amount; i++)
		buffer[i] = static_cast<bool>((data[i / 8] >> (i % 8)) & 0x01);

	return 0;
}
uint8_t Modbus::ReadHoldingRegisters(uint16_t address, uint16_t amount, uint16_t* buffer)
{
	if (sock == 0)
		return NoConnection;

	uint16_t transactionID = SendReadRequest(address, amount, ModbusFunctions::ReadRegisters);
	if (!transactionID)
		return NoConnection;

	uint8_t msg[MAX_MSG_LENGTH];
	ssize_t receivedBytes = ReceiveResponse(msg, transactionID);

	if (receivedBytes <= 0)
		return NoConnection;

	uint8_t exception = HandleError(msg, ModbusFunctions::ReadRegisters);
	if (exception)
		return exception;

	uint8_t* data = msg + 9;

	for (int i = 0; i < amount; i++)
		buffer[i] = (static_cast<uint16_t>(data[i * 2]) << 8) | static_cast<uint16_t>(data[i * 2 + 1]);

	return 0;
}
uint8_t Modbus::ReadInputRegisters(uint16_t address, uint16_t amount, uint16_t* buffer)
{
	if (sock == 0)
		return NoConnection;

	uint16_t transactionID = SendReadRequest(address, amount, ModbusFunctions::ReadInputRegisters);
	if (!transactionID)
		return NoConnection;

	uint8_t msg[MAX_MSG_LENGTH];
	ssize_t receivedBytes = ReceiveResponse(msg, transactionID);

	if (receivedBytes <= 0)
		return NoConnection;

	uint8_t exception = HandleError(msg, ModbusFunctions::ReadInputRegisters);
	if (exception)
		return exception;

	uint8_t* data = msg + 9;

	for (int i = 0; i < amount; i++)
		buffer[i] = (static_cast<uint16_t>(data[i * 2]) << 8) | static_cast<uint16_t>(data[i * 2 + 1]);

	return 0;
}

uint8_t Modbus::WriteCoil(uint16_t address, bool value)
{
	if (sock == 0)
		return NoConnection;

	uint16_t v = value * 0xFF00;
	uint16_t transactionID = SendWriteRequest(address, 1, ModbusFunctions::WriteCoil, &v);
	if (!transactionID)
		return NoConnection;

	uint8_t msg[MAX_MSG_LENGTH];
	ssize_t receivedBytes = ReceiveResponse(msg, transactionID);

	if (receivedBytes <= 0)
		return NoConnection;

	return HandleError(msg, ModbusFunctions::WriteCoil);
}
uint8_t Modbus::WriteRegisters(uint16_t address, uint16_t amount, uint16_t* value)
{
	if (sock == 0)
		return NoConnection;

	uint16_t transactionID = SendWriteRequest(address, amount, ModbusFunctions::WriteRegisters, value);
	if (!transactionID)
		return NoConnection;

	uint8_t msg[MAX_MSG_LENGTH];
	ssize_t receivedBytes = ReceiveResponse(msg, transactionID);

	if (receivedBytes <= 0)
		return NoConnection;

	return HandleError(msg, ModbusFunctions::WriteRegisters);
}
uint8_t Modbus::WriteCoils(uint16_t address, uint16_t amount, bool* value)
{
	if (sock == 0)
		return NoConnection;

	uint16_t temp[128];
	for (int i = 0; i < amount; i++) {
		temp[i] = static_cast<uint16_t>(value[i]);
	}
	uint16_t transactionID = SendWriteRequest(address, amount, ModbusFunctions::WriteCoils, temp);
	if (!transactionID)
		return NoConnection;

	uint8_t msg[MAX_MSG_LENGTH];
	ssize_t receivedBytes = ReceiveResponse(msg, transactionID);

	if (receivedBytes <= 0)
		return NoConnection;

	return HandleError(msg, ModbusFunctions::WriteCoils);
}
uint8_t Modbus::WriteRegister(uint16_t address, uint16_t value)
{
	if (sock == 0)
		return NoConnection;

	uint16_t transactionID = SendWriteRequest(address, 1, ModbusFunctions::WriteRegister, &value);
	if (!transactionID)
		return NoConnection;

	uint8_t msg[MAX_MSG_LENGTH];
	ssize_t receivedBytes = ReceiveResponse(msg, transactionID);

	if (receivedBytes <= 0)
		return NoConnection;

	return HandleError(msg, ModbusFunctions::WriteRegister);
}

ssize_t Modbus::SendRequest(uint8_t* msg, int length)
{
	return send(sock, (const char*)msg, static_cast<size_t>(length), 0);
}

enum class ReceiveState
{
	ReceiveHeader,
	ReceiveBody,
};

ssize_t Modbus::ReceiveResponse(uint8_t* msg, uint16_t transactionID)
{
	ReceiveState state = ReceiveState::ReceiveHeader;

	ssize_t receivedBytes = 0;
	ssize_t leftBytes = 8;

	fd_set set = {};
	timeval timeout = {};

	FD_ZERO(&set);
	FD_SET(sock, &set);
	do
	{
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;

		if (select(sock + 1, &set, nullptr, nullptr, &timeout) < 0)
			return -1;

		ssize_t r = recv(sock, (char*)msg + receivedBytes, static_cast<int>(leftBytes), 0);
		if (r <= 0)
			return -1;

		receivedBytes += r;
		leftBytes -= r;

		if (leftBytes == 0)
		{
			switch (state)
			{
			case ReceiveState::ReceiveHeader:
			{
				//Verify transaction id
				if (transactionID != (static_cast<uint16_t>(msg[0]) << 8 | static_cast<uint16_t>(msg[1])))
					return -1;

				//Verfiy protocol
				if (msg[2] != 0 || msg[3] != 0)
					return -1;

				//Verify unit identifier
				if (msg[6] != slaveID)
					return -1;

				//Verify length
				leftBytes = static_cast<ssize_t>(static_cast<uint16_t>(msg[4]) << 8 | static_cast<uint16_t>(msg[5])) - 2;
				if (leftBytes <= 0)
					return -1;

				state = ReceiveState::ReceiveBody;
			}break;
			default:
				break;
			}
		}
	} while (leftBytes > 0);

	return receivedBytes;
}

uint8_t Modbus::HandleError(uint8_t* msg, uint8_t function)
{
	if (msg[7] == function + 0x80)
		return msg[8];
	return 0;
}

void Modbus::Close()
{
	if (sock)
	{
#ifdef _WIN32
		closesocket(sock);
#else
		close(sock);
#endif
		sock = 0;
	}
}
