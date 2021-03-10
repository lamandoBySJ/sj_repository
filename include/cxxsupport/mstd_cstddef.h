/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MSTD_CSTDDEF_
#define MSTD_CSTDDEF_

/* <mstd_cstddef>
 *
 * - provides <cstddef>
 * - For ARM C 5, standard C++11/14 features:
 * - - adds macro replacements for alignof and alignas keywords
 * - - adds missing std::nullptr_t
 * - For all toolchains:
 * - - MSTD_CONSTEXPR_XX_14 macros that can be used to mark
 *     things that are valid as constexpr only for C++14 or later,
 *     permitting constexpr use where ARM C 5 would reject it.
  * - - MSTD_CONSTEXPR_XX_11 macros that can be used to permit
  *     constexpr alternatives for C.
 */

#if __cplusplus

#include <cstddef>
//#include <type_traits>
//#include <chrono>
/* Macros that can be used to mark functions and objects that are
 * constexpr in C++14 or later, but not in earlier versions.
 */
#if __cplusplus >= 201402
#define MSTD_CONSTEXPR_FN_14 constexpr
#define MSTD_CONSTEXPR_OBJ_14 constexpr
#else
#define MSTD_CONSTEXPR_FN_14 inline
#define MSTD_CONSTEXPR_OBJ_14 const
#endif

#define MSTD_CONSTEXPR_FN_11 constexpr
#define MSTD_CONSTEXPR_OBJ_11 constexpr

namespace mstd {
using std::size_t;
using std::ptrdiff_t;
using std::nullptr_t;
using std::max_align_t;
}

/*
using namespace std;
namespace __parse_int
{

  template<unsigned _Base, char _Dig>
    struct _Digit;

  template<unsigned _Base>
    struct _Digit<_Base, '0'> : integral_constant<unsigned, 0>
    {
      using __valid = true_type;
    };

  template<unsigned _Base>
    struct _Digit<_Base, '1'> : integral_constant<unsigned, 1>
    {
      using __valid = true_type;
    };

  template<unsigned _Base, unsigned _Val>
    struct _Digit_impl : integral_constant<unsigned, _Val>
    {
      static_assert(_Base > _Val, "invalid digit");
      using __valid = true_type;
    };

  template<unsigned _Base>
    struct _Digit<_Base, '2'> : _Digit_impl<_Base, 2>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, '3'> : _Digit_impl<_Base, 3>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, '4'> : _Digit_impl<_Base, 4>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, '5'> : _Digit_impl<_Base, 5>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, '6'> : _Digit_impl<_Base, 6>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, '7'> : _Digit_impl<_Base, 7>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, '8'> : _Digit_impl<_Base, 8>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, '9'> : _Digit_impl<_Base, 9>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, 'a'> : _Digit_impl<_Base, 0xa>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, 'A'> : _Digit_impl<_Base, 0xa>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, 'b'> : _Digit_impl<_Base, 0xb>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, 'B'> : _Digit_impl<_Base, 0xb>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, 'c'> : _Digit_impl<_Base, 0xc>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, 'C'> : _Digit_impl<_Base, 0xc>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, 'd'> : _Digit_impl<_Base, 0xd>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, 'D'> : _Digit_impl<_Base, 0xd>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, 'e'> : _Digit_impl<_Base, 0xe>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, 'E'> : _Digit_impl<_Base, 0xe>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, 'f'> : _Digit_impl<_Base, 0xf>
    { };

  template<unsigned _Base>
    struct _Digit<_Base, 'F'> : _Digit_impl<_Base, 0xf>
    { };

  //  Digit separator
  template<unsigned _Base>
    struct _Digit<_Base, '\''> : integral_constant<unsigned, 0>
    {
      using __valid = false_type;
    };

//------------------------------------------------------------------------------

  template<unsigned long long _Val>
    using __ull_constant = integral_constant<unsigned long long, _Val>;

  template<unsigned _Base, char _Dig, char... _Digs>
    struct _Power_help
    {
      using __next = typename _Power_help<_Base, _Digs...>::type;
      using __valid_digit = typename _Digit<_Base, _Dig>::__valid;
      using type
	= __ull_constant<__next::value * (__valid_digit{} ? _Base : 1ULL)>;
    };

  template<unsigned _Base, char _Dig>
    struct _Power_help<_Base, _Dig>
    {
      using __valid_digit = typename _Digit<_Base, _Dig>::__valid;
      using type = __ull_constant<__valid_digit::value>;
    };

  template<unsigned _Base, char... _Digs>
    struct _Power : _Power_help<_Base, _Digs...>::type
    { };

  template<unsigned _Base>
    struct _Power<_Base> : __ull_constant<0>
    { };

//------------------------------------------------------------------------------

  template<unsigned _Base, unsigned long long _Pow, char _Dig, char... _Digs>
    struct _Number_help
    {
      using __digit = _Digit<_Base, _Dig>;
      using __valid_digit = typename __digit::__valid;
      using __next = _Number_help<_Base,
				  __valid_digit::value ? _Pow / _Base : _Pow,
				  _Digs...>;
      using type = __ull_constant<_Pow * __digit::value + __next::type::value>;
      static_assert((type::value / _Pow) == __digit::value,
		    "integer literal does not fit in unsigned long long");
    };

  template<unsigned _Base, unsigned long long _Pow, char _Dig>
    struct _Number_help<_Base, _Pow, _Dig>
    {
      //static_assert(_Pow == 1U, "power should be one");
      using type = __ull_constant<_Digit<_Base, _Dig>::value>;
    };

  template<unsigned _Base, char... _Digs>
    struct _Number
    : _Number_help<_Base, _Power<_Base, _Digs...>::value, _Digs...>::type
    { };

  template<unsigned _Base>
    struct _Number<_Base>
    : __ull_constant<0>
    { };

//------------------------------------------------------------------------------

  template<char... _Digs>
    struct _Parse_int;

  template<char... _Digs>
    struct _Parse_int<'0', 'b', _Digs...>
    : _Number<2U, _Digs...>::type
    { };

  template<char... _Digs>
    struct _Parse_int<'0', 'B', _Digs...>
    : _Number<2U, _Digs...>::type
    { };

  template<char... _Digs>
    struct _Parse_int<'0', 'x', _Digs...>
    : _Number<16U, _Digs...>::type
    { };

  template<char... _Digs>
    struct _Parse_int<'0', 'X', _Digs...>
    : _Number<16U, _Digs...>::type
    { };

  template<char... _Digs>
    struct _Parse_int<'0', _Digs...>
    : _Number<8U, _Digs...>::type
    { };

  template<char... _Digs>
    struct _Parse_int
    : _Number<10U, _Digs...>::type
    { };

} // namespace __parse_int


namespace __select_int
{
  template<unsigned long long _Val, typename... _Ints>
    struct _Select_int_base;

  template<unsigned long long _Val, typename _IntType, typename... _Ints>
    struct _Select_int_base<_Val, _IntType, _Ints...>
    : conditional_t<(_Val <= std::numeric_limits<_IntType>::max()),
		    integral_constant<_IntType, _Val>,
		    _Select_int_base<_Val, _Ints...>>
    { };

  template<unsigned long long _Val>
    struct _Select_int_base<_Val>
    { };

  template<char... _Digs>
    using _Select_int = typename _Select_int_base<
	__parse_int::_Parse_int<_Digs...>::value,
	unsigned char,
	unsigned short,
	unsigned int,
	unsigned long,
	unsigned long long
      >::type;
}


 inline namespace literals
  {
  inline namespace chrono_literals
  {

    template<typename _Rep, unsigned long long _Val>
      struct _Checked_integral_constant
      : integral_constant<_Rep, static_cast<_Rep>(_Val)>
      {
	static_assert(_Checked_integral_constant::value >= 0
		      && _Checked_integral_constant::value == _Val,
		      "literal value cannot be represented by duration type");
      };

    template<typename _Dur, char... _Digits>
      constexpr _Dur __check_overflow()
      {
	using _Val = __parse_int::_Parse_int<_Digits...>;
	using _Rep = typename _Dur::rep;
	// TODO: should be simply integral_constant<_Rep, _Val::value>
	// but GCC doesn't reject narrowing conversions to _Rep.
	using _CheckedVal = _Checked_integral_constant<_Rep, _Val::value>;
	return _Dur{_CheckedVal::value};
      }

    constexpr chrono::duration<long double, ratio<3600,1>>
    operator""_h(long double __hours)
    { return chrono::duration<long double, ratio<3600,1>>{__hours}; }

    template <char... _Digits>
      constexpr chrono::hours
      operator""h()
      { return __check_overflow<chrono::hours, _Digits...>(); }

    constexpr chrono::duration<long double, ratio<60,1>>
    operator""min(long double __mins)
    { return chrono::duration<long double, ratio<60,1>>{__mins}; }

    template <char... _Digits>
      constexpr chrono::minutes
      operator""min()
      { return __check_overflow<chrono::minutes, _Digits...>(); }

    constexpr chrono::duration<long double>
    operator""s(long double __secs)
    { return chrono::duration<long double>{__secs}; }

    template <char... _Digits>
      constexpr chrono::seconds
      operator""s()
      { return __check_overflow<chrono::seconds, _Digits...>(); }

    constexpr chrono::duration<long double, milli>
    operator""ms(long double __msecs)
    { return chrono::duration<long double, milli>{__msecs}; }

    template <char... _Digits>
      constexpr chrono::milliseconds
      operator""ms()
      { return __check_overflow<chrono::milliseconds, _Digits...>(); }

    constexpr chrono::duration<long double, micro>
    operator""us(long double __usecs)
    { return chrono::duration<long double, micro>{__usecs}; }

    template <char... _Digits>
      constexpr chrono::microseconds
      operator""us()
      { return __check_overflow<chrono::microseconds, _Digits...>(); }

    constexpr chrono::duration<long double, nano>
    operator""ns(long double __nsecs)
    { return chrono::duration<long double, nano>{__nsecs}; }

    template <char... _Digits>
      constexpr chrono::nanoseconds
      operator""ns()
      { return __check_overflow<chrono::nanoseconds, _Digits...>(); }

  } // inline namespace chrono_literals
  } // inline namespace literals
}
*/
#else // __cplusplus

#define MSTD_CONSTEXPR_FN_14 inline
#define MSTD_CONSTEXPR_OBJ_14 const
#define MSTD_CONSTEXPR_FN_11 inline
#define MSTD_CONSTEXPR_OBJ_11 const

#endif // __cplusplus

#endif // MSTD_CSTDDEF_