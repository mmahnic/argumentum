// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <regex>
#include <string_view>

namespace argumentum {

ARGUMENTUM_INLINE std::tuple<int, int, int> parse_int_prefix( std::string_view sv )
{
   static auto rxPrefix = std::regex( "^([-+]*)(0[bdox])?" );
   int sign = 1;
   int base = 10;
   std::match_results<std::string_view::iterator> m;
   if ( std::regex_search( sv.begin(), sv.end(), m, rxPrefix ) ) {
      if ( m.length( 1 ) )
         sign = std::count( m[1].first, m[1].second, '-' ) % 2 ? -1 : 1;
      if ( m.length( 2 ) ) {
         switch ( *( m[2].first + 1 ) ) {
            case 'b':
               base = 2;
               break;
            case 'd':
               base = 10;
               break;
            case 'o':
               base = 8;
               break;
            case 'x':
               base = 16;
               break;
         }
      }
      return std::make_tuple( sign, base, m.length( 0 ) );
   }
   return std::make_tuple( sign, base, 0 );
}

ARGUMENTUM_INLINE std::tuple<int, int> parse_float_prefix( std::string_view sv )
{
   static auto rxPrefix = std::regex( "^([-+]*)(0[dx])?" );
   int sign = 1;
   std::match_results<std::string_view::iterator> m;
   if ( std::regex_search( sv.begin(), sv.end(), m, rxPrefix ) ) {
      if ( m.length( 1 ) )
         sign = std::count( m[1].first, m[1].second, '-' ) % 2 ? -1 : 1;
      if ( m.length( 2 ) ) {
         switch ( *( m[2].first + 1 ) ) {
            case 'd':
               return std::make_tuple( sign, m.length( 0 ) );
            case 'x':
               return std::make_tuple( sign, m.length( 1 ) );
         }
      }
      return std::make_tuple( sign, m.length( 1 ) );
   }
   return std::make_tuple( sign, 0 );
}

}   // namespace argumentum
