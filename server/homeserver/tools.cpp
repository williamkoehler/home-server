#include "common.h"
#include "tools.h"

uint8_t lookupTable[256] = {
	123, 251, 67, 250, 161, 0, 107, 97, 241, 111, 181, 82, 249, 33, 69, 55,
	221, 31, 209, 182, 143, 92, 149, 184, 148, 62, 113, 65, 37, 27, 106, 166,
	189, 205, 199, 128, 176, 19, 211, 236, 127, 192, 231, 70, 233, 88, 146, 44,
	98, 6, 85, 150, 36, 23, 112, 164, 135, 207, 169, 5, 26, 64, 165, 219,
	90, 168, 156, 203, 177, 120, 2, 190, 188, 7, 100, 185, 174, 243, 162, 10,
	237, 18, 253, 225, 8, 208, 172, 244, 255, 126, 101, 79, 145, 235, 228, 121,
	183, 201, 22, 83, 13, 214, 116, 109, 159, 32, 95, 226, 140, 220, 57, 12,
	3, 14, 204, 72, 21, 41, 56, 66, 28, 193, 40, 217, 25, 54, 179, 117,
	238, 87, 240, 155, 180, 170, 242, 212, 191, 163, 78, 218, 137, 194, 175, 110,
	61, 20, 68, 89, 130, 63, 52, 102, 24, 229, 132, 245, 80, 216, 195, 115,
	197, 96, 210, 45, 16, 227, 248, 202, 51, 152, 252, 125, 81, 206, 215, 186,
	39, 158, 178, 187, 131, 136, 1, 49, 50, 17, 141, 91, 47, 129, 60, 99,
	154, 35, 86, 171, 105, 34, 38, 200, 147, 58, 77, 118, 173, 246, 76, 254,
	59, 153, 29, 9, 213, 167, 84, 93, 30, 46, 94, 75, 151, 114, 73, 222,
	133, 232, 196, 144, 98, 124, 53, 4, 108, 74, 223, 234, 134, 230, 157, 139,
	43, 119, 224, 71, 122, 142, 42, 160, 104, 48, 247, 103, 15, 11, 138, 239
};

uint8_t pearson8(std::string input)
{
	const char* in = input.data();
	const size_t size = input.size();

	uint8_t h1 = lookupTable[in[0]];
	for (size_t j = 1; j < size; j++)
		h1 = lookupTable[h1 ^ in[j]];

	return static_cast<int8_t>(h1);
}
uint8_t pearson8(const char* input, size_t size)
{
	uint8_t h1 = lookupTable[input[0]];
	for (size_t j = 1; j < size; j++)
		h1 = lookupTable[h1 ^ input[j]];

	return static_cast<int8_t>(h1);
}
uint16_t pearson16(std::string input)
{
	const char* in = input.data();
	const size_t size = input.size();

	uint8_t h1 = lookupTable[in[0]];
	uint8_t h2 = lookupTable[in[0] + 1];
	for (size_t j = 1; j < size; j++)
	{
		h1 = lookupTable[h1 ^ in[j]];
		h2 = lookupTable[h2 ^ in[j]];
	}

	return ((static_cast<int16_t>(h1) << 8) | static_cast<int16_t>(h2));
}
uint16_t pearson16(const char* input, size_t size)
{
	uint8_t h1 = lookupTable[input[0]];
	uint8_t h2 = lookupTable[input[0] + 1];
	for (size_t j = 1; j < size; j++)
	{
		h1 = lookupTable[h1 ^ input[j]];
		h2 = lookupTable[h2 ^ input[j]];
	}

	return ((static_cast<int16_t>(h1) << 8) | static_cast<int16_t>(h2));
}
uint32_t pearson32(std::string input)
{
	const char* in = input.data();
	const size_t size = input.size();

	uint8_t h1 = lookupTable[in[0]];
	uint8_t h2 = lookupTable[in[0] + 1];
	uint8_t h3 = lookupTable[in[0] + 2];
	uint8_t h4 = lookupTable[in[0] + 3];
	for (size_t j = 1; j < size; j++)
	{
		h1 = lookupTable[h1 ^ in[j]];
		h2 = lookupTable[h2 ^ in[j]];
		h3 = lookupTable[h3 ^ in[j]];
		h4 = lookupTable[h4 ^ in[j]];
	}
	
	return ((static_cast<int32_t>(h1) << 24) | (static_cast<int32_t>(h2) << 16) | (static_cast<int32_t>(h3) << 8) | static_cast<int32_t>(h4));
}
uint32_t pearson32(const char* input, size_t size)
{
	uint8_t h1 = lookupTable[input[0]];
	uint8_t h2 = lookupTable[input[0] + 1];
	uint8_t h3 = lookupTable[input[0] + 2];
	uint8_t h4 = lookupTable[input[0] + 3];
	for (size_t j = 1; j < size; j++)
	{
		h1 = lookupTable[h1 ^ input[j]];
		h2 = lookupTable[h2 ^ input[j]];
		h3 = lookupTable[h3 ^ input[j]];
		h4 = lookupTable[h4 ^ input[j]];
	}

	return ((static_cast<int32_t>(h1) << 24) | (static_cast<int32_t>(h2) << 16) | (static_cast<int32_t>(h3) << 8) | static_cast<int32_t>(h4));
}
uint64_t pearson64(std::string input)
{
	const char* in = input.data();
	const size_t size = input.size();

	uint8_t h1 = lookupTable[in[0]];
	uint8_t h2 = lookupTable[in[0] + 1];
	uint8_t h3 = lookupTable[in[0] + 2];
	uint8_t h4 = lookupTable[in[0] + 3];
	uint8_t h5 = lookupTable[in[0] + 4];
	uint8_t h6 = lookupTable[in[0] + 5];
	uint8_t h7 = lookupTable[in[0] + 6];
	uint8_t h8 = lookupTable[in[0] + 7];
	for (size_t j = 1; j < size; j++)
	{
		h1 = lookupTable[h1 ^ in[j]];
		h2 = lookupTable[h2 ^ in[j]];
		h3 = lookupTable[h3 ^ in[j]];
		h4 = lookupTable[h4 ^ in[j]];
		h5 = lookupTable[h5 ^ in[j]];
		h6 = lookupTable[h6 ^ in[j]];
		h7 = lookupTable[h7 ^ in[j]];
		h8 = lookupTable[h8 ^ in[j]];
	}

	return ((static_cast<uint64_t>(h1) << 56) |
		(static_cast<uint64_t>(h2) << 48) |
		(static_cast<uint64_t>(h3) << 40) |
		(static_cast<uint64_t>(h4) << 32) |
		(static_cast<uint64_t>(h5) << 24) |
		(static_cast<uint64_t>(h6) << 16) |
		(static_cast<uint64_t>(h7) << 8) |
		static_cast<uint64_t>(h8));
}
uint64_t pearson64(const char* input, size_t size)
{
	uint8_t h1 = lookupTable[input[0]];
	uint8_t h2 = lookupTable[input[0] + 1];
	uint8_t h3 = lookupTable[input[0] + 2];
	uint8_t h4 = lookupTable[input[0] + 3];
	uint8_t h5 = lookupTable[input[0] + 4];
	uint8_t h6 = lookupTable[input[0] + 5];
	uint8_t h7 = lookupTable[input[0] + 6];
	uint8_t h8 = lookupTable[input[0] + 7];
	for (size_t j = 1; j < size; j++)
	{
		h1 = lookupTable[h1 ^ input[j]];
		h2 = lookupTable[h2 ^ input[j]];
		h3 = lookupTable[h3 ^ input[j]];
		h4 = lookupTable[h4 ^ input[j]];
		h5 = lookupTable[h5 ^ input[j]];
		h6 = lookupTable[h6 ^ input[j]];
		h7 = lookupTable[h7 ^ input[j]];
		h8 = lookupTable[h8 ^ input[j]];
	}

	return ((static_cast<uint64_t>(h1) << 56) | 
		(static_cast<uint64_t>(h2) << 48) | 
		(static_cast<uint64_t>(h3) << 40) | 
		(static_cast<uint64_t>(h4) << 32) | 
		(static_cast<uint64_t>(h5) << 24) | 
		(static_cast<uint64_t>(h6) << 16) | 
		(static_cast<uint64_t>(h7) << 8) | 
		static_cast<uint64_t>(h8));
}

std::string PrettyBytes(size_t bytes)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(2);

	if (bytes >= 1000000000000ull)
		ss << bytes / 1000000000000.0 << "TB";
	else if (bytes >= 1000000000ull)
		ss << bytes / 1000000000.0 << "GB";
	else if (bytes >= 1000000ull)
		ss << bytes / 1000000.0 << "MB";
	else if (bytes >= 1000ull)
		ss << bytes / 1000.0 << "KB";
	else
		ss << bytes << "B";

	return ss.str();
}
