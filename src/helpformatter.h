// Copyright (c) 2018, 2019 Marko Mahnič
// License: MIT. See LICENSE in the root of the project.

#pragma once

#include "helpformatter_i.h"

#include "argparser.h"

namespace argparse {

class Writer
{
   std::ostream& stream;
   size_t position = 0;
   size_t width = 80;

public:
   Writer( std::ostream& outStream, size_t widthBytes=80 )
      : stream( outStream ), width( widthBytes )
   {}

   void write( std::string_view text );

   std::vector<std::string_view> splitIntoWords( std::string_view text )
   {
      std::vector<std::string_view> words;

      size_t pos = 0;
      while ( pos < text.size() ) {
         while ( pos < text.size() && iswspace( text[pos] ) )
            ++pos;

         size_t end = pos;
         while ( end < text.size() && !iswspace( text[end] ) )
            ++end;

         if ( end > pos )
            words.push_back( text.substr( pos, end - pos ) );

         pos = end;
      }

      return words;
   }
};

inline void HelpFormatter::format( const ArgumentParser& parser, std::ostream& out )
{
   auto config = parser.getConfig();
   auto args = parser.describe_arguments();
   auto iopt = std::stable_partition(  std::begin(args), std::end(args),
         []( auto&& d ) { return d.is_positional(); } );
   auto hasPositional = iopt != std::begin( args );
   auto hasOptional = iopt != std::end( args );
   auto argWidth = deriveMaxArgumentWidth( args );

   if ( !config.usage.empty() )
      out << "usage: " << config.usage << "\n\n";

   if ( !config.description.empty() )
      out << config.description << "\n\n";

   if ( hasPositional ) {
      out << "positional arguments:\n";
      for ( auto it = std::begin( args ); it != iopt; ++it )
         out << " " << std::left << std::setw( argWidth )
            << formatArgument( *it ) << " " << it->help << "\n";
      out << "\n";
   }

   if ( hasOptional ) {
      out << "optional arguments:\n";
      for ( auto it = iopt; it != std::end( args ); ++it )
         out << " " << std::left << std::setw( argWidth )
            << formatArgument( *it ) << " " << it->help << "\n";
      out << "\n";
   }

   if ( !config.epilog.empty() )
      out << config.epilog << "\n\n";
}

}
