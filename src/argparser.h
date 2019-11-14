// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "argdescriber.h"
#include "commandconfig.h"
#include "commands.h"
#include "environment.h"
#include "exceptions.h"
#include "groupconfig.h"
#include "groups.h"
#include "helpformatter.h"
#include "notifier.h"
#include "optionconfig.h"
#include "options.h"
#include "parser.h"
#include "parserconfig.h"
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
   ParserConfig& config()
   {
      return mParserDef.mConfig;
   }

   /**
    * Get a reference to the parser configuration for inspection.
    */
   const ParserConfig::Data& getConfig() const
   {
      return mParserDef.getConfig();
   }

   /**
    * Get a reference to the definition of the parser for inspection.
    */
   const ParserDefinition& getDefinition() const
   {
      return mParserDef;
   }

   // Add a factory that will create an Options structure for subcommand
   // arguments for the command @p name.
   CommandConfig add_command( const std::string& name, Command::options_factory_t factory )
   {
      auto command = Command( name, factory );
      return tryAddCommand( command );
   }

   template<typename TValue, typename = std::enable_if_t<std::is_base_of<Value, TValue>::value>>
   OptionConfigA<TValue> add_argument(
         TValue value, const std::string& name = "", const std::string& altName = "" )
   {
      auto option = Option( value );
      return OptionConfigA<TValue>( tryAddArgument( option, { name, altName } ) );
   }

   /**
    * Add an argument with names @p name and @p altName and store the reference
    * to @p target value that will receive the parsed parameter(s).
    */
   template<typename TValue, typename = std::enable_if_t<!std::is_base_of<Value, TValue>::value>>
   OptionConfigA<TValue> add_argument(
         TValue& value, const std::string& name = "", const std::string& altName = "" )
   {
      auto option = Option( value );
      return OptionConfigA<TValue>( tryAddArgument( option, { name, altName } ) );
   }

   /**
    * Add the @p pOptions structure and call its add_arguments method to add
    * the arguments to the parser.  The pointer to @p pOptions is stored in the
    * parser so that the structure outlives the parser.
    */
   void add_arguments( std::shared_ptr<Options> pOptions )
   {
      if ( pOptions ) {
         mTargets.push_back( pOptions );
         pOptions->add_arguments( *this );
      }
   }

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
   VoidOptionConfig add_default_help_option()
   {
      const auto shortName = "-h";
      const auto longName = "--help";
      auto pShort = mParserDef.findOption( shortName );
      auto pLong = mParserDef.findOption( longName );

      if ( !pShort && !pLong )
         return add_help_option( shortName, longName );
      if ( !pShort )
         return add_help_option( shortName );
      if ( !pLong )
         return add_help_option( longName );

      throw std::invalid_argument( "The default help options are hidden by other options." );
   }

   /**
    * Add a special option that will display the help and terminate the parser.
    *
    * If neither this method nor add_default_help_option is called, the default
    * help options --help and -h will be used as long as they are not used for
    * other purposes.
    */
   VoidOptionConfig add_help_option( const std::string& name, const std::string& altName = "" )
   {
      if ( !name.empty() && name[0] != '-' || !altName.empty() && altName[0] != '-' )
         throw std::invalid_argument( "A help argument must be an option." );

      auto value = VoidValue{};
      auto option = Option( value );
      auto optionConfig = VoidOptionConfig( tryAddArgument( option, { name, altName } ) )
                                .help( "Display this help message and exit." );

      if ( !name.empty() )
         mHelpOptionNames.insert( name );
      if ( !altName.empty() )
         mHelpOptionNames.insert( altName );

      return optionConfig;
   }

   // Begin a group of options named @p name. The group definition ends at
   // end_group().
   GroupConfig add_group( const std::string& name )
   {
      auto pGroup = findGroup( name );
      if ( pGroup ) {
         if ( pGroup->isExclusive() )
            throw MixingGroupTypes( name );
         mpActiveGroup = pGroup;
      }
      else
         mpActiveGroup = addGroup( name, false );

      return GroupConfig( mpActiveGroup );
   }

   // Begin an exclusive group of options named @p name.  At most one of the
   // options from an exclusive can be used in input arguments.  The group
   // definition ends at end_group().
   GroupConfig add_exclusive_group( const std::string& name )
   {
      auto pGroup = findGroup( name );
      if ( pGroup ) {
         if ( !pGroup->isExclusive() )
            throw MixingGroupTypes( name );
         mpActiveGroup = pGroup;
      }
      else
         mpActiveGroup = addGroup( name, true );

      return GroupConfig( mpActiveGroup );
   }

   // End a group.
   void end_group()
   {
      mpActiveGroup = nullptr;
   }

   // Parse input arguments and return errors in a ParseResult.
   ParseResult parse_args( int argc, char** argv, int skip_args = 1 )
   {
      if ( !argv ) {
         auto res = ParseResultBuilder{};
         res.addError( "argv", INVALID_ARGV );
         return res.getResult();
      }

      std::vector<std::string> args;
      for ( int i = std::max( 0, skip_args ); i < argc; ++i )
         args.emplace_back( argv[i] );

      return parse_args( std::begin( args ), std::end( args ) );
   }

   // Parse input arguments and return errors in a ParseResult.
   ParseResult parse_args( const std::vector<std::string>& args, int skip_args = 0 )
   {
      auto ibegin = std::begin( args );
      if ( skip_args > 0 )
         ibegin += std::min<size_t>( skip_args, args.size() );

      return parse_args( ibegin, std::end( args ) );
   }

   // Parse input arguments and return errors in a ParseResult.
   ParseResult parse_args( std::vector<std::string>::const_iterator ibegin,
         std::vector<std::string>::const_iterator iend )
   {
      verifyDefinedOptions();
      ParseResultBuilder result;
      parse_args( ibegin, iend, result );

      if ( result.hasArgumentProblems() ) {
         result.signalErrorsShown();
         auto res = std::move( result.getResult() );
         describe_errors( res );
         return std::move( res );
      }

      return std::move( result.getResult() );
   }

   ArgumentHelpResult describe_argument( std::string_view name ) const
   {
      ArgumentDescriber describer;
      return describer.describe_argument( mParserDef, name );
   }

   std::vector<ArgumentHelpResult> describe_arguments() const
   {
      ArgumentDescriber describer;
      return describer.describe_arguments( mParserDef );
   }

private:
   void parse_args( std::vector<std::string>::const_iterator ibegin,
         std::vector<std::string>::const_iterator iend, ParseResultBuilder& result )
   {
      resetOptionValues();

      if ( mustDisplayHelp( ibegin, iend ) ) {
         generate_help();
         result.signalHelpShown();
         result.requestExit();
         return;
      }

      Parser parser( mParserDef, result );
      parser.parse( ibegin, iend );
      if ( result.wasExitRequested() ) {
         result.addError( {}, EXIT_REQUESTED );
         return;
      }

      assignDefaultValues();
      validateParsedOptions( result );
   }

   void resetOptionValues()
   {
      for ( auto& pOption : mParserDef.mOptions )
         pOption->resetValue();

      for ( auto& pOption : mParserDef.mPositional )
         pOption->resetValue();
   }

   bool mustDisplayHelp( std::vector<std::string>::const_iterator ibegin,
         std::vector<std::string>::const_iterator iend ) const
   {
      if ( ibegin == iend && hasRequiredArguments() )
         return true;

      auto isHelpOption = [this]( auto&& arg ) { return mHelpOptionNames.count( arg ) > 0; };
      return std::any_of( ibegin, iend, isHelpOption );
   }

   void assignDefaultValues()
   {
      for ( auto& pOption : mParserDef.mOptions )
         if ( !pOption->wasAssigned() && pOption->hasDefault() )
            pOption->assignDefault();

      for ( auto& pOption : mParserDef.mPositional )
         if ( !pOption->wasAssigned() && pOption->hasDefault() )
            pOption->assignDefault();
   }

   void verifyDefinedOptions()
   {
      // Check if any help options are defined and add the default if not.
      if ( mHelpOptionNames.empty() ) {
         end_group();
         try {
            add_default_help_option();
         }
         catch ( const std::invalid_argument& ) {
            Notifier::warn( "Failed to add default help options." );
         }
      }

      // A required option can not be in an exclusive group.
      for ( auto& pOption : mParserDef.mOptions ) {
         if ( pOption->isRequired() ) {
            auto pGroup = pOption->getGroup();
            if ( pGroup && pGroup->isExclusive() )
               throw RequiredExclusiveOption( pOption->getName(), pGroup->getName() );
         }
      }
   }

   void validateParsedOptions( ParseResultBuilder& result )
   {
      reportMissingOptions( result );
      reportExclusiveViolations( result );
      reportMissingGroups( result );
   }

   void reportMissingOptions( ParseResultBuilder& result )
   {
      for ( auto& pOption : mParserDef.mOptions )
         if ( pOption->isRequired() && !pOption->wasAssigned() )
            result.addError( pOption->getHelpName(), MISSING_OPTION );

      for ( auto& pOption : mParserDef.mPositional )
         if ( pOption->needsMoreArguments() )
            result.addError( pOption->getHelpName(), MISSING_ARGUMENT );
   }

   bool hasRequiredArguments() const
   {
      for ( auto& pOption : mParserDef.mOptions )
         if ( pOption->isRequired() )
            return true;

      for ( auto& pOption : mParserDef.mPositional )
         if ( pOption->isRequired() )
            return true;

      return false;
   }

   void reportExclusiveViolations( ParseResultBuilder& result )
   {
      std::map<std::string, std::vector<std::string>> counts;
      for ( auto& pOption : mParserDef.mOptions ) {
         auto pGroup = pOption->getGroup();
         if ( pGroup && pGroup->isExclusive() && pOption->wasAssigned() )
            counts[pGroup->getName()].push_back( pOption->getHelpName() );
      }

      for ( auto& c : counts )
         if ( c.second.size() > 1 )
            result.addError( c.second.front(), EXCLUSIVE_OPTION );
   }

   void reportMissingGroups( ParseResultBuilder& result )
   {
      std::map<std::string, int> counts;
      for ( auto& pOption : mParserDef.mOptions ) {
         auto pGroup = pOption->getGroup();
         if ( pGroup && pGroup->isRequired() )
            counts[pGroup->getName()] += pOption->wasAssigned() ? 1 : 0;
      }

      for ( auto& c : counts )
         if ( c.second < 1 )
            result.addError( c.first, MISSING_OPTION_GROUP );
   }

   OptionConfig tryAddArgument( Option& newOption, std::vector<std::string_view> names )
   {
      // Remove empty names
      auto is_empty = [&]( auto&& name ) { return name.empty(); };
      names.erase( std::remove_if( names.begin(), names.end(), is_empty ), names.end() );

      if ( names.empty() )
         throw std::invalid_argument( "An argument must have a name." );

      for ( auto& name : names )
         for ( auto ch : name )
            if ( std::isspace( ch ) )
               throw std::invalid_argument( "Argument names must not contain spaces." );

      auto has_dash = []( auto name ) { return name[0] == '-'; };

      auto isOption = [&]( auto&& names ) -> bool {
         return std::all_of( names.begin(), names.end(), has_dash );
      };

      auto isPositional = [&]( auto&& names ) -> bool {
         return std::none_of( names.begin(), names.end(), has_dash );
      };

      if ( isPositional( names ) )
         return addPositional( std::move( newOption ), names );
      else if ( isOption( names ) )
         return addOption( std::move( newOption ), names );

      throw std::invalid_argument( "The argument must be either positional or an option." );
   }

   OptionConfig addPositional( Option&& newOption, const std::vector<std::string_view>& names )
   {
      auto pOption = std::make_shared<Option>( std::move( newOption ) );
      auto& option = *pOption;

      option.setLongName( names.empty() ? "arg" : names[0] );
      option.setRequired( true );

      if ( option.hasVectorValue() )
         option.setMinArgs( 0 );
      else
         option.setNArgs( 1 );

      // Positional parameters are required so they can't be in an exclusive
      // group.  We simply ignore them.
      if ( mpActiveGroup && !mpActiveGroup->isExclusive() )
         option.setGroup( mpActiveGroup );

      mParserDef.mPositional.push_back( pOption );
      return { pOption };
   }

   OptionConfig addOption( Option&& newOption, const std::vector<std::string_view>& names )
   {
      trySetNames( newOption, names );
      ensureIsNewOption( newOption.getLongName() );
      ensureIsNewOption( newOption.getShortName() );

      auto pOption = std::make_shared<Option>( std::move( newOption ) );

      if ( mpActiveGroup )
         pOption->setGroup( mpActiveGroup );

      mParserDef.mOptions.push_back( pOption );
      return { pOption };
   }

   void trySetNames( Option& option, const std::vector<std::string_view>& names ) const
   {
      for ( auto name : names ) {
         if ( name.empty() || name == "-" || name == "--" || name[0] != '-' )
            continue;

         if ( name.substr( 0, 2 ) == "--" )
            option.setLongName( name );
         else if ( name.substr( 0, 1 ) == "-" ) {
            if ( name.size() > 2 )
               throw std::invalid_argument( "Short option name has too many characters." );
            option.setShortName( name );
         }
      }

      if ( option.getName().empty() )
         throw std::invalid_argument( "An option must have a name." );
   }

   void ensureIsNewOption( const std::string& name )
   {
      if ( name.empty() )
         return;

      auto pOption = mParserDef.findOption( name );
      if ( pOption ) {
         auto groupName = pOption->getGroup() ? pOption->getGroup()->getName() : "";
         throw DuplicateOption( groupName, name );
      }
   }

   CommandConfig tryAddCommand( Command& command )
   {
      if ( command.getName().empty() )
         throw std::invalid_argument( "A command must have a name." );
      if ( !command.hasFactory() )
         throw std::invalid_argument( "A command must have an options factory." );
      if ( command.getName()[0] == '-' )
         throw std::invalid_argument( "Command name must not start with a dash." );

      ensureIsNewCommand( command.getName() );

      auto pCommand = std::make_shared<Command>( std::move( command ) );
      mParserDef.mCommands.push_back( pCommand );
      return { pCommand };
   }

   void ensureIsNewCommand( const std::string& name )
   {
      auto pCommand = mParserDef.findCommand( name );
      if ( pCommand )
         throw DuplicateCommand( name );
   }

   std::shared_ptr<OptionGroup> addGroup( std::string name, bool isExclusive )
   {
      if ( name.empty() )
         throw std::invalid_argument( "A group must have a name." );

      std::transform( name.begin(), name.end(), name.begin(), tolower );
      assert( mGroups.count( name ) == 0 );

      auto pGroup = std::make_shared<OptionGroup>( name, isExclusive );
      mGroups[name] = pGroup;
      return pGroup;
   }

   std::shared_ptr<OptionGroup> findGroup( std::string name ) const
   {
      std::transform( name.begin(), name.end(), name.begin(), tolower );
      auto igrp = mGroups.find( name );
      if ( igrp == mGroups.end() )
         return {};
      return igrp->second;
   }

   void generate_help()
   {
      // TODO: The formatter should be configurable
      auto formatter = HelpFormatter();
      auto pStream = mParserDef.getConfig().pOutStream;
      if ( !pStream )
         pStream = &std::cout;

      formatter.format( mParserDef, *pStream );
   }

   void describe_errors( ParseResult& result )
   {
      auto pStream = mParserDef.getConfig().pOutStream;
      if ( !pStream )
         pStream = &std::cout;

      for ( const auto& e : result.errors )
         e.describeError( *pStream );

      if ( !result.ignoredArguments.empty() ) {
         auto it = result.ignoredArguments.begin();
         *pStream << "Error: Ignored arguments: " << *it;
         for ( ++it; it != result.ignoredArguments.end(); ++it )
            *pStream << ", " << *it;
         *pStream << "\n";
      }
   }
};

}   // namespace argparse

#include "helpformatter_impl.h"
#include "parser_impl.h"
