#include "Method.hpp"

namespace server
{
    namespace scripting
    {
        namespace native
        {
            Method::Method() : method(nullptr), parameterType(ValueType::kUnknownType)
            {
            }
            Method::Method(void* method, ValueType parameterType) : method(method), parameterType(parameterType)
            {
            }
            Method::~Method()
            {
            }
        }
    }
}