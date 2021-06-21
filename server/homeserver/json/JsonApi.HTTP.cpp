#include "JsonApi.h"
#include "../home/Home.h"
#include "../home/DeviceManager.h"
#include "../home/Room.h"
#include "../home/Device.h"
#include "../user/UserManager.h"
#include "../user/User.h"
#include <Script.h>
#include "../io/DynamicResources.h"

#include "../dependencies/stbi.h"

namespace server
{
	void JsonApi::BuildJsonErrorMessageHTTP(const char* error, rapidjson::Document& output)
	{
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();
		output.AddMember("error", rapidjson::Value(error, allocator), allocator);
	}

	bool JsonApi::ProcessResApiCallHTTP(boost::beast::http::verb verb, std::string_view target, const Ref<User>& user, std::string_view input, rapidjson::StringBuffer& output, std::string& contentType)
	{
		switch (verb)
		{
		case boost::beast::http::verb::patch: // Get
		case boost::beast::http::verb::get:
		{
			if (strncmp(target.data(), "user/photo", 10) == 0)
			{
				target.remove_prefix(10); // Skip user/photo/
				// Now the target should only contain the id or nothing

				std::string id;

				if (target.size() > 1 && target.data()[0] == '/')
				{
					target.remove_prefix(1); // Skip / 
					id = std::string(target);
				}
				else
					id = std::to_string(user->GetUserID());

				Ref<DynamicResources> resources = DynamicResources::GetInstance();

				assert(resources != nullptr);

				Ref<Resource> resource = resources->GetFile("user_photo", id);

				if (resource == nullptr)
					return false;

				if (resource->buffer.size == 0)
					return false;

				output.Push(resource->buffer.size);
				memcpy((void*)output.GetString(), resource->buffer.data, resource->buffer.size);

				contentType = "image/png";

				return true;
			}

			return false;
		}
		case boost::beast::http::verb::put: // Set
		{
			if (strncmp(target.data(), "user/photo", 10) == 0)
			{
				if (input.size() == 0)
					return false;

				// Decode image and resize it if needed
				int x, y, channels;

				stbi_uc* raw = stbi_load_from_memory((const stbi_uc*)input.data(), input.size(), &x, &y, &channels, 4);
				if (raw == nullptr)
					return false;

				// Check if resizing is needed
				if ((x * y) > (128 * 128))
				{
					int x2, y2;

					if (x > y)
					{
						x2 = 128;
						y2 = static_cast<int>(static_cast<double>(y) * 128.0 / static_cast<double>(x));
					}
					else
					{
						x2 = static_cast<int>(static_cast<double>(x) * 128.0 / static_cast<double>(y));
						y2 = 128;
					}

					stbi_uc* raw2 = static_cast<stbi_uc*>(malloc(x2 * y2 * 4));

					if (stbir_resize_uint8(raw, x, y, 0, raw2, x2, y2, 0, 4) == 0)
					{
						stbi_image_free(raw);
						stbi_image_free(raw2);
						return false;
					}

					// Delete old image and replace it with the resized image
					stbi_image_free(raw);
					raw = raw2;

					// Set correct dimensions
					x = x2;
					y = y2;
				}

				// Write image to memory
				MemoryBuffer image;

				const int result = stbi_write_png_to_func(
					[](void* context, void* data, int size) -> void
					{
						MemoryBuffer* image = static_cast<MemoryBuffer*>(context);
						image->data = new uint8_t[size];
						image->size = size;
						memcpy(image->data, data, size);
					}, &image,
					x, y, 4, raw, x * 4);

				stbi_image_free(raw); // Delete image

				// Check for errors
				if (result == 0)
				{
					SAFE_DELETE_ARRAY(image.data);
					return false;
				}

				// Save image
				Ref<DynamicResources> resources = DynamicResources::GetInstance();

				assert(resources != nullptr);

				const Ref<Resource> resource = resources->UpdateResourceFromMemoryWithoutCopy("user_photo", std::to_string(user->GetUserID()), image.data, image.size);
				if (resource == nullptr)
					return false;

				return true;
			}

			return false;
		}
		default:
			return false;
		}

		return true;
	}
}