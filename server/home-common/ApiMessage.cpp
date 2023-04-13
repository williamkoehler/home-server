#include "ApiMessage.hpp"

namespace server
{
    void ApiRequestMessage::Build(size_t id, rapidjson::Writer<rapidjson::StringBuffer>& writer) const
    {
        writer.StartObject();

        // Message id field
        writer.Key("msgid", 5);
        writer.Uint64(id);

        // Message type field
        writer.Key("msg", 3);
        writer.String(type.data(), type.size(), true);

        // Content field
        writer.Key("content", 7);
        document.Accept(writer);

        writer.EndObject(3);
    }

    void ApiResponseMessage::Build(size_t id, rapidjson::Writer<rapidjson::StringBuffer>& writer) const
    {
        writer.StartObject();

        // Message id field
        writer.Key("msgid", 5);
        writer.Uint64(id);

        // Message type field
        writer.Key("msg", 3);

        if (errorCode == kApiErrorCode_NoError)
            writer.String("ack", 3);
        else
            writer.String("nack", 4);

        // Content field
        writer.Key("content", 7);
        document.Accept(writer);

        writer.EndObject(3);
    }
}