// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#ifndef ARGUMENTUM_BUILD_MODULE
#include <string>
#include <string_view>
#endif

namespace argumentum {

class ParseResult;
class argument_parser;
class ParameterConfig;

class Options
{
public:
   virtual ~Options() = default;
   virtual void add_parameters( ParameterConfig& args )
   {
      // TODO (mmahnic): make method add_parameters( ParameterConfig) abstract.
   }

#ifdef ARGUMENTUM_DEPRECATED_ATTR
// Attributes are not handled well by clang-format so we use a macro.
#define ARGUMENTUM_DEPRECATED( x ) [[deprecated( x )]]
#else
#define ARGUMENTUM_DEPRECATED( x )
#endif

   // Deprecated. Use add_arguments( ParameterConfig ).
   ARGUMENTUM_DEPRECATED( "Use add_parameters( ParameterConfig )" )
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
