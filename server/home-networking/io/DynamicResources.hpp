#pragma once
#include "../common.hpp"
#include "Mime.hpp"

namespace server
{
    namespace networking
    {
        struct MemoryBuffer
        {
            uint8_t* data = nullptr;
            size_t size = 0;
        };

        struct Resource
        {
            std::string name = "";
            MemoryBuffer buffer;
        };

        class DynamicResources
        {
          private:
            boost::shared_mutex mutex;
            boost::unordered::unordered_map<uint32_t, Ref<Resource>> resourceList;

          public:
            DynamicResources();
            virtual ~DynamicResources();
            static Ref<DynamicResources> Create();
            static Ref<DynamicResources> GetInstance();

            bool HasResource(std::string name);

            Ref<Resource> AddResourceFromMemory(std::string prefix, std::string name, uint8_t* data, size_t size);
            Ref<Resource> AddResourceFromMemoryWithoutCopy(std::string prefix, std::string name, uint8_t* data,
                                                           size_t size);

            Ref<Resource> UpdateResourceFromMemory(std::string prefix, std::string name, uint8_t* data, size_t size);
            Ref<Resource> UpdateResourceFromMemoryWithoutCopy(std::string prefix, std::string name, uint8_t* data,
                                                              size_t size);

            Ref<Resource> GetFile(std::string prefix, std::string name);

            void RemoveResource(std::string prefix, std::string name);

            void Load();
            void Save();
        };
    }
}