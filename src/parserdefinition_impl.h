// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "command.h"
#include "option.h"

#include <string_view>

namespace argumentum {

ARGUMENTUM_INLINE Option* ParserDefinition::findOption( std::string_view optionName ) const
{
   for ( auto& pOption : mOptions )
      if ( pOption->hasName( optionName ) )
         return pOption.get();

   return nullptr;
}

ARGUMENTUM_INLINE Command* ParserDefinition::findCommand( std::string_view commandName ) const
{
   for ( auto& pCommand : mCommands )
      if ( pCommand->hasName( commandName ) )
         return pCommand.get();

   return nullptr;
}

ARGUMENTUM_INLINE std::shared_ptr<OptionGroup> ParserDefinition::findGroup( std::string name ) const
{
   std::transform( name.begin(), name.end(), name.begin(), []( char ch ) {
      return char( tolower( ch ) );
   } );
   auto igrp = mGroups.find( name );
   if ( igrp == mGroups.end() )
      return {};
   return igrp->second;
}

ARGUMENTUM_INLINE const ParserConfig::Data& ParserDefinition::getConfig() const
{
   return mConfig.data();
}

ARGUMENTUM_INLINE bool ParserDefinition::hasNumericOptions() const
{
   for ( auto& pOption : mOptions )
      if ( pOption->isShortNumeric() )
         return true;

   return false;
}

}   // namespace argumentum
