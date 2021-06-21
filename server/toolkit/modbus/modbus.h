#pragma once
#include <iostream>
#include <string.h>
#include <cstdint>
#ifdef _WIN32
typedef intptr_t ssize_t;
#endif

#ifdef _WIN32
#ifdef MODBUS_COMPILE
#define MODBUS_EXPORT __declspec(dllexport)
#else
#define MODBUS_EXPORT __declspec(dllimport)
#endif
#else
#define MODBUS_EXPORT
#endif

enum ModbusExceptions
{
    //Modbus
    IllegalFunction = 0x01,
    IllegalAddress = 0x02,
    IllegalValue = 0x03,
    ServerFailure = 0x04,
    Acknowledge = 0x05,
    ServerBusy = 0x06,
    NegativeAcknowledge = 0x07,
    MemoryPartyError = 0x08,
    NoConnection = 0x09, // Not defined by modbus
    GatewayPathUnavailable = 0x0A,
    GatewayTargetDeviceFailedToRespond = 0x0B,
};

class MODBUS_EXPORT Modbus
{
private:
    std::string host;
    uint16_t port;
    int sock;
    uint16_t msgID;
    uint8_t slaveID;

    void BuildRequest(uint8_t *msg, uint16_t address, uint8_t function);

    uint16_t SendReadRequest(uint16_t address, uint16_t amount, uint8_t function);
    uint16_t SendWriteRequest(uint16_t address, uint16_t amount, uint8_t function, uint16_t *value);

    ssize_t SendRequest(uint8_t *msg, int length);
    ssize_t ReceiveResponse(uint8_t *buffer, uint16_t transactionID);

    uint8_t HandleError(uint8_t *msg, uint8_t function);

public:
    Modbus();
    ~Modbus();

    inline bool IsConnected() { return sock != 0; }

    void SetEndpoint(std::string host, uint16_t port);
    bool Connect();
         
    void SetSlaveID(uint8_t id);
         
    uint8_t ReadHoldingRegisters(uint16_t address, uint16_t amount, uint16_t *buffer);
    uint8_t ReadInputRegisters(uint16_t address, uint16_t amount, uint16_t *buffer);
    uint8_t ReadCoils(uint16_t address, uint16_t amount, bool* buffer);
    uint8_t ReadInputBits(uint16_t address, uint16_t amount, bool* buffer);
    
    uint8_t WriteCoil(uint16_t address, bool to_write);
    uint8_t WriteRegister(uint16_t address, uint16_t value);
    uint8_t WriteCoils(uint16_t address, uint16_t amount, bool* value );
    uint8_t WriteRegisters(uint16_t address, uint16_t amount, uint16_t *value);
    
    void Close();

    const char* GetException(uint8_t exception);
};
