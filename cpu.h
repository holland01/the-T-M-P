#pragma once

#include <DirectXMath.h>
#include <Windows.h>

#undef max
#undef min

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <sstream>
#include <iostream>
#include <array>

#define CU_COMP_TIME static constexpr
#define CU_FUNC static
#define CU_FUNC_COMP_TIME static inline constexpr
#define CU_STREAM_VALUE(v) #v ": " << (unsigned int)v << ",\n"
#define CU_STATIC_IF if constexpr

#define CU_CACHE_ALIGNED __declspec(align(16))

namespace cu {

using default_word_t = std::uint64_t;

namespace detail {

CU_COMP_TIME default_word_t log2i(default_word_t n)
{
	return (n < 2) ? 1 : 1 + log2i(n / 2);
}


template <typename T, size_t N>
CU_FUNC_COMP_TIME T align(T x)
{
	constexpr auto U = N - 1;
	return ((x + U) & (~U));
}

template <typename T, size_t N>
using static_mem_t = std::array<T, N>;

} // end namespace detail

//----------------------------------------------
// CPU Cache params
//----------------------------------------------

// We (hopefully) avoid UB by calculating everything up front with the highest word size
// and then converting after the fact.
template <default_word_t tnum_lines_per_set, default_word_t tnum_bytes_per_block, default_word_t tnum_cache_bytes, default_word_t tnum_physical_address_bits>
struct cache_base
{
	CU_COMP_TIME default_word_t num_lines_per_set = tnum_lines_per_set;
	CU_COMP_TIME default_word_t num_bytes_per_block = tnum_bytes_per_block;
	CU_COMP_TIME default_word_t num_cache_bytes = tnum_cache_bytes;
	CU_COMP_TIME default_word_t num_physical_address_bits = tnum_physical_address_bits;

	CU_COMP_TIME default_word_t num_sets = num_cache_bytes / (num_lines_per_set * num_bytes_per_block);
	CU_COMP_TIME default_word_t num_set_index_bits = detail::log2i(num_sets);
	CU_COMP_TIME default_word_t num_block_offset_bits = detail::log2i(num_bytes_per_block);
	CU_COMP_TIME default_word_t num_tag_bits = num_physical_address_bits - (num_set_index_bits + num_block_offset_bits);

	CU_COMP_TIME default_word_t block_offset_mask = (1ull << num_block_offset_bits) - 1ull;
	CU_COMP_TIME default_word_t set_index_mask = ((1ull << (num_block_offset_bits + num_set_index_bits)) - 1ull) & (~block_offset_mask);
	CU_COMP_TIME default_word_t tag_mask = ((1ull << (num_block_offset_bits + num_set_index_bits + num_tag_bits)) - 1ull) & (~(block_offset_mask | set_index_mask));

	CU_COMP_TIME default_word_t max_block_offset = block_offset_mask;
	CU_COMP_TIME default_word_t max_set_index = set_index_mask >> num_block_offset_bits;
	CU_COMP_TIME default_word_t max_tag = tag_mask >> (num_block_offset_bits + num_set_index_bits);
};

// Literally just forwarding the parameters through a cast

#define CU_CAST_CACHE_BASE(w_t, x) static_cast<w_t>(cache_base_t::x)

#define CU_CACHE_PARAMS_DECL(word_t, decl_prefix) \
	decl_prefix word_t num_lines_per_set = CU_CAST_CACHE_BASE(word_t, num_lines_per_set);	\
	decl_prefix word_t num_bytes_per_block = CU_CAST_CACHE_BASE(word_t, num_bytes_per_block);	\
	decl_prefix word_t num_cache_bytes = CU_CAST_CACHE_BASE(word_t, num_cache_bytes); \
	decl_prefix word_t num_physical_address_bits = CU_CAST_CACHE_BASE(word_t, num_physical_address_bits); \
																									\
	decl_prefix word_t num_sets = CU_CAST_CACHE_BASE(word_t, num_sets);									\
	decl_prefix word_t num_set_index_bits = CU_CAST_CACHE_BASE(word_t, num_set_index_bits);				\
	decl_prefix word_t num_block_offset_bits = CU_CAST_CACHE_BASE(word_t, num_block_offset_bits);			\
	decl_prefix word_t num_tag_bits = CU_CAST_CACHE_BASE(word_t, num_tag_bits);							\
																									\
	decl_prefix word_t block_offset_mask = CU_CAST_CACHE_BASE(word_t, block_offset_mask);					\
	decl_prefix word_t set_index_mask = CU_CAST_CACHE_BASE(word_t, set_index_mask);						\
	decl_prefix word_t tag_mask = CU_CAST_CACHE_BASE(word_t, tag_mask);									\
																									\
	decl_prefix word_t max_block_offset = CU_CAST_CACHE_BASE(word_t, max_block_offset);					\
	decl_prefix word_t max_set_index = CU_CAST_CACHE_BASE(word_t, max_set_index);							\
	decl_prefix word_t max_tag = CU_CAST_CACHE_BASE(word_t, max_tag);										

template <typename wordType, typename baseType>
struct cache_cast {
	using word_t = wordType;
	using cache_base_t = baseType;

	CU_CACHE_PARAMS_DECL(word_t, CU_COMP_TIME)
};

using arch_x86_64_cache_base = cache_base<8, 64, 1 << 15, 48>;

using arch_x86_64_cache_reg_wordq = cache_cast<std::uint64_t, arch_x86_64_cache_base>;
using arch_x86_64_cache_reg_wordd = cache_cast<std::uint32_t, arch_x86_64_cache_base>;
using arch_x86_64_cache_reg_word = cache_cast<std::uint16_t, arch_x86_64_cache_base>;
using arch_x86_64_cache_reg_byte = cache_cast<std::uint8_t, arch_x86_64_cache_base>;

using x64rwq = arch_x86_64_cache_reg_wordq;
using x64rwd = arch_x86_64_cache_reg_wordd;
using x64rw = arch_x86_64_cache_reg_word;
using x64rb = arch_x86_64_cache_reg_byte;

using arch_x86_cache_base = cache_base<8, 64, 1 << 15, 32>;

using arch_x86_cache_reg_wordd = cache_cast<std::uint32_t, arch_x86_cache_base>;
using arch_x86_cache_reg_word = cache_cast<std::uint16_t, arch_x86_cache_base>;
using arch_x86_cache_reg_byte = cache_cast<std::uint8_t, arch_x86_cache_base>;

using x32rwd = arch_x86_cache_reg_wordd;
using x32rw = arch_x86_cache_reg_word;
using x32rb = arch_x86_64_cache_reg_byte;

using template_int_t = int64_t;

#ifdef _WIN64
using cache_params_t = x64rwq;
#else
using cache_params_t = x32rwd;
#endif

template <typename T>
using cache_blocked_t = detail::static_mem_t<T, cache_params_t::num_bytes_per_block / sizeof(T)>;

//----------------------------------------------
// base meta routines
//----------------------------------------------

namespace detail {

struct false_type {
	static constexpr bool value = false;
};

struct true_type {
	static constexpr bool value = true;
};

template <bool cond, typename trueType, typename falseType>
struct type_if {
	using type = trueType;
};

template <typename trueType, typename falseType>
struct type_if<false, trueType, falseType> {
	using type = falseType;
};

template <template_int_t count, typename memType, typename ...Args>
struct pop_type {
	using value_type = typename pop_type<count - 1, Args...>::value_type;
};

template <typename memType, typename ...Args>
struct pop_type<0, memType, Args...> {
	using value_type = memType;
};

template <template_int_t ta, template_int_t tb>
struct larger {
	CU_COMP_TIME template_int_t value = ta > tb ? ta : tb; 
};

template <template_int_t tvalue, template_int_t... targs>
struct greatest {
	CU_COMP_TIME template_int_t cmp_value = tvalue;

	using next_level = typename greatest<targs...>;

	CU_COMP_TIME template_int_t value = larger<cmp_value, next_level::value>::value;
};

template <template_int_t tvalue>
struct greatest<tvalue> {
	CU_COMP_TIME template_int_t cmp_value = tvalue;
	CU_COMP_TIME template_int_t value = cmp_value;
};

template <template_int_t offset, template <typename ...Args> typename nextWrapType, typename memType, typename ...Args>
struct get_member_impl {
	using this_type = memType;
	using block_type = cache_blocked_t<this_type>;

	template <typename... Args>
	using aggregate_type = nextWrapType<Args...>;
	
	using return_type = typename pop_type<offset, memType, Args...>::value_type;
	using next_get_member_impl = get_member_impl<offset - 1, aggregate_type, Args...>;

	using next_block_type = typename aggregate_type<Args...>::block_type;
	using next_type = typename aggregate_type<Args...>::this_type;

	template <typename nextType>
	CU_FUNC nextType * skip_memory(this_type *base, std::size_t offset)
	{
		return reinterpret_cast<nextType *>(base + offset);
	}

	CU_FUNC_COMP_TIME return_type & call(this_type &s)
	{
		auto p_next = skip_memory<next_type>(&s, 1);
		return next_get_member_impl::call(*p_next); 
	}

	template <template_int_t index>
	CU_FUNC_COMP_TIME return_type & call_array_static(block_type &s)
	{		
		auto p_next = skip_memory<next_block_type>(s.data(), s.size());	
		return next_get_member_impl::call_array_static<index>(*p_next); 
	}

	CU_FUNC return_type & call_array(block_type &s, default_word_t index)
	{
		auto p_next = skip_memory<next_block_type>(s.data(), s.size());
		return next_get_member_impl::call_array(*p_next, index); 
	}
};

template <template <typename ...Args> class nextWrapType, typename memType, typename ...Args>
struct get_member_impl<0, nextWrapType, memType, Args...> {
	using this_type = memType;
	using block_type = cache_blocked_t<this_type>;
	using return_type =  typename pop_type<0, this_type, Args...>::value_type;

	CU_FUNC_COMP_TIME return_type & call(this_type &s)
	{
		return s;
	}

	template <template_int_t index>
	CU_FUNC_COMP_TIME return_type & call_array_static(block_type &s)
	{
		return s.at(index);
	}

	CU_FUNC return_type & call_array(block_type &s, default_word_t index)
	{
		return s.at(index);
	}
};

} // end namespace detail

//----------------------------------------------
// contiguous memory
//----------------------------------------------

template <typename memType, typename ...Args>
struct contig_mem {
	using this_type = memType;
	using next_head_type = contig_mem<Args...>;

	this_type mem;
	typename next_head_type::this_type next;
};

template <typename memType>
struct contig_mem<memType> {
	using this_type = memType;

	this_type mem;
};

template <>
struct contig_mem<void> {};

template <template_int_t offset, typename memType, typename ...Args>
using member_return_type = typename detail::pop_type<offset, memType, Args...>::value_type;

template <template_int_t offset, typename memType, typename ...Args>
CU_FUNC_COMP_TIME member_return_type<offset, memType, Args...> & member(contig_mem<memType, Args...> &s)
{
	return detail::get_member_impl<offset, contig_mem, memType, Args...>::call(s.mem);
}

//----------------------------------------------
// cache friendly data
//----------------------------------------------

template <typename memType, typename ...Args>
struct CU_CACHE_ALIGNED cache_mem {
	using this_type = memType;
	using block_type = cache_blocked_t<this_type>;
	using next_head_type = typename cache_mem<Args...>;

	static constexpr default_word_t max_type_size = detail::larger<sizeof(this_type), next_head_type::max_type_size>::value;
	static constexpr default_word_t array_length = cache_params_t::num_bytes_per_block / max_type_size;
	static constexpr default_word_t bytes_left = sizeof(block_type) + next_head_type::bytes_left; 

	block_type mem;
	typename next_head_type::this_type next;
};

template <typename memType>
struct CU_CACHE_ALIGNED cache_mem<memType> {
	using this_type = memType;
	using block_type = cache_blocked_t<this_type>;

	static constexpr default_word_t max_type_size = sizeof(this_type);
	static constexpr default_word_t bytes_left = sizeof(block_type);

	block_type mem;
};

template <>
struct cache_mem<void> {};

template <template_int_t offset, typename memType, typename ...Args>
member_return_type<offset, memType, Args...> & member(cache_mem<memType, Args...> &s, default_word_t index = 0)
{
	return detail::get_member_impl<offset, cache_mem, memType, Args...>::call_array(s.mem, index);
}

template <template_int_t offset, template_int_t index, typename memType, typename ...Args>
CU_FUNC_COMP_TIME member_return_type<offset, memType, Args...> & member(cache_mem<memType, Args...> &s)
{
	return detail::get_member_impl<offset, cache_mem, memType, Args...>::call_array_static<index>(s.mem);
}

//----------------------------------------------
// con
//----------------------------------------------

#define CU_CACHE_FRIENDLY

template <typename memType, typename... Args>
#ifdef CU_CACHE_FRIENDLY
using aggregate_t = cache_mem<memType, Args...>;
#else
using aggregate_t = contig_mem<memType, Args...>;
#endif

//----------------------------------------------
// tests
//----------------------------------------------


namespace test {

using contig1_t = aggregate_t<uint32_t, uint16_t, uint8_t, uint64_t>;

using u32lim_t = std::numeric_limits<uint32_t>;
using u16lim_t = std::numeric_limits<uint16_t>;
using u8lim_t = std::numeric_limits<uint8_t>;

template <typename T>
CU_FUNC T mmax()
{
	return std::numeric_limits<T>::max();
}

#define CU_MAX(x) std::numeric_limits<x>::max() 

#define CU_SIZEOF_STRING(type) "sizeof(" #type "): " << sizeof(type)

using vertex_cmem_t = cache_mem<
	DirectX::XMVECTOR, 
	DirectX::XMVECTOR, 
	
	float, 
	float, 

	uint8_t,
	uint8_t,
	uint8_t,
	uint8_t
>;

enum  {
	vertex_cmem_position = 0,
	vertex_cmem_normal,
	
	vertex_cmem_tex_u,
	vertex_cmem_tex_v,

	vertex_cmem_color_r,
	vertex_cmem_color_g,
	vertex_cmem_color_b,
	vertex_cmem_color_a
};

#define vertex_cmem_get(inst, member_suffix, index) member<vertex_cmem_##member_suffix>(inst, index)

struct vertex
{
	DirectX::XMVECTOR position;
	DirectX::XMVECTOR normal;
	
	float tex_u;
	float tex_v;

	uint8_t color_r;
	uint8_t	color_g;
	uint8_t	color_b;
	uint8_t	color_a;
};

using vertex_array_t = std::array<vertex, vertex_cmem_t::array_length>;

template <template_int_t tnum_iterations, typename runFunc, typename ...Args>
struct benchmark {
	using run_func_type = runFunc;

	using bench_time_t = double;

	LARGE_INTEGER win_perf_counter{};
	LARGE_INTEGER win_perf_counter_end{};

	bench_time_t time_value_avg{};
	bench_time_t time_value_start{};
	bench_time_t inverse_iterations{};
	bench_time_t win_time_perf_frequency{};

	DWORD	win_error_code = ERROR_SUCCESS;

	bool run(run_func_type func, Args ...args)
	{
		LARGE_INTEGER freq;
		if (!QueryPerformanceFrequency(&freq)) {
			win_error_code = GetLastError();
			return false;
		}

		win_time_perf_frequency = 1.0 / static_cast<bench_time_t>(freq.QuadPart);
		inverse_iterations = 1.0 / static_cast<bench_time_t>(tnum_iterations);

		for (template_int_t i = 0; i < tnum_iterations; ++i)
		{
			if (!QueryPerformanceCounter(&win_perf_counter)) {
				win_error_code = GetLastError();
				return false;
			}

			func(args...);

			if (!QueryPerformanceCounter(&win_perf_counter_end)) {
				win_error_code = GetLastError();
				return false;
			}

			bench_time_t diff = static_cast<bench_time_t>(win_perf_counter_end.QuadPart - win_perf_counter.QuadPart);
			time_value_avg += diff * win_time_perf_frequency;
		}

		time_value_avg *= inverse_iterations;

		std::cout	<< "-----------------------------------------------\n"
					<< "Time (seconds): " << time_value_avg << "\n"
					<< "Num Iterations: " << tnum_iterations << "\n"
					<< "Performance Frequency: " << freq.QuadPart << "\n"
					<< "-----------------------------------------------\n"
					<< std::endl;

		return true;
	}
};

CU_FUNC void vertex_cmem_test(bool print_vals, std::size_t iterations)
{
	vertex_cmem_t vmem;

	constexpr float sz = 1.0f / static_cast<float>(vertex_cmem_t::array_length);

	for (std::size_t i = 0; i < vmem.array_length; ++i) {
		auto & [
				position,
				color_r,
				color_g,
				color_b,
				color_a
			] = std::tie(
				vertex_cmem_get(vmem, position, i),
				vertex_cmem_get(vmem, color_r, i),
				vertex_cmem_get(vmem, color_g, i),
				vertex_cmem_get(vmem, color_b, i),
				vertex_cmem_get(vmem, color_a, i)
			);

		for (std::size_t x = 0; x < iterations; ++x) {
			position = DirectX::XMVectorSet(0.0f, static_cast<float>(i), 0.0f, 1.0f);
			color_r = static_cast<uint8_t>(255.0f * sz * static_cast<float>(i));
			color_g = 0;
			color_b = 0;
			color_a = 255;
		}
	}

	if (print_vals)
	{
		for (std::size_t i = 0; i < vmem.array_length; ++i) {
			const auto & [
				position,
				color_r,
				color_g,
				color_b,
				color_a
			] = std::tie(
				vertex_cmem_get(vmem, position, i),
				vertex_cmem_get(vmem, color_r, i),
				vertex_cmem_get(vmem, color_g, i),
				vertex_cmem_get(vmem, color_b, i),
				vertex_cmem_get(vmem, color_a, i)
			);

			std::cout << i << "\n---\n\n"
					  << CU_STREAM_VALUE(DirectX::XMVectorGetY(position))
					  << CU_STREAM_VALUE(color_r)
					  << CU_STREAM_VALUE(color_g)
					  << CU_STREAM_VALUE(color_b)
					  << CU_STREAM_VALUE(color_a)
					  << "------\n"
					  << std::endl;
		}
	}
}

#define CU_DEFAULT_IN_ITERATIONS 10000

#define CU_BENCHMARK_TYPE(func) benchmark<1000, decltype(&func), bool, std::size_t>

using vertex_cmem_test_benchmark_t = CU_BENCHMARK_TYPE(vertex_cmem_test);

CU_FUNC void vertex_array_test(bool print_vals, std::size_t iterations)
{
	vertex_array_t varray;

	constexpr float sz = 1.0f / static_cast<float>(vertex_cmem_t::array_length);

	 for (std::size_t i = 0; i < varray.size(); ++i) {
		for (std::size_t x = 0; x < iterations; ++x) {
			varray[i].position = DirectX::XMVectorSet(0.0f, static_cast<float>(i), 0.0f, 1.0f);
			varray[i].color_r = static_cast<uint8_t>(255.0f * sz * static_cast<float>(i));
			varray[i].color_g = 0;
			varray[i].color_b = 0;
			varray[i].color_a = 255;
		}
	}

	if (print_vals)
	{
		for (std::size_t i = 0; i < varray.size(); ++i) {
			std::cout << i << "\n---\n\n"
					  << CU_STREAM_VALUE(DirectX::XMVectorGetY(varray[i].position))
					  << CU_STREAM_VALUE(varray[i].color_r)
					  << CU_STREAM_VALUE(varray[i].color_g)
					  << CU_STREAM_VALUE(varray[i].color_b)
					  << CU_STREAM_VALUE(varray[i].color_a)
					  << "------\n"
					  << std::endl;
		}
	}
}

using vertex_array_test_benchmark_t = CU_BENCHMARK_TYPE(vertex_array_test);

CU_FUNC void contig_print()
{
	contig1_t lol;

	std::cout 
		<< CU_STREAM_VALUE(contig1_t::max_type_size)
		<<	CU_SIZEOF_STRING(contig1_t) << ",\n" 
		<<	CU_STREAM_VALUE(contig1_t::array_length)  
		<< CU_STREAM_VALUE(lol.bytes_left)
		<< std::endl;

	{
		auto &a = member<0, 1>(lol);
		auto &b = member<1, 1>(lol);
		auto &c = member<2, 1>(lol);
		auto &d = member<3, 1>(lol);

		a = CU_MAX(uint32_t);
		b = CU_MAX(uint16_t);
		c = CU_MAX(uint8_t);
		d = CU_MAX(uint64_t);
	}

	{
		auto &a = member<0>(lol);
		auto &b = member<1>(lol);
		auto &c = member<2>(lol);
		auto &d = member<3>(lol);

		std::cout << "index 0\n\n"
				  << sizeof(a) << "\n"
				  << sizeof(b) << "\n"
				  << sizeof(c) << "\n"
				  << std::endl;

		std::cout << std::hex << a << "\n" << b << "\n" << (uint32_t)c << "\n" << d << "\n" << std::endl;
		 
	}

	{
		auto &a = member<0>(lol, 1);
		auto &b = member<1>(lol, 1);
		auto &c = member<2>(lol, 1);
		auto &d = member<3>(lol, 1);

		std::cout << "index 1\n\n"
				  << sizeof(a) << "\n"
				  << sizeof(b) << "\n"
				  << sizeof(c) << "\n"
				  << std::endl;

		std::cout << a << "\n" << b << "\n" << (uint32_t)c << "\n" << d << "\n" << std::endl;
	}
}

CU_FUNC void print_constexpr_max()
{
	auto m = detail::greatest<1, 2, 3, 4, 5, 6, 7>::value;

	std::cout << "m: " << m << std::endl;
}

template <typename wordType, typename baseType>
CU_FUNC void print_cache_params()
{
	using cache_base_t = baseType;
	using vword_t = wordType;

	CU_CACHE_PARAMS_DECL(vword_t,const);

	std::stringstream ss;
	ss << "word size: " << sizeof(vword_t) << "\n\n\n";
	ss << CU_STREAM_VALUE(num_lines_per_set)
	   << CU_STREAM_VALUE(num_bytes_per_block)
	   << CU_STREAM_VALUE(num_cache_bytes)
	   << CU_STREAM_VALUE(num_physical_address_bits)
	   << CU_STREAM_VALUE(num_sets)
	   << CU_STREAM_VALUE(num_set_index_bits)
	   << CU_STREAM_VALUE(num_block_offset_bits)
	   << CU_STREAM_VALUE(num_tag_bits)
	   << CU_STREAM_VALUE(block_offset_mask)
	   << CU_STREAM_VALUE(set_index_mask)
	   << CU_STREAM_VALUE(tag_mask)
	   << CU_STREAM_VALUE(max_block_offset)
	   << CU_STREAM_VALUE(max_set_index)
	   << CU_STREAM_VALUE(max_tag);

	std::cout << "--------\n" << ss.str() << "\n----------\n" << std::endl;
	   
}
	
} // end namespace test

}


