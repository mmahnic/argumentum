
// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "parserconfig.h"

#include <string_view>
#include <vector>

namespace argumentum {

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
   Option* findOption( std::string_view optionName ) const;
   Command* findCommand( std::string_view commandName ) const;

   /**
    * Get a reference to the parser configuration for inspection.
    */
   const ParserConfig::Data& getConfig() const;

   /**
    * @Returns true if there are short options that include digits.
    */
   bool hasNumericOptions() const;
};

}   // namespace argumentum
