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

namespace argparse {

template<typename T>
T parse_int( const std::string& s )
{
   std::string_view sv( s );
   int sign = 1;
   if ( sv.substr( 0, 2 ) == "0d" )
      sv = sv.substr( 2 );
   else if ( sv.substr( 0, 3 ) == "-0d" ) {
      sv = sv.substr( 3 );
      sign = -1;
   }

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
      return checkResult( sign * strtoll( sv.data(), &pend, 10 ) );
   else if ( sign > 0 )
      return checkResult( strtoull( sv.data(), &pend, 10 ) );
   else
      throw std::out_of_range( s );
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
      return parse_int<bool>( s );
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
      return stof( s );
   }
};

template<>
struct from_string<double>
{
   static double convert( const std::string& s )
   {
      return stod( s );
   }
};

template<>
struct from_string<long double>
{
   static long double convert( const std::string& s )
   {
      return stold( s );
   }
};

}   // namespace argparse
