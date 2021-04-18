// Copyright (c) 2021 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "completionparams.h"

#include "argparser.h"

#include <sstream>

namespace argumentum {

void CompletionParams::splitArguments( std::vector<std::string>::const_iterator ibegin,
      std::vector<std::string>::const_iterator iend )
{
   for ( auto iarg = ibegin; iarg != iend; ++iarg )
      if ( std::string_view( *iarg ).substr( 0, 11 ) == "---complete" )
         completeArgs.push_back( iarg->substr( 1 ) );
      else
         programArgs.push_back( std::string_view( *iarg ) );
}

void CompletionParams::parseCompletionArguments()
{
   auto parser = argument_parser{};
   std::stringstream strout;
   parser.config().cout( strout );
   auto params = parser.params();

   struct _position_
   {
      std::optional<int> index;
      std::optional<int> offset;
      bool isNew{};
   } position;

   // TODO (mmahnic): The help for the completion parser must be included in
   // the main help when --help-complete is requested.  The parser should be
   // encapsulated in an Options structure.

   params.add_parameter( position, "--complete-new" )
         .maxargs( 1 )
         .help( "Complete the word before the index passed as a parameter. "
                "If the parmeter is omitted, the last word will be completed. "
                "Word indices start with 1. " )
         .action( []( auto& position, const std::string& value ) {
            position.index = from_string<int>::convert( value );
            position.isNew = true;
         } );

   params.add_parameter( position, "--complete-extend" )
         .maxargs( 1 )
         .help( "Complete the word at the index passed as a parameter. "
                "If the parmeter is omitted, the last word will be completed. "
                "Word indices start with 1. "
                "By default the whole word is used as a prefix for filtering completions. "
                "If the prefix should be shorter, it can be set after the "
                "character / of the parameter. "
                "---complete-extend=1/5 will return completions for the first word "
                "that start with the first 5 bytes of the word. "
                "---complete-extend=1/0 will return all completions for the first word." )
         .action( []( auto& position, const std::string& value ) {
            position.index = from_string<int>::convert( value );
            position.isNew = false;
         } );

   auto res = parser.parse_args( completeArgs );
   if ( !res ) {
      // TODO (mmahnic): We should return errors to the caller so that they can be reported to the user.
      return;
   }

   if ( position.index ) {
      argumentIndex = *position.index;
      if ( argumentIndex > 0 )
         argumentIndex -= 1;
      isNewArgument = position.isNew;
   }

   if ( position.offset )
      byteOffset = *position.offset;
}

}   // namespace argumentum
