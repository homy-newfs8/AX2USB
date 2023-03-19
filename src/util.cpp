#include "util.h"
#include <cstddef>
#include <string>

namespace util {
static constexpr const std::string_view WHITESPACES = " ";

std::string
ltrim(const std::string& str) {
	auto pos = str.find_first_not_of(WHITESPACES);
	return pos == str.npos ? std::string() : str.substr(pos);
}

std::string
rtrim(const std::string& str) {
	auto pos = str.find_last_not_of(WHITESPACES);
	return pos == str.npos ? std::string() : str.substr(0, pos + 1);
}

std::string
trim(const std::string& str) {
	return rtrim(ltrim(str));
}

// static std::string::size_type
// skip_blanks(const std::string& str, std::string::size_type pos) {
// 	while (pos != str.npos && str[pos] == ' ') {
// 		pos++;
// 	}
// 	return pos;
// }

int8_t
nibble(char c) {
	if (c >= '0' && c <= '9') {
		return c - '0';
	}
	if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	}
	if (c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	}
	return -1;
}

char
hexchar(int b, bool upper) {
	if (b >= 0 && b <= 9) {
		return '0' + b;
	}
	if (b >= 10 && b <= 15) {
		return (upper ? 'A' : 'a') + (b - 10);
	}
	return 0;
}

std::string
bin2hex(const std::byte* data, size_t data_size, bool upper) {
	std::string out(data_size * 2, 0);
	for (size_t i = 0; i < data_size; i++) {
		out[i * 2] = hexchar(std::to_integer<uint8_t>(data[i]) >> 4, upper);
		out[i * 2 + 1] = hexchar(std::to_integer<uint8_t>(data[i]) & 0x0f, upper);
	}

	return out;
}

}  // namespace util
