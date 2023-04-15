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
        for (rapidjson::Value::ConstMemberIterator memberIt = document.MemberBegin(); memberIt != document.MemberEnd();
             memberIt++)
        {
            writer.Key(memberIt->name.GetString(), memberIt->name.GetStringLength());
            memberIt->value.Accept(writer);
        }

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
        for (rapidjson::Value::ConstMemberIterator memberIt = document.MemberBegin(); memberIt != document.MemberEnd();
             memberIt++)
        {
            writer.Key(memberIt->name.GetString(), memberIt->name.GetStringLength());
            memberIt->value.Accept(writer);
        }

        writer.EndObject(3);
    }
}