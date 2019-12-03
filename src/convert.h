// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <cerrno>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace argparse {

template<typename T, typename TStrtoxx>
T parse_int( std::string_view s, TStrtoxx strtoxx )
{
   if ( s.substr( 0, 2 ) == "0d" )
      s = s.substr( 2 );

   T res = strtoxx( s.data(), nullptr, 10 );
   if ( errno == ERANGE )
      throw std::out_of_range( std::string{ s } );

   return res;
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
      return parse_int<bool>( s, std::strtol );
   }
};

template<>
struct from_string<int8_t>
{
   static int8_t convert( const std::string& s )
   {
      return parse_int<int8_t>( s, std::strtol );
   }
};

template<>
struct from_string<uint8_t>
{
   static uint8_t convert( const std::string& s )
   {
      return parse_int<uint8_t>( s, std::strtoul );
   }
};

template<>
struct from_string<short>
{
   static short convert( const std::string& s )
   {
      return parse_int<short>( s, std::strtol );
   }
};

template<>
struct from_string<unsigned short>
{
   static unsigned short convert( const std::string& s )
   {
      return parse_int<unsigned short>( s, std::strtoul );
   }
};

template<>
struct from_string<int>
{
   static int convert( const std::string& s )
   {
      return parse_int<int>( s, std::strtol );
   }
};

template<>
struct from_string<unsigned int>
{
   static unsigned int convert( const std::string& s )
   {
      return parse_int<unsigned int>( s, std::strtoul );
   }
};

template<>
struct from_string<long>
{
   static long convert( const std::string& s )
   {
      return parse_int<long>( s, std::strtol );
   }
};

template<>
struct from_string<unsigned long>
{
   static unsigned long convert( const std::string& s )
   {
      return parse_int<unsigned long>( s, std::strtoul );
   }
};

template<>
struct from_string<long long>
{
   static long long convert( const std::string& s )
   {
      return parse_int<long long>( s, std::strtoll );
   }
};

template<>
struct from_string<unsigned long long>
{
   static unsigned long long convert( const std::string& s )
   {
      return parse_int<unsigned long long>( s, std::strtoull );
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
