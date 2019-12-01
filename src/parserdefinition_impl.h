// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "command.h"
#include "option.h"

#include <string_view>

namespace argparse {

CPPARGPARSE_INLINE Option* ParserDefinition::findOption( std::string_view optionName )
{
   for ( auto& pOption : mOptions )
      if ( pOption->hasName( optionName ) )
         return pOption.get();

   return nullptr;
}

CPPARGPARSE_INLINE Command* ParserDefinition::findCommand( std::string_view commandName )
{
   for ( auto& pCommand : mCommands )
      if ( pCommand->hasName( commandName ) )
         return pCommand.get();

   return nullptr;
}

CPPARGPARSE_INLINE const ParserConfig::Data& ParserDefinition::getConfig() const
{
   return mConfig.data();
}

}   // namespace argparse
