#include "cpu.h"

int main()
{
	using base_t = cu::arch_x86_64_cache_base;
	using u64_t = std::uint64_t;
	using u32_t = std::uint32_t;
	using u16_t = std::uint16_t;
	using u8_t = std::uint8_t;

	cu::test::abc_print();

	cu::test::print_cache_params<u64_t, base_t>();
	cu::test::print_cache_params<u32_t, base_t>();
	cu::test::print_cache_params<u16_t, base_t>();
	cu::test::print_cache_params<u8_t, base_t>();

	system("pause");

	return 0;
}