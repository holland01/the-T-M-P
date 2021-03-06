#include "cpu.h"
#include <array>

int main()
{
	using base_t = cu::arch_x86_64_cache_base;
	using u64_t = std::uint64_t;
	using u32_t = std::uint32_t;
	using u16_t = std::uint16_t;
	using u8_t = std::uint8_t;

	using arr_t  = std::array<uint32_t, 16>;

	std::cout << CU_SIZEOF_STRING(arr_t) << std::endl;

	std::cout << "\n\nnon-cached\n";
	cu::test::vertex_array_test_benchmark_t va{};
	va.run(cu::test::vertex_array_test, false, CU_DEFAULT_IN_ITERATIONS);

	std::cout << "cache memory\n";
	cu::test::vertex_cmem_test_benchmark_t vb{};
	vb.run(cu::test::vertex_cmem_test, false, CU_DEFAULT_IN_ITERATIONS);

	cu::test::contig_print();
	cu::test::print_constexpr_max();
	cu::test::print_cache_params<u64_t, base_t>();
	cu::test::print_cache_params<u32_t, base_t>();
	cu::test::print_cache_params<u16_t, base_t>();
	cu::test::print_cache_params<u8_t, base_t>();

	system("pause");

	return 0;
}