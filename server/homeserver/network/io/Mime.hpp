#pragma once
#include "../../common.hpp"

namespace server
{
	enum FileTypes
	{
		kHTMLFileType,
		kCSSFileType,
		kJSFileType,
		kJSONFileType,
		kXMLFileType,
		kPNGFileType,
		kSVGFileType,
		kJPGFileType,
		kICONFileType,
		kTEXTFileType,
	};
	FileTypes GetFileTypeFromExt(const char* ext);
	const char* GetExtFromFileType(FileTypes type);
	FileTypes GetFileTypeFromIdentifier(const char* identifier);
	const char* GetIdentifierFromFileType(FileTypes type);
	std::string GetMimeFromFileType(FileTypes type);
}