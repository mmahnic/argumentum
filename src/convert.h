// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <cerrno>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace argumentum {

std::tuple<int, int, int> parse_int_prefix( std::string_view sv );
std::tuple<int, int> parse_float_prefix( std::string_view sv );

template<typename T>
T parse_int( const std::string& s )
{
   std::string_view sv( s );
   auto [sign, base, skip] = parse_int_prefix( sv );
   if ( skip > 0 )
      sv = sv.substr( skip );

   struct ClearErrno
   {
      ~ClearErrno()
      {
         errno = 0;
      }
   } clear_errno;

   char* pend;
   auto checkResult = [&]( auto res ) {
      if ( errno == ERANGE )
         throw std::out_of_range( s );
      if ( errno == EINVAL || pend == sv.data() )
         throw std::invalid_argument( s );
      if ( res < std::numeric_limits<T>::min() || res > std::numeric_limits<T>::max() )
         throw std::out_of_range( s );
      return static_cast<T>( res );
   };

   if ( std::numeric_limits<T>::is_signed )
      return checkResult( sign * strtoll( sv.data(), &pend, base ) );
   else if ( sign > 0 )
      return checkResult( strtoull( sv.data(), &pend, base ) );
   else
      throw std::out_of_range( s );
}

namespace strtodx {
template<typename T>
T parse( const char* pdata, char** pend )
{
   return strtod( pdata, pend );
}
template<>
inline float parse<float>( const char* pdata, char** pend )
{
   return strtof( pdata, pend );
}
template<>
inline long double parse<long double>( const char* pdata, char** pend )
{
   return strtold( pdata, pend );
}
}   // namespace strtodx

template<typename T>
T parse_float( const std::string& s )
{
   std::string_view sv( s );
   auto [sign, skip] = parse_float_prefix( sv );
   if ( skip > 0 )
      sv = sv.substr( skip );

   struct ClearErrno
   {
      ~ClearErrno()
      {
         errno = 0;
      }
   } clear_errno;

   char* pend;
   auto checkResult = [&]( auto res ) {
      if ( errno == ERANGE )
         throw std::out_of_range( s );
      if ( errno == EINVAL || pend == sv.data() )
         throw std::invalid_argument( s );
      if ( res < -std::numeric_limits<T>::max() || res > std::numeric_limits<T>::max() )
         throw std::out_of_range( s );
      return static_cast<T>( res );
   };

   return checkResult( sign * strtodx::parse<T>( sv.data(), &pend ) );
}

template<typename T>
struct from_string
{
};

template<typename T>
struct from_string<std::optional<T>>
{
   static T convert( const std::string& s )
   {
      return from_string<T>::convert( s );
   }
};

template<>
struct from_string<std::string>
{
   static std::string convert( const std::string& s )
   {
      return s;
   }
};

template<>
struct from_string<bool>
{
   static bool convert( const std::string& s )
   {
      return parse_int<int>( s );
   }
};

template<>
struct from_string<int8_t>
{
   static int8_t convert( const std::string& s )
   {
      return parse_int<int8_t>( s );
   }
};

template<>
struct from_string<uint8_t>
{
   static uint8_t convert( const std::string& s )
   {
      return parse_int<uint8_t>( s );
   }
};

template<>
struct from_string<short>
{
   static short convert( const std::string& s )
   {
      return parse_int<short>( s );
   }
};

template<>
struct from_string<unsigned short>
{
   static unsigned short convert( const std::string& s )
   {
      return parse_int<unsigned short>( s );
   }
};

template<>
struct from_string<int>
{
   static int convert( const std::string& s )
   {
      return parse_int<int>( s );
   }
};

template<>
struct from_string<unsigned int>
{
   static unsigned int convert( const std::string& s )
   {
      return parse_int<unsigned int>( s );
   }
};

template<>
struct from_string<long>
{
   static long convert( const std::string& s )
   {
      return parse_int<long>( s );
   }
};

template<>
struct from_string<unsigned long>
{
   static unsigned long convert( const std::string& s )
   {
      return parse_int<unsigned long>( s );
   }
};

template<>
struct from_string<long long>
{
   static long long convert( const std::string& s )
   {
      return parse_int<long long>( s );
   }
};

template<>
struct from_string<unsigned long long>
{
   static unsigned long long convert( const std::string& s )
   {
      return parse_int<unsigned long long>( s );
   }
};

template<>
struct from_string<float>
{
   static float convert( const std::string& s )
   {
      return parse_float<float>( s );
   }
};

template<>
struct from_string<double>
{
   static double convert( const std::string& s )
   {
      return parse_float<double>( s );
   }
};

template<>
struct from_string<long double>
{
   static long double convert( const std::string& s )
   {
      return parse_float<long double>( s );
   }
};

}   // namespace argumentum
