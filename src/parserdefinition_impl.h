// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "commands.h"
#include "options.h"

#include <string_view>

namespace argparse {

inline Option* ParserDefinition::findOption( std::string_view optionName )
{
   for ( auto& pOption : mOptions )
      if ( pOption->hasName( optionName ) )
         return pOption.get();

   return nullptr;
}

inline Command* ParserDefinition::findCommand( std::string_view commandName )
{
   for ( auto& pCommand : mCommands )
      if ( pCommand->hasName( commandName ) )
         return pCommand.get();

   return nullptr;
}

inline const ParserConfig::Data& ParserDefinition::getConfig() const
{
   return mConfig.data();
}

}   // namespace argparse
