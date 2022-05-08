#pragma once
#include "../../common.hpp"

inline uint16_t pseudoRand(uint32_t& seed)
{
    seed = (214013 * seed + 2531011);
    return (seed >> 16) & 0x7FFF;
}

std::string GenerateString(size_t size, uint32_t seed)
{
    std::string output = std::string(size, ' ');

    for (size_t i = 0; i < size; i++)
        output[i] = (char)pseudoRand(seed);

    return output;
}