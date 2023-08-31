// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "argparser.h"
#include "optionpack.h"
#include "parameterconfig.h"

namespace argumentum {

void Options::add_parameters( ParameterConfig& )
{}

void Options::add_arguments( argument_parser& parser )
{
   auto params = parser.params();
   add_parameters( params );
}

ARGUMENTUM_INLINE CommandOptions::CommandOptions( std::string_view name )
   : mName( name )
{}

ARGUMENTUM_INLINE const std::string& CommandOptions::getName() const
{
   return mName;
}

ARGUMENTUM_INLINE void CommandOptions::execute( const ParseResult& )
{}

}   // namespace argumentum
