#pragma once
#include "../../common.hpp"

inline uint16_t pseudoRand(uint32_t& seed)
{
    seed = (214013 * seed + 2531011);
    return (seed >> 16) & 0x7FFF;
}

std::string GenerateRandomString(size_t size, uint32_t seed)
{
    std::string output = std::string(size, ' ');

    for (size_t i = 0; i < size; i++)
        output[i] = (char)pseudoRand(seed);

    return output;
}

std::string GenerateReadableRandomString(size_t size, uint32_t seed)
{
    static char characters[100] = {
        'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E',  'f', 'F', 'g', 'G', 'h', 'H', 'i', 'I', 'j', 'J',
        'k', 'K', 'l', 'L', 'm', 'M', 'n', 'N', 'o', 'O',  'p', 'P', 'q', 'Q', 'r', 'R', 's', 'S', 't', 'T',
        'u', 'U', 'v', 'V', 'w', 'W', 'x', 'X', 'y', 'Y',  'z', 'Z', '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', ',', ';', ':', '?', '!', '.', '"', '\'', '_', '-', '+', '=', '*', '/', '#', '%', '$', '(',
        ')', '[', ']', '{', '}', '&', '@', '<', '>', '|',  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    };

    std::string output = std::string(size, ' ');

    for (size_t i = 0; i < size; i++)
        output[i] = characters[pseudoRand(seed) % 100];

    return output;
}