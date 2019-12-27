// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <string>
#include <string_view>

namespace argumentum {

class ParseResult;
class argument_parser;

class Options
{
public:
   virtual void add_arguments( argument_parser& parser ) = 0;
};

class CommandOptions : public Options
{
   std::string mName;

public:
   CommandOptions( std::string_view name );
   const std::string& getName() const;
   virtual void execute( const ParseResult& result );
};

}   // namespace argumentum
