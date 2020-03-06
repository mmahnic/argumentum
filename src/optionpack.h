// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <string>
#include <string_view>

namespace argumentum {

class ParseResult;
class argument_parser;
class ArgumentConfig;

class Options
{
public:
   virtual void add_arguments( ArgumentConfig& args )
   {
      // TODO (mmahnic): make method add_arguments( ArgumentConfig) abstract.
   }

// Attributes are not handled well by clang-format so we use a macro.
#define ARGUMENTUM_DEPRECATED( x ) [[deprecated( x )]]

   // Deprecated. Use add_arguments( ArgumentConfig ).
   ARGUMENTUM_DEPRECATED( "Use add_arguments( ArgumentConfig )" )
   virtual void add_arguments( argument_parser& parser );

#undef ARGUMENTUM_DEPRECATED
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
