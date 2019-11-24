// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "commandconfig.h"

#include "commands.h"

namespace argparse {

inline CommandConfig::CommandConfig( const std::shared_ptr<Command>& pCommand )
   : mpCommand( pCommand )
{
   assert( pCommand );
   if ( !mpCommand )
      throw std::invalid_argument( "CommandConfig requires a command." );
}

// Define the description of the command that will be displayed in the
// generated help.
inline CommandConfig& CommandConfig::help( std::string_view help )
{
   getCommand().setHelp( help );
   return *this;
}

inline Command& CommandConfig::getCommand()
{
   return *mpCommand;
}

}   // namespace argparse
