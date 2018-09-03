#pragma once

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <sstream>
#include <iostream>
#include <array>

#define CU_COMP_TIME static constexpr
#define CU_FUNC static
#define CU_FUNC_COMP_TIME static constexpr
#define CU_STREAM_VALUE(v) #v ": " << (unsigned int)v << ",\n"
#define CU_STATIC_IF if constexpr

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

using template_int_t = int;

#ifdef _WIN64
using cache_params_t = x64rwq;
#else
using cache_params_t = x32rwd;
#endif

template <typename T>
using cache_blocked_t = detail::static_mem_t<T, cache_params_t::num_bytes_per_block / sizeof(T)>;

//----------------------------------------------
// base routines
//----------------------------------------------

namespace detail {

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

template <template_int_t offset, template <typename... Args> typename nextWrapType, typename memType, typename ...Args>
struct get_member_impl {
	using this_type = memType;
	using block_type = cache_blocked_t<this_type>;

	template <typename... Args>
	using aggregate_type = nextWrapType<Args...>;
	
	CU_FUNC_COMP_TIME typename pop_type<offset, memType, Args...>::value_type & call(this_type &s)
	{
		using next_type = typename aggregate_type<Args...>::this_type;
		using this_type = memType;

		auto p_next = reinterpret_cast<next_type *>(static_cast<this_type *>(&s) + 1);

		return get_member_impl<offset - 1, aggregate_type, Args...>::call(*p_next); 
	}

	CU_FUNC typename pop_type<offset, memType, Args...>::value_type & call_array(block_type &s, default_word_t index)
	{
		using next_block_type = typename aggregate_type<Args...>::block_type;

		auto p_next = reinterpret_cast<next_block_type *>(static_cast<this_type *>(s.data()) + s.size());

		return get_member_impl<offset - 1, aggregate_type, Args...>::call_array(*p_next, index); 
	}
};

template <template <typename ...Args> class nextWrapType, typename memType, typename ...Args>
struct get_member_impl<0, nextWrapType, memType, Args...> {
	using this_type = memType;
	using block_type = cache_blocked_t<this_type>;

	CU_FUNC_COMP_TIME typename pop_type<0, this_type, Args...>::value_type & call(this_type &s)
	{
		return s;
	}

	CU_FUNC typename pop_type<0, this_type, Args...>::value_type & call_array(block_type &s, default_word_t index)
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
typename detail::pop_type<offset, memType, Args...>::value_type & member(contig_mem<memType, Args...> &s)
{
	return detail::get_member_impl<offset, contig_mem, memType, Args...>::call(s.mem);
}

//----------------------------------------------
// cache friendly data
//----------------------------------------------

template <typename memType, typename ...Args>
struct cache_friendly {
	using this_type = memType;
	using block_type = cache_blocked_t<this_type>;
	using next_head_type = contig_mem<Args...>;
	
	block_type mem;
	typename next_head_type::this_type next;
};

template <typename memType>
struct cache_friendly<memType> {
	using this_type = memType;
	using block_type = cache_blocked_t<this_type>;

	block_type mem;
};

template <>
struct cache_friendly<void> {};

template <template_int_t offset, typename memType, typename ...Args>
typename detail::pop_type<offset, memType, Args...>::value_type & member(cache_friendly<memType, Args...> &s, default_word_t index = 0)
{
	return detail::get_member_impl<offset, cache_friendly, memType, Args...>::call_array(s.mem, index);
}

//----------------------------------------------
// con
//----------------------------------------------

#define CU_CACHE_FRIENDLY

template <typename memType, typename... Args>
#ifdef CU_CACHE_FRIENDLY
using aggregate_t = cache_friendly<memType, Args...>;
#else
using aggregate_t = contig_mem<memType, Args...>;
#endif

//----------------------------------------------
// tests
//----------------------------------------------


namespace test {

using contig1_t = aggregate_t<uint32_t, uint16_t, uint8_t>;

using u32lim_t = std::numeric_limits<uint32_t>;
using u16lim_t = std::numeric_limits<uint16_t>;
using u8lim_t = std::numeric_limits<uint8_t>;

template <typename T>
CU_FUNC T mmax()
{
	return std::numeric_limits<T>::max();
}

#define CU_MAX(x) std::numeric_limits<decltype(x)>::max() 

#define CU_SIZEOF_STRING(type) "sizeof(" #type "): " << sizeof(type)

CU_FUNC void contig_print()
{
	contig1_t lol;

	{
		auto &a = member<0>(lol, 1);
		auto &b = member<1>(lol, 1);
		auto &c = member<2>(lol, 1);

		a = 3;
		b = 2;
		c = 1;
	}

	{
		auto &a = member<0>(lol);
		auto &b = member<1>(lol);
		auto &c = member<2>(lol);

		std::cout << "index 0\n\n"
				  << sizeof(a) << "\n"
				  << sizeof(b) << "\n"
				  << sizeof(c) << "\n"
				  << std::endl;

		std::cout << std::hex << a << "\n" << b << "\n" << (uint32_t)c << "\n" << std::endl;
	}

	{
		auto &a = member<0>(lol, 1);
		auto &b = member<1>(lol, 1);
		auto &c = member<2>(lol, 1);

		std::cout << "index 1\n\n"
				  << sizeof(a) << "\n"
				  << sizeof(b) << "\n"
				  << sizeof(c) << "\n"
				  << std::endl;

		std::cout << a << "\n" << b << "\n" << (uint32_t)c << "\n" << std::endl;
	}
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


