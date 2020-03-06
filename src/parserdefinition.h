// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "parserconfig.h"

#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace argumentum {

class Option;
class OptionGroup;
class Command;

class ParserDefinition
{
   friend class ParameterConfig;

private:
   // TODO (mmahnic): mpActiveGroup should be removed, ParserDefinition should
   // not have any state related to argument configuration.  Groups should be
   // set explicitly with OptionConfig::group().
   std::shared_ptr<OptionGroup> mpActiveGroup;

public:
   ParserConfig mConfig;
   std::vector<std::shared_ptr<Command>> mCommands;
   std::vector<std::shared_ptr<Option>> mOptions;
   std::vector<std::shared_ptr<Option>> mPositional;
   std::map<std::string, std::shared_ptr<OptionGroup>> mGroups;
   std::set<std::string> mHelpOptionNames;

public:
   Option* findOption( std::string_view optionName ) const;
   Command* findCommand( std::string_view commandName ) const;
   std::shared_ptr<OptionGroup> findGroup( std::string name ) const;

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
