// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "argumentstream.h"
#include "commandconfig.h"
#include "environment.h"
#include "groupconfig.h"
#include "helpformatter.h"
#include "optionconfig.h"
#include "parserconfig.h"
#include "parserdefinition.h"
#include "parseresult.h"
#include "values.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace argparse {

class argument_parser
{
   friend class Parser;

private:
   ParserDefinition mParserDef;
   std::set<std::string> mHelpOptionNames;
   std::vector<std::shared_ptr<Options>> mTargets;
   std::map<std::string, std::shared_ptr<OptionGroup>> mGroups;
   std::shared_ptr<OptionGroup> mpActiveGroup;

public:
   /**
    * Get a reference to the parser configuration through which the parser can
    * be configured.
    */
   ParserConfig& config();

   /**
    * Get a reference to the parser configuration for inspection.
    */
   const ParserConfig::Data& getConfig() const;
   /**
    * Get a reference to the definition of the parser for inspection.
    */
   const ParserDefinition& getDefinition() const;

   // Define a command.  The CommandOptions (@p TOptions) will be instantiated
   // when the command is activated with an input argument.
   template<typename TOptions>
   CommandConfig add_command( const std::string& name );

   // Define a command. The @p factory will create an instance of CommandOptions
   // when the command is activated with an input argument.
   CommandConfig add_command( const std::string& name, Command::options_factory_t factory );

   template<typename TValue, typename = std::enable_if_t<std::is_base_of<Value, TValue>::value>>
   OptionConfigA<TValue> add_argument(
         TValue value, const std::string& name = "", const std::string& altName = "" );

   /**
    * Add an argument with names @p name and @p altName and store the reference
    * to @p target value that will receive the parsed parameter(s).
    */
   template<typename TValue, typename = std::enable_if_t<!std::is_base_of<Value, TValue>::value>>
   OptionConfigA<TValue> add_argument(
         TValue& value, const std::string& name = "", const std::string& altName = "" );

   /**
    * Add the @p pOptions structure and call its add_arguments method to add
    * the arguments to the parser.  The pointer to @p pOptions is stored in the
    * parser so that the structure outlives the parser.
    */
   void add_arguments( std::shared_ptr<Options> pOptions );

   /**
    * Add default help options --help and -h that will display the help and
    * terminate the parser.
    *
    * The method will throw an invalid_argument exception if none of the option
    * names --help and -h can be used.
    *
    * This method will be called from parse_args if neither it nor the method
    * add_help_option were called before parse_args.
    */
   VoidOptionConfig add_default_help_option();

   /**
    * Add a special option that will display the help and terminate the parser.
    *
    * If neither this method nor add_default_help_option is called, the default
    * help options --help and -h will be used as long as they are not used for
    * other purposes.
    */
   VoidOptionConfig add_help_option( const std::string& name, const std::string& altName = "" );

   // Begin a group of options named @p name. The group definition ends at
   // end_group().
   GroupConfig add_group( const std::string& name );

   // Begin an exclusive group of options named @p name.  At most one of the
   // options from an exclusive group can be used in input arguments.  The group
   // definition ends at end_group().
   GroupConfig add_exclusive_group( const std::string& name );

   // End a group.
   void end_group();

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
   void resetOptionValues();
   void assignDefaultValues();
   void verifyDefinedOptions();
   void validateParsedOptions( ParseResultBuilder& result );
   void reportMissingOptions( ParseResultBuilder& result );
   bool hasRequiredArguments() const;
   void reportExclusiveViolations( ParseResultBuilder& result );
   void reportMissingGroups( ParseResultBuilder& result );
   OptionConfig tryAddArgument( Option& newOption, std::vector<std::string_view> names );
   OptionConfig addPositional( Option&& newOption, const std::vector<std::string_view>& names );
   OptionConfig addOption( Option&& newOption, const std::vector<std::string_view>& names );
   void trySetNames( Option& option, const std::vector<std::string_view>& names ) const;
   void ensureIsNewOption( const std::string& name );
   CommandConfig tryAddCommand( Command& command );
   void ensureIsNewCommand( const std::string& name );
   std::shared_ptr<OptionGroup> addGroup( std::string name, bool isExclusive );
   std::shared_ptr<OptionGroup> findGroup( std::string name ) const;
   void generate_help();
   void describe_errors( ParseResult& result );
};

}   // namespace argparse

#include "argparser_impl.h"
#include "helpformatter_impl.h"
#include "parser_impl.h"
#include "parserdefinition_impl.h"
