#include "Mime.hpp"
#include "../../tools.hpp"

namespace server
{
	const char* mimeTypes[] = {
		"text/html",
		"text/css",
		"text/javascript",
		"application/json",
		"application/xml",
		"image/png",
		"image/svg+xml",
		"image/jpeg",
		"image/x-icon",
		"text/plain",
	};

	const char* fileExts[] = {
		".html",
		".css",
		".js",
		".json",
		".xml",
		".png",
		".svg",
		".jpg",
		".ico",
		".txt",
	};
	const char* fileIdentifiers[] = {
		"html",
		"css",
		"javascript",
		"json",
		"xml",
		"png",
		"svg",
		"jpg",
		"icon",
		"text",
	};

	FileTypes GetFileTypeFromExt(const std::string& ext)
	{
		switch (crc32(ext.data(), ext.size()))
		{
		case CRC32(".htm"):
		case CRC32(".html"):
			return FileTypes::kHTMLFileType;
		case CRC32(".css"):
			return FileTypes::kHTMLFileType;
		case CRC32(".js"):
			return FileTypes::kHTMLFileType;
		case CRC32(".json"):
			return FileTypes::kJSONFileType;
		case CRC32(".xml"):
			return FileTypes::kXMLFileType;
		case CRC32(".png"):
			return FileTypes::kPNGFileType;
		case CRC32(".jpeg"):
		case CRC32(".jpg"):
			return FileTypes::kJPGFileType;
		case CRC32(".svg"):
			return FileTypes::kSVGFileType;
		case CRC32(".ico"):
			return FileTypes::kICONFileType;
		default:
			return FileTypes::kTEXTFileType;
		}
	}

	const char* GetExtFromFileType(FileTypes type)
	{
		return fileExts[type];
	}

	FileTypes GetFileTypeFromIdentifier(const std::string& identifier)
	{
		switch (crc32(identifier.data(), identifier.size()))
		{
		case CRC32("html"):
			return FileTypes::kHTMLFileType;
		case CRC32("css"):
			return FileTypes::kHTMLFileType;
		case CRC32("javascript"):
			return FileTypes::kHTMLFileType;
		case CRC32("json"):
			return FileTypes::kJSONFileType;
		case CRC32("xml"):
			return FileTypes::kXMLFileType;
		case CRC32("png"):
			return FileTypes::kPNGFileType;
		case CRC32("jpg"):
			return FileTypes::kJPGFileType;
		case CRC32("svg"):
			return FileTypes::kSVGFileType;
		case CRC32("icon"):
			return FileTypes::kICONFileType;
		default:
			return FileTypes::kTEXTFileType;
		}
	}

	const char* GetIdentifierFromFileType(FileTypes type)
	{
		return fileIdentifiers[type];
	}

	std::string GetMimeFromFileType(FileTypes type)
	{
		return mimeTypes[type];
	}
}