// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "argumentstream.h"
#include "commandconfig.h"
#include "environment.h"
#include "groupconfig.h"
#include "helpformatter.h"
#include "optionconfig.h"
#include "optionfactory.h"
#include "optionpack.h"
#include "parameterconfig.h"
#include "parserconfig.h"
#include "parserdefinition.h"
#include "parseresult.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace argumentum {

class argument_parser
{
   friend class Parser;
   friend class ParameterConfig;

private:
   bool mTopLevel = true;
   ParserDefinition mParserDef;
   std::unique_ptr<OptionFactory> mpOptionFactory;

public:
   /**
    * Get a reference to the parser configuration through which the parser can
    * be configured.
    */
   ParserConfig& config();

   /**
    * Get an instance of the parameter configuration through which parameter can
    * be defined.
    */
   ParameterConfig params()
   {
      return ParameterConfig( *this );
   }

   /**
    * Get a reference to the parser configuration for inspection.
    */
   const ParserConfig::Data& getConfig() const;

   /**
    * Get a reference to the definition of the parser for inspection.
    */
   const ParserDefinition& getDefinition() const;

// Attributes are not handled well by clang-format so we use a macro.
#define ARGUMENTUM_DEPRECATED( x ) [[deprecated( x )]]

   // Deprecated. Use args = parser.params(); args.add_command(...);
   ARGUMENTUM_DEPRECATED( "Use parser.params()" )
   CommandConfig add_command( std::shared_ptr<CommandOptions> pOptions )
   {
      return params().add_command( pOptions );
   };

   // Deprecated. Use args = parser.params(); args.add_command(...);
   template<typename TOptions>
   ARGUMENTUM_DEPRECATED( "Use parser.params()" )
   CommandConfig add_command( const std::string& name )
   {
      return params().add_command<TOptions>( name );
   };

   // Deprecated. Use args = parser.params(); args.add_command(...);
   ARGUMENTUM_DEPRECATED( "Use parser.params()" )
   CommandConfig add_command( const std::string& name, Command::options_factory_t factory )
   {
      return params().add_command( name, factory );
   }

   // Deprecated. Use args = parser.params(); args.add_parameter(...);
   template<typename TTarget>
   ARGUMENTUM_DEPRECATED( "Use parser.params()" )
   OptionConfigA<TTarget> add_argument(
         TTarget& target, const std::string& name = "", const std::string& altName = "" )
   {
      return params().add_parameter( target, name, altName );
      // return params().add_argument( std::forward<TTarget>( target ), name, altName );
   }

   // Deprecated. Use args = parser.params(); args.add_parameters(...);
   ARGUMENTUM_DEPRECATED( "Use parser.params()" )
   void add_arguments( std::shared_ptr<Options> pOptions )
   {
      return params().add_parameters( pOptions );
   }

   // Deprecated. Use args = parser.params(); args.add_default_help_option(...);
   ARGUMENTUM_DEPRECATED( "Use parser.params()" )
   VoidOptionConfig add_default_help_option()
   {
      return params().add_default_help_option();
   }

   // Deprecated. Use args = parser.params(); args.add_help_option(...);
   ARGUMENTUM_DEPRECATED( "Use parser.params()" )
   VoidOptionConfig add_help_option( const std::string& name, const std::string& altName = "" )
   {
      return params().add_help_option( name, altName );
   }

   // Deprecated. Use args = parser.params(); args.add_group(...);
   ARGUMENTUM_DEPRECATED( "Use parser.params()" )
   GroupConfig add_group( const std::string& name )
   {
      return params().add_group( name );
   }

   // Deprecated. Use args = parser.params(); args.add_exclusive_group(...);
   ARGUMENTUM_DEPRECATED( "Use parser.params()" )
   GroupConfig add_exclusive_group( const std::string& name )
   {
      return params().add_exclusive_group( name );
   }

   // Deprecated. Use args = parser.params(); args.end_group(...);
   ARGUMENTUM_DEPRECATED( "Use parser.params()" )
   void end_group()
   {
      params().end_group();
   }

#undef ARGUMENTUM_DEPRECATED

   // Parse input arguments and return commands and errors in a ParseResult.
   ParseResult parse_args( int argc, char** argv, int skip_args = 1 );

   // Parse input arguments and return commands and errors in a ParseResult.
   ParseResult parse_args( const std::vector<std::string>& args, int skip_args = 0 );

   // Parse input arguments and return commands and errors in a ParseResult.
   ParseResult parse_args( std::vector<std::string>::const_iterator ibegin,
         std::vector<std::string>::const_iterator iend );

   // Parse input arguments and return errors in a ParseResult.
   ParseResult parse_args( ArgumentStream& args );

   ArgumentHelpResult describe_argument( std::string_view name ) const;
   std::vector<ArgumentHelpResult> describe_arguments() const;

private:
   static argument_parser createSubParser();
   void resetOptionValues();
   void assignDefaultValues();
   void verifyDefinedOptions();
   void validateParsedOptions( ParseResultBuilder& result );
   void reportMissingOptions( ParseResultBuilder& result );
   bool hasRequiredArguments() const;
   void reportExclusiveViolations( ParseResultBuilder& result );
   void reportMissingGroups( ParseResultBuilder& result );
   void describe_errors( ParseResult& result );
   // TODO (mmahnic): remove, moved to ParameterConfig
   OptionFactory& getOptionFactory();
};

}   // namespace argumentum
