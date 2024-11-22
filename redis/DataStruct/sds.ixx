export module sds;

import std;

constexpr size_t SDS_MAX_PREALLOC = 1024 * 1024;

export enum class sds_type : std::uint8_t {
	SDS_UNINIT = 0,
	SDS_TYPE_8 = 1,
	SDS_TYPE_16 = 2,
	SDS_TYPE_32 = 4,
	SDS_TYPE_64 = 8,
};

#pragma pack(push, 1)
export template <typename T>
	requires std::is_same_v<T, std::uint8_t> || std::is_same_v<T, std::uint16_t> ||
			 std::is_same_v<T, std::uint32_t> || std::is_same_v<T, std::uint64_t>
struct sdshdr {
	T len;
	T alloc;
	sds_type type;
	char buf[1] = { '\0' };
};
#pragma pack(pop)

#pragma pack(push, 1)
export class sds {
public:
	sds_type type;
	char buf[1] = { '\0' };

public:
	static void destroy(sds* s);
	static sds* create(const char* str, size_t len, size_t alloc);

	size_t length();
	size_t capacity();
	size_t available();

	sds* dilatation(size_t add_len);

	sds* copy(sds* str);
	sds* copy(const char* str, size_t len);

	sds* append(sds* str);
	sds* append(const char* str, size_t len);
};
#pragma pack(pop)

template <typename Func, typename Ret = std::invoke_result_t<Func, sdshdr<std::uint64_t>*>>
inline auto access_sdshdr(sds* str, Func&& operation) -> Ret;