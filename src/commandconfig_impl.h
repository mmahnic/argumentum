// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "commandconfig.h"

#include "command.h"

namespace argumentum {

ARGUMENTUM_INLINE CommandConfig::CommandConfig( const std::shared_ptr<Command>& pCommand )
   : mpCommand( pCommand )
{
   assert( pCommand );
   if ( !mpCommand )
      throw std::invalid_argument( "CommandConfig requires a command." );
}

// Define the description of the command that will be displayed in the
// generated help.
ARGUMENTUM_INLINE CommandConfig& CommandConfig::help( std::string_view help )
{
   getCommand().setHelp( help );
   return *this;
}

ARGUMENTUM_INLINE Command& CommandConfig::getCommand()
{
   return *mpCommand;
}

}   // namespace argumentum
