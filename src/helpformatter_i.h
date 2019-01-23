// Copyright (c) 2018, 2019 Marko Mahnič
// License: MIT. See LICENSE in the root of the project.

#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>

namespace argparse {

class ArgumentParser;

struct ArgumentHelpResult
{
   std::string short_name;
   std::string long_name;
   std::string help;

   bool is_positional() const
   {
      return short_name.substr( 0, 1 ) != "-" && long_name.substr( 0, 1 ) != "-";
   }
};


class HelpFormatter
{
   size_t mArgumentIndent = 2;

   // The width of the formatted text in bytes.
   size_t mTextWidth = 80;

public:
   void format( const ArgumentParser& parser, std::ostream& out );

   void setTextWidth( size_t widthBytes )
   {
      mTextWidth = widthBytes;
   }

private:
   std::string formatArgument( const ArgumentHelpResult& arg ) const
   {
      if ( !arg.short_name.empty() && !arg.long_name.empty() )
         return arg.short_name + ", " + arg.long_name;
      else if ( !arg.short_name.empty() )
         return arg.short_name;
      else if ( !arg.long_name.empty() )
         return arg.is_positional() ? arg.long_name : "    " + arg.long_name;
      else
         return {};
   }

   size_t deriveMaxArgumentWidth( const std::vector<ArgumentHelpResult>& args ) const
   {
      if ( args.empty() )
         return 0U;

      auto imax = std::max_element( std::begin( args ), std::end( args ),
            [this]( auto&& a, auto&& b ) {
               return formatArgument( a ).size() < formatArgument( b ).size();
            } );

      return formatArgument( *imax ).size();
   }
};

}
