#include "Property.hpp"

namespace server
{
    namespace scripting
    {
        namespace native
        {
            Property::Property() : type(ValueType::kUnknownType), getter(nullptr), setter(nullptr)
            {
            }
            Property::Property(ValueType type, void* getter, void* setter)
                : type(type), getter(getter), setter(setter)
            {
            }
            Property::~Property()
            {
            }
        }
    }
}