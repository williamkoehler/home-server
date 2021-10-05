#pragma once
#include "../common.hpp"
#include "Mime.hpp"

namespace server
{
	struct WebPageFile
	{
		size_t size;
		char* data;

		FileTypes fileType;
	};

	class WebPageFiles
	{
	private:
		static boost::unordered::unordered_map<std::string, WebPageFile> fileList;

	public:
		static void LoadWebPage(boost::filesystem::path directoryPath);

		static WebPageFile* GetFile(const char* target);
		static WebPageFile *GetFile(std::string target);
	};
}