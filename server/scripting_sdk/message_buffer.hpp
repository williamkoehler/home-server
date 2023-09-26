#pragma once
#include <cstring>
#include <inttypes.h>
#include <memory>
#include <string_view>
#include <utility>

void PrintData(uint8_t* data, uint16_t dataLength);

enum class FieldType : uint8_t
{
    // Fixed size
    kNull = 0x01,
    kFalse = 0x02,
    kTrue = 0x03,
    kInt8 = 0x04,
    kInt16 = 0x05,
    kInt32 = 0x06,
    kInt64 = 0x07,
    kUint8 = 0x08,
    kUint16 = 0x09,
    kUint32 = 0x0a,
    kUint64 = 0x0b,
    kSingle = 0x0c,
    kDouble = 0x0d,

    // Variable size
    kString = 0x0e,
    kArray = 0x0f,
    kVariant = 0x10,
    kStructure = 0x11,
};

class MessageBuffer final
{
  public:
    uint16_t capacity;
    uint16_t size;
    uint8_t* buffer;

    // inline constexpr size_t GetFieldTypeSize(FieldType type)
    // {
    //     switch (type)
    //     {
    //     case FieldType::kNull:
    //         return 0;
    //     case FieldType::kTrue:
    //         return 0;
    //     case FieldType::kFalse:
    //         return 0;
    //     case FieldType::kInt8:
    //         return sizeof(int8_t);
    //     case FieldType::kInt16:
    //         return sizeof(int16_t);
    //     case FieldType::kInt32:
    //         return sizeof(int32_t);
    //     case FieldType::kInt64:
    //         return sizeof(int64_t);
    //     case FieldType::kUInt8:
    //         return sizeof(uint8_t);
    //     case FieldType::kUInt16:
    //         return sizeof(uint16_t);
    //     case FieldType::kUInt32:
    //         return sizeof(uint32_t);
    //     case FieldType::kUInt64:
    //         return sizeof(uint64_t);
    //     case FieldType::kSingle:
    //         return sizeof(float);
    //     case FieldType::kDouble:
    //         return sizeof(double);
    //     default:
    //         return 0;
    //     }
    // }

  public:
    MessageBuffer() : capacity(5), size(0), buffer(new uint8_t[capacity])
    {
    }
    MessageBuffer(const MessageBuffer&) = delete;
    MessageBuffer(MessageBuffer&& message_buffer) noexcept
        : capacity(std::exchange(message_buffer.capacity, 0)), size(std::exchange(message_buffer.size, 0)),
          buffer(std::exchange(message_buffer.buffer, nullptr))
    {
    }
    ~MessageBuffer()
    {
        if (buffer != nullptr)
            delete[] buffer;
    }

    void Reserve(uint16_t new_capacity)
    {
        // Allocate new buffer
        uint8_t* new_buffer = new uint8_t[new_capacity];

        if (buffer != nullptr)
        {
            // Copy data
            std::memcpy((void*)new_buffer, (void*)buffer, size);

            // Free old buffer
            delete[] buffer;
        }

        // Set new buffer
        capacity = new_capacity;
        buffer = new_buffer;
    }

    inline uint16_t GetSize() const
    {
        return size;
    }

    inline void SetSize(uint16_t new_size)
    {
        if (new_size > capacity)
            Reserve(new_size);

        size = new_size;
    }

    inline uint8_t* GetBuffer() const
    {
        return buffer;
    }
};

/// @brief Calculate value max size
///
/// @tparam args Types
template <typename... args>
struct StaticMax;

template <typename arg>
struct StaticMax<arg>
{
    static const size_t value = sizeof(arg);
};

template <typename arg1, typename arg2, typename... args>
struct StaticMax<arg1, arg2, args...>
{
    static const size_t value =
        sizeof(arg1) >= sizeof(arg2) ? StaticMax<arg1, args...>::value : StaticMax<arg2, args...>::value;
};