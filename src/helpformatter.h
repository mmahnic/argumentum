// Copyright (c) 2018, 2019 Marko Mahnič
// License: MIT. See LICENSE in the root of the project.

#pragma once

#include "helpformatter_i.h"

#include "argparser.h"

namespace argparse {

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
