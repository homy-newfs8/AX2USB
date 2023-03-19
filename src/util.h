#pragma once
#include <array>
#include <cstring>
#include <string>

namespace util {

std::string ltrim(const std::string& str);
std::string rtrim(const std::string& str);
std::string trim(const std::string& str);

int8_t nibble(char c);
char hexchar(int b, bool upper = false);

template <size_t N>
bool
hex2bin(const std::string& str, std::array<std::byte, N>& out, size_t& out_len) {
	if ((str.length() & 1) == 1 || str.length() > N * 2) {
		return false;
	}
	size_t olen = str.length() / 2;
	for (int i = 0; i < olen; i++) {
		int8_t n1 = nibble(str[i * 2]);
		int8_t n2 = nibble(str[i * 2 + 1]);
		if (n1 < 0 || n2 < 0) {
			return false;
		}
		out[i] = std::byte{static_cast<uint8_t>((n1 << 4) + n2)};
	}
	out_len = olen;
	return true;
}
std::string bin2hex(const std::byte* data, size_t data_size, bool upper = false);

}  // namespace util
