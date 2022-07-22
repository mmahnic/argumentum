// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "iformathelp.h"

#ifndef ARGUMENTUM_BUILD_MODULE
#include <string>
#include <string_view>
#include <vector>
#endif

namespace argumentum {

class Option;
class Command;
class ParserDefinition;

class ArgumentDescriber
{
public:
   ArgumentHelpResult describe_argument(
         const ParserDefinition& parserDef, std::string_view name ) const;
   std::vector<ArgumentHelpResult> describe_arguments( const ParserDefinition& parserDef ) const;
   ArgumentHelpResult describeOption( const Option& option ) const;
   ArgumentHelpResult describeCommand( const Command& command ) const;

private:
   std::string describeArguments(
         const Option& option, const std::vector<std::string>& metavar ) const;
};

}   // namespace argumentum
