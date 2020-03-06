// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "argparser.h"
#include "argumentconfig.h"
#include "optionpack.h"

namespace argumentum {

void Options::add_arguments( argument_parser& parser )
{
   auto args = parser.arguments();
   add_arguments( args );
}

ARGUMENTUM_INLINE CommandOptions::CommandOptions( std::string_view name )
   : mName( name )
{}

ARGUMENTUM_INLINE const std::string& CommandOptions::getName() const
{
   return mName;
}

ARGUMENTUM_INLINE void CommandOptions::execute( const ParseResult& result )
{}

}   // namespace argumentum
