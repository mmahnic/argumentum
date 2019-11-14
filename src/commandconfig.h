// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "commands.h"

namespace argparse {

class CommandConfig
{
   std::shared_ptr<Command> mpCommand;

public:
   CommandConfig( const std::shared_ptr<Command>& pCommand )
      : mpCommand( pCommand )
   {
      assert( pCommand );
      if ( !mpCommand )
         throw std::invalid_argument( "CommandConfig requires a command." );
   }

   // Define the description of the command that will be displayed in the
   // generated help.
   CommandConfig& help( std::string_view help )
   {
      getCommand().setHelp( help );
      return *this;
   }

private:
   Command& getCommand()
   {
      return *mpCommand;
   }
};

}   // namespace argparse
