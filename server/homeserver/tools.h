#pragma once
#include <iostream>

uint8_t pearson8(std::string input);
uint8_t pearson8(const char *input, size_t size);
uint16_t pearson16(std::string input);
uint16_t pearson16(const char *input, size_t size);
uint32_t pearson32(std::string input);
uint32_t pearson32(const char *input, size_t size);
uint64_t pearson64(std::string input);
uint64_t pearson64(const char *input, size_t size);

std::string PrettyBytes(size_t bytes);