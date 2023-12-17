#include "pch.h"
#include "scanner.inl"

static size_t AsHexadecimal(char ch) {
	if (ch >= '0' && ch <= '9') {
		return ch - '0';
	} else if (ch >= 'A' && ch <= 'F') {
		return ch - 'A' + 10;
	} else if (ch >= 'a' && ch <= 'f') {
		return ch - 'a' + 10;
	}
	throw std::runtime_error("invalid hexadecimal escape");
}

static std::string* MakeString(char const* p) {
	static std::unordered_map<char, char> map = {
		{ 'a', '\a' },
		{ 'b', '\b' },
		{ 'f', '\f' },
		{ 'n', '\n' },
		{ 'r', '\r' },
		{ 't', '\t' },
		{ 'v', '\v' },
	};
	auto* rv = new std::string();
	rv->reserve(std::strlen(p + 2));
	while (++p, p[1]) {
		if (*p != '\\') {
			*rv += *p;
		} else {
			char ch = *++p;
			auto it = map.find(ch);
			if (it != map.end()) {
				*rv += it->second;
			} else if (ch == 'o') {
				if (p[1] < '0' || p[1] > '7') {
					throw std::runtime_error("invalid octal escape");
				}
				int value = *++p - '0';
				for (char const* q = p + 2; p < q && p[1] >= '0' && p[1] <= '7'; ++p) {
					value <<= 3;
					value += p[1] - '0';
				}
				if (value > 0377) {
					throw std::runtime_error("invalid octal escape");
				}
				*rv += static_cast<char>(value);
			} else if (ch == 'x') {
				size_t value = (AsHexadecimal(p[1]) << 4) | AsHexadecimal(p[2]);
				p += 2;
				*rv += static_cast<char>(value);
			} else if (ch == 'u') {
				size_t value = (AsHexadecimal(p[1]) << 12) | (AsHexadecimal(p[2]) << 8) | (AsHexadecimal(p[3]) << 4) | AsHexadecimal(p[4]);
				if (value < 128) {
					*rv += static_cast<char>(value);
				} else if (value < 2'048) {
					*rv += static_cast<char>(0xc0 | (value >> 6));
					*rv += static_cast<char>(0x80 | (value & 0x3f));
				} else {
					*rv += static_cast<char>(0xe0 | (value >> 12));
					*rv += static_cast<char>(0x80 | ((value >> 6) & 0x3f));
					*rv += static_cast<char>(0x80 | (value & 0x3f));
				}
				p += 4;
			} else if (ch == 'U') {
				size_t value = (AsHexadecimal(p[1]) << 28) | (AsHexadecimal(p[2]) << 24) | (AsHexadecimal(p[3]) << 20) | (AsHexadecimal(p[4]) << 16) | (AsHexadecimal(p[5]) << 12) | (AsHexadecimal(p[6]) << 8) | (AsHexadecimal(p[7]) << 4) | AsHexadecimal(p[8]);
				if (value < 128) {
					*rv += static_cast<char>(value);
				} else if (value < 2'048) {
					*rv += static_cast<char>(0xc0 | (value >> 6));
					*rv += static_cast<char>(0x80 | (value & 0x3f));
				} else if (value < 65'536) {
					*rv += static_cast<char>(0xe0 | (value >> 12));
					*rv += static_cast<char>(0x80 | ((value >> 6) & 0x3f));
					*rv += static_cast<char>(0x80 | (value & 0x3f));
				} else if (value < 1'114'112) {
					*rv += static_cast<char>(0xf0 | (value >> 18));
					*rv += static_cast<char>(0x80 | ((value >> 12) & 0x3f));
					*rv += static_cast<char>(0x80 | ((value >> 6) & 0x3f));
					*rv += static_cast<char>(0x80 | (value & 0x3f));
				} else {
					throw std::runtime_error("invalid Unicode escape");
				}
				p += 8;
			} else {
				*rv += *p;
			}
		}
	}
	return rv;
}
