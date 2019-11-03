// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace argparse {

class argument_parser;
class Writer;

struct ArgumentHelpResult
{
   std::string help_name;
   std::string short_name;
   std::string long_name;
   std::string metavar;
   std::string arguments;
   std::string help;
   bool isRequired = false;
   bool isCommand = false;
   struct
   {
      std::string name;
      std::string title;
      std::string description;
      bool isExclusive = false;
      bool isRequired = false;
   } group;

   bool is_positional() const
   {
      return short_name.substr( 0, 1 ) != "-" && long_name.substr( 0, 1 ) != "-";
   }

   bool is_required() const
   {
      return isRequired || is_positional();
   }
};

class HelpFormatter
{
   // The number of spaces before the argument names.
   size_t mArgumentIndent = 2;

   // The width of the formatted text in bytes.
   size_t mTextWidth = 80;

   // The maximum width of an argument at which the description of the argument
   // can start in the same line.
   size_t mMaxDescriptionIndent = 30;

public:
   void format( const argument_parser& parser, std::ostream& out );

   void setTextWidth( size_t widthBytes )
   {
      mTextWidth = widthBytes;
   }

   void setMaxDescriptionIndent( size_t widthBytes )
   {
      mMaxDescriptionIndent = widthBytes;
   }

private:
   void formatUsage(
         const argument_parser& parser, std::vector<ArgumentHelpResult>& args, Writer& writer );

   std::string formatArgument( const ArgumentHelpResult& arg ) const
   {
      if ( arg.isCommand )
         return arg.help_name;
      else if ( arg.is_positional() )
         return arg.help_name;

      std::string res;
      if ( !arg.short_name.empty() && !arg.long_name.empty() )
         res = arg.short_name + ", " + arg.long_name;
      else if ( !arg.short_name.empty() )
         res = arg.short_name;
      else if ( !arg.long_name.empty() )
         res = "    " + arg.long_name;

      return !arg.arguments.empty() ? res + " " + arg.arguments : res;
   }

   size_t deriveMaxArgumentWidth( const std::vector<ArgumentHelpResult>& args ) const
   {
      if ( args.empty() )
         return 0U;

      auto imax =
            std::max_element( std::begin( args ), std::end( args ), [this]( auto&& a, auto&& b ) {
               return formatArgument( a ).size() < formatArgument( b ).size();
            } );

      return formatArgument( *imax ).size();
   }
};

}   // namespace argparse
