#include "Mime.h"

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

	FileTypes GetFileTypeFromExt(const char* ext)
	{
		if (strncmp(ext, ".htm", 4) == 0)
			return FileTypes::kHTMLFileType;
		else if (strncmp(ext, ".css", 4) == 0)
			return FileTypes::kCSSFileType;
		else if (strncmp(ext, ".js", 3) == 0)
			return FileTypes::kJSFileType;

		else if (strncmp(ext, ".json", 5) == 0)
			return FileTypes::kHTMLFileType;
		else if (strncmp(ext, ".xml", 4) == 0)
			return FileTypes::kHTMLFileType;

		else if (strncmp(ext, ".png", 4) == 0)
			return FileTypes::kPNGFileType;
		else if (strncmp(ext, ".svg", 4) == 0)
			return FileTypes::kSVGFileType;
		else if (strncmp(ext, ".jpg", 4) == 0)
			return FileTypes::kJPGFileType;
		else if (strncmp(ext, ".jpe", 4) == 0)
			return FileTypes::kJPGFileType;
		else if (strncmp(ext, ".ico", 4) == 0)
			return FileTypes::kICONFileType;
		else
			return FileTypes::kTEXTFileType;
	}

	const char* GetExtFromFileType(FileTypes type)
	{
		return fileExts[type];
	}

	FileTypes GetFileTypeFromIdentifier(const char* identifier)
	{
		if (strncmp(identifier, "html", 4) == 0)
			return FileTypes::kHTMLFileType;
		else if (strncmp(identifier, "css", 3) == 0)
			return FileTypes::kCSSFileType;
		else if (strncmp(identifier, "javascript", 10) == 0)
			return FileTypes::kJSFileType;

		else if (strncmp(identifier, "json", 4) == 0)
			return FileTypes::kHTMLFileType;
		else if (strncmp(identifier, "xml", 3) == 0)
			return FileTypes::kHTMLFileType;

		else if (strncmp(identifier, "png", 3) == 0)
			return FileTypes::kPNGFileType;
		else if (strncmp(identifier, "svg", 3) == 0)
			return FileTypes::kSVGFileType;
		else if (strncmp(identifier, "jpg", 3) == 0)
			return FileTypes::kJPGFileType;
		else if (strncmp(identifier, "icon", 4) == 0)
			return FileTypes::kICONFileType;
		else
			return FileTypes::kTEXTFileType;
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