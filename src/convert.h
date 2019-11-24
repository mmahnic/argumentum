// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <optional>
#include <string>
#include <vector>

namespace argparse {

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
      return stoi( s ) != 0;
   }
};

template<>
struct from_string<int8_t>
{
   static int8_t convert( const std::string& s )
   {
      return stoi( s );
   }
};

template<>
struct from_string<uint8_t>
{
   static uint8_t convert( const std::string& s )
   {
      return stoi( s );
   }
};

template<>
struct from_string<short>
{
   static short convert( const std::string& s )
   {
      return stoi( s );
   }
};

template<>
struct from_string<unsigned short>
{
   static unsigned short convert( const std::string& s )
   {
      return stoi( s );
   }
};

template<>
struct from_string<int>
{
   static int convert( const std::string& s )
   {
      return stoi( s );
   }
};

template<>
struct from_string<unsigned int>
{
   static unsigned int convert( const std::string& s )
   {
      return stoi( s );
   }
};

template<>
struct from_string<long>
{
   static long convert( const std::string& s )
   {
      return stol( s );
   }
};

template<>
struct from_string<unsigned long>
{
   static unsigned long convert( const std::string& s )
   {
      return stoul( s );
   }
};

template<>
struct from_string<long long>
{
   static long long convert( const std::string& s )
   {
      return stoll( s );
   }
};

template<>
struct from_string<unsigned long long>
{
   static unsigned long long convert( const std::string& s )
   {
      return stoull( s );
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
