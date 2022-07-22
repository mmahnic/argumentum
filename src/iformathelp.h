// Copyright (c) 2018-2021 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#ifndef ARGUMENTUM_BUILD_MODULE
#include <string>
#include <vector>
#endif

namespace argumentum {

class ParserDefinition;

struct ArgumentHelpResult
{
   std::string help_name;
   std::string short_name;
   std::string long_name;
   std::vector<std::string> metavar;
   std::string arguments;
   std::string help;
   bool isRequired = false;
   bool isCommand = false;
   struct
   {
      std::string name;
      std::string title;
      std::string description;
      bool isExclusive = false;
      bool isRequired = false;
   } group;

   bool is_positional() const
   {
      return short_name.substr( 0, 1 ) != "-" && long_name.substr( 0, 1 ) != "-";
   }

   bool is_required() const
   {
      return isRequired || is_positional();
   }
};

class IFormatHelp
{
public:
   virtual void format( const ParserDefinition& parserDef, std::ostream& out ) = 0;
};

}   // namespace argumentum
