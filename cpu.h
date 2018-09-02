#pragma once

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <sstream>
#include <iostream>

#define CU_COMP_TIME static constexpr
#define CU_FUNC
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
};

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

template <typename memType, typename... Args>
struct decl_struct
{
	using mem_type = memType;

	memType data;
	decl_struct<Args...> next;
};

template <typename memType>
struct decl_struct<memType>
{
	using mem_type = memType;

	memType data;
};

template <>
struct decl_struct<void>
{
};

template <typename memType, typename ...Args>
CU_FUNC_COMP_TIME decl_struct<Args...> pass_through(decl_struct<memType, Args...>&& p)
{
	return s.next;
}

template <typename memType>
CU_FUNC_COMP_TIME decl_struct<memType> pass_through(decl_struct<memType>&& p)
{
	return s;
}

template <bool cond, typename trueType, typename falseType>
struct type_if
{
	using type = trueType;
};

template <typename trueType, typename falseType>
struct type_if<false, trueType, falseType>
{
	using type = falseType;
};

template <template_int_t count, typename memType, typename ...Args>
struct pop_type
{
	using value_type = typename pop_type<count - 1, Args...>::value_type;
};

template <typename memType, typename ...Args>
struct pop_type<0, memType, Args...>
{
	using value_type = memType;
};

template <template_int_t offset, typename memType, typename ...Args>
struct get_impl;

template <template_int_t offset, typename memType, typename ...Args>
struct get_impl
{
	static typename pop_type<offset, memType, Args...>::value_type call(decl_struct<memType, Args...> s)
	{
		return get_impl<offset - 1, Args...>::call(s.next);
	}
};

template <typename memType, typename ...Args>
struct get_impl<0, memType, Args...>
{
	static typename pop_type<0, memType, Args...>::value_type call(decl_struct<memType, Args...> s)
	{
		return s.data;
	}
};

template <template_int_t offset, typename memType, typename ...Args>
CU_FUNC_COMP_TIME typename pop_type<offset, memType, Args...>::value_type get(decl_struct<memType, Args...> s)
{
	return get_impl<offset, memType, Args...>::call(s);
}

namespace test {

using abc_t = decl_struct<uint8_t, uint32_t, uint16_t>;

CU_FUNC void abc_print()
{
	abc_t t{};

	auto a = get<0>(t);
	auto b = get<1>(t);
	auto c = get<2>(t);

	std::cout << sizeof(a) << "\n"
		      << sizeof(b) << "\n"
		      << sizeof(c) << "\n"
		      << std::endl;
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


