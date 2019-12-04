
// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "parserconfig.h"

#include <string_view>
#include <vector>

namespace argparse {

class Option;
class Command;

class ParserDefinition
{
public:
   ParserConfig mConfig;
   std::vector<std::shared_ptr<Command>> mCommands;
   std::vector<std::shared_ptr<Option>> mOptions;
   std::vector<std::shared_ptr<Option>> mPositional;

public:
   Option* findOption( std::string_view optionName );
   Command* findCommand( std::string_view commandName );

   /**
    * Get a reference to the parser configuration for inspection.
    */
   const ParserConfig::Data& getConfig() const;
};

}   // namespace argparse
