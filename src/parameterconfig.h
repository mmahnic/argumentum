// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "command.h"
#include "commandconfig.h"
#include "groupconfig.h"
#include "optionconfig.h"
#include "optionfactory.h"
#include "optionpack.h"

#ifndef ARGUMENTUM_BUILD_MODULE
#include "nomodule.h"

#include <memory>
#include <string>
#endif

namespace argumentum {

class argument_parser;
class ParserDefinition;

ARGUMENTUM_EXPORT
class ParameterConfig
{
   friend class argument_parser;
   // TODO (mmahnic): this should be ParserDefinition mParserDef.
   argument_parser& mParser;
   ParserDefinition& mParserDef;

public:
   // Define a command.  The options from @p pOptions will be registered only
   // when the command is activated with an input argument.
   CommandConfig add_command( std::shared_ptr<CommandOptions> pOptions );

   // Define a command.  The CommandOptions (@p TOptions) will be instantiated
   // when the command is activated with an input argument.
   template<typename TOptions>
   CommandConfig add_command( const std::string& name )
   {
      auto factory = []( std::string_view name ) { return std::make_shared<TOptions>( name ); };
      auto command = Command( name, factory );
      return tryAddCommand( command );
   }

   // Define a command. The @p factory will create an instance of CommandOptions
   // when the command is activated with an input argument.
   CommandConfig add_command( const std::string& name, Command::options_factory_t factory );

   /**
    * Add an argument with names @p name and @p altName and store the reference
    * to @p target value that will receive the parsed parameter(s).
    */
   template<typename TTarget>
   OptionConfigA<TTarget> add_parameter(
         TTarget& target, const std::string& name = "", const std::string& altName = "" )
   {
      auto option = getOptionFactory().createOption( target );
      return OptionConfigA<TTarget>( tryAddParameter( option, { name, altName } ) );
   }

   /**
    * Add an argument with names @p name and @p altName and store the reference
    * to @p target value that will receive the parsed parameter(s).  This is an
    * alias for `add_parameter`.
    */
   template<typename TTarget>
   OptionConfigA<TTarget> add(
         TTarget& target, const std::string& name = "", const std::string& altName = "" )
   {
      return add_parameter( target, name, altName );
   }

   /**
    * Add the @p pOptions structure and call its add_parameters method to add
    * the arguments to the parser.  The pointer to @p pOptions is stored in the
    * parser so that the structure outlives the parser.
    */
   void add_parameters( std::shared_ptr<Options> pOptions );

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

private:
   ParameterConfig( argument_parser& parser );

private:
   OptionConfig tryAddParameter( Option& newOption, std::vector<std::string_view> names );
   OptionConfig addPositional( Option&& newOption, const std::vector<std::string_view>& names );
   OptionConfig addOption( Option&& newOption, const std::vector<std::string_view>& names );
   void trySetNames( Option& option, const std::vector<std::string_view>& names ) const;
   void ensureIsNewOption( const std::string& name );
   CommandConfig tryAddCommand( Command& command );
   void ensureIsNewCommand( const std::string& name );
   std::shared_ptr<OptionGroup> addGroup( std::string name, bool isExclusive );
   OptionFactory& getOptionFactory();
};

}   // namespace argumentum
