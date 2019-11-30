// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "testutil.h"

namespace testutil {

std::vector<std::string_view> splitLines( std::string_view text, bool keepEmpty )
{
   std::vector<std::string_view> output;
   size_t start = 0;
   auto delims = "\n\r";

   auto isWinEol = [&text]( auto pos ) { return text[pos] == '\r' && text[pos + 1] == '\n'; };

   while ( start < text.size() ) {
      const auto stop = text.find_first_of( delims, start );

      if ( keepEmpty || start != stop )
         output.emplace_back( text.substr( start, stop - start ) );

      if ( stop == std::string_view::npos )
         break;

      start = stop + ( isWinEol( stop ) ? 2 : 1 );
   }

   return output;
}

bool strHasText( std::string_view line, std::string_view text )
{
   return line.find( text ) != std::string::npos;
}

bool strHasTexts( std::string_view line, std::vector<std::string_view> texts )
{
   if ( texts.empty() )
      return true;
   auto it = std::begin( texts );
   size_t pos = line.find( *it );
   while ( it != std::end( texts ) && pos != std::string::npos ) {
      if ( ++it != std::end( texts ) )
         pos = line.find( *it, pos + 1 );
   }
   return pos != std::string::npos;
}

}   // namespace testutil
