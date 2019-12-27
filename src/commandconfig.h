// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

namespace argumentum {

class Command;

class CommandConfig
{
   std::shared_ptr<Command> mpCommand;

public:
   CommandConfig( const std::shared_ptr<Command>& pCommand );

   // Define the description of the command that will be displayed in the
   // generated help.
   CommandConfig& help( std::string_view help );

private:
   Command& getCommand();
};

}   // namespace argumentum
