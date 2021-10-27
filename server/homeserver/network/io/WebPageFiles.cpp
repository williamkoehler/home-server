#include "WebPageFiles.hpp"
#include "../../tools.hpp"

namespace server
{
	boost::unordered::unordered_map<std::string, WebPageFile> WebPageFiles::fileList;

	void WebPageFiles::LoadWebPage(boost::filesystem::path directoryPath)
	{
		LOG_INFO("Loading website from directory '{0}'", directoryPath.string());

		if(boost::filesystem::exists(directoryPath))
		{
			for (boost::filesystem::directory_entry fileEntry : boost::filesystem::recursive_directory_iterator(directoryPath))
			{
				if (boost::filesystem::is_regular_file(fileEntry))
				{
					boost::filesystem::path filePath = fileEntry.path();

					//URL name
					boost::system::error_code ec;
					std::string relative = boost::filesystem::relative(filePath, directoryPath, ec).string();
					if (ec)
						continue;

					relative = boost::replace(relative, '\\', '/');
					relative.insert(relative.begin(), '/');

					WebPageFile file = {};

					//File type
					if (filePath.has_extension())
						file.fileType = GetFileTypeFromExt(filePath.extension().string().c_str());
					else
						file.fileType = FileTypes::kTEXTFileType;

					//Load file
					boost::beast::file stream;
					stream.open(filePath.string().c_str(), boost::beast::file_mode::scan, ec);
					if (ec)
						continue;

					file.size = stream.size(ec);
					if (ec)
						continue;

					file.data = new char[file.size];
					stream.read(file.data, file.size, ec);
					if (ec)
					{
						SAFE_DELETE_ARRAY(file.data);
						continue;
					}
					stream.close(ec);

					fileList[relative] = file;

					LOG_INFO("Adding file '{0}':({1}, {2})", relative, PrettyBytes(file.size), GetIdentifierFromFileType(file.fileType));
				}
			}
		}
		else
		{
			LOG_ERROR("Could not find website directory {0}", directoryPath.string());
		}
	}
	WebPageFile *WebPageFiles::GetFile(const char* target)
	{
		boost::unordered::unordered_map<std::string, WebPageFile>::iterator it = fileList.find(target);
		if (it == fileList.end())
			return nullptr;

		return &it->second;
	}
	WebPageFile* WebPageFiles::GetFile(std::string target)
	{
		boost::unordered::unordered_map<std::string, WebPageFile>::iterator it = fileList.find(target);
		if (it == fileList.end())
			return nullptr;

		return &it->second;
	}
}