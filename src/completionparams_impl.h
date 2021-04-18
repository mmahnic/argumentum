// Copyright (c) 2021 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "completionparams.h"

#include "argparser.h"

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
{}

}   // namespace argumentum
