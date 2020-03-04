// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "argparser.h"

#include "argdescriber.h"
#include "command.h"
#include "exceptions.h"
#include "group.h"
#include "notifier.h"
#include "option.h"
#include "parser.h"

namespace argumentum {

ARGUMENTUM_INLINE argument_parser argument_parser::createSubParser()
{
   auto parser = argument_parser{};
   parser.mTopLevel = false;
   return parser;
}

ARGUMENTUM_INLINE ParserConfig& argument_parser::config()
{
   return mParserDef.mConfig;
}

ARGUMENTUM_INLINE const ParserConfig::Data& argument_parser::getConfig() const
{
   return mParserDef.getConfig();
}

ARGUMENTUM_INLINE const ParserDefinition& argument_parser::getDefinition() const
{
   return mParserDef;
}

ARGUMENTUM_INLINE OptionFactory& argument_parser::getOptionFactory()
{
   if ( !mpOptionFactory )
      mpOptionFactory = std::make_unique<OptionFactory>();

   return *mpOptionFactory;
}

ARGUMENTUM_INLINE CommandConfig argument_parser::add_command(
      std::shared_ptr<CommandOptions> pOptions )
{
   if ( !pOptions )
      throw MissingCommandOptions( "<unknown>" );

   auto command = Command( pOptions->getName(), pOptions );
   return tryAddCommand( command );
}

ARGUMENTUM_INLINE CommandConfig argument_parser::add_command(
      const std::string& name, Command::options_factory_t factory )
{
   auto command = Command( name, factory );
   return tryAddCommand( command );
}

ARGUMENTUM_INLINE void argument_parser::add_arguments( std::shared_ptr<Options> pOptions )
{
   if ( pOptions ) {
      mTargets.push_back( pOptions );
      pOptions->add_arguments( *this );
   }
}

ARGUMENTUM_INLINE VoidOptionConfig argument_parser::add_default_help_option()
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

ARGUMENTUM_INLINE VoidOptionConfig argument_parser::add_help_option(
      const std::string& name, const std::string& altName )
{
   if ( !name.empty() && name[0] != '-' || !altName.empty() && altName[0] != '-' )
      throw std::invalid_argument( "A help argument must be an option." );

   auto value = VoidValue{};
   auto option = getOptionFactory().createOption( value );
   auto optionConfig =   // (clf)
         VoidOptionConfig( tryAddArgument( option, { name, altName } ) )
               .help( "Display this help message and exit." )
               .action( [this]( const std::string&, Environment& env ) {
                  generate_help();
                  env.notify_help_was_shown();
                  env.exit_parser();
               } );

   if ( !name.empty() )
      mHelpOptionNames.insert( name );
   if ( !altName.empty() )
      mHelpOptionNames.insert( altName );

   return optionConfig;
}

ARGUMENTUM_INLINE GroupConfig argument_parser::add_group( const std::string& name )
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

ARGUMENTUM_INLINE GroupConfig argument_parser::add_exclusive_group( const std::string& name )
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

ARGUMENTUM_INLINE void argument_parser::end_group()
{
   mpActiveGroup = nullptr;
}

ARGUMENTUM_INLINE ParseResult argument_parser::parse_args( int argc, char** argv, int skip_args )
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

ARGUMENTUM_INLINE ParseResult argument_parser::parse_args(
      const std::vector<std::string>& args, int skip_args )
{
   auto ibegin = std::begin( args );
   if ( skip_args > 0 )
      ibegin += std::min<size_t>( skip_args, args.size() );

   return parse_args( ibegin, std::end( args ) );
}

ARGUMENTUM_INLINE ParseResult argument_parser::parse_args(
      std::vector<std::string>::const_iterator ibegin,
      std::vector<std::string>::const_iterator iend )
{
   if ( ibegin == iend ) {
      verifyDefinedOptions();
      if ( hasRequiredArguments() ) {
         generate_help();
         ParseResultBuilder result;
         result.signalHelpShown();
         result.requestExit();
         return std::move( result.getResult() );
      }
   }

   auto argStream = IteratorArgumentStream( ibegin, iend );
   return parse_args( argStream );
}

ARGUMENTUM_INLINE ParseResult argument_parser::parse_args( ArgumentStream& args )
{
   verifyDefinedOptions();
   resetOptionValues();

   ParseResultBuilder result;
   Parser parser( mParserDef, result );
   parser.parse( args );
   if ( result.wasExitRequested() )
      return std::move( result.getResult() );

   assignDefaultValues();
   validateParsedOptions( result );

   if ( mTopLevel && result.hasArgumentProblems() ) {
      result.signalErrorsShown();
      auto res = std::move( result.getResult() );
      describe_errors( res );
      return std::move( res );
   }

   return std::move( result.getResult() );
}

ARGUMENTUM_INLINE ArgumentHelpResult argument_parser::describe_argument(
      std::string_view name ) const
{
   ArgumentDescriber describer;
   return describer.describe_argument( mParserDef, name );
}

ARGUMENTUM_INLINE std::vector<ArgumentHelpResult> argument_parser::describe_arguments() const
{
   ArgumentDescriber describer;
   return describer.describe_arguments( mParserDef );
}

ARGUMENTUM_INLINE void argument_parser::resetOptionValues()
{
   for ( auto& pOption : mParserDef.mOptions )
      pOption->resetValue();

   for ( auto& pOption : mParserDef.mPositional )
      pOption->resetValue();
}

ARGUMENTUM_INLINE void argument_parser::assignDefaultValues()
{
   for ( auto& pOption : mParserDef.mOptions )
      if ( !pOption->wasAssigned() && pOption->hasDefault() )
         pOption->assignDefault();

   for ( auto& pOption : mParserDef.mPositional )
      if ( !pOption->wasAssigned() && pOption->hasDefault() )
         pOption->assignDefault();
}

ARGUMENTUM_INLINE void argument_parser::verifyDefinedOptions()
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

ARGUMENTUM_INLINE void argument_parser::validateParsedOptions( ParseResultBuilder& result )
{
   reportMissingOptions( result );
   reportExclusiveViolations( result );
   reportMissingGroups( result );
}

ARGUMENTUM_INLINE void argument_parser::reportMissingOptions( ParseResultBuilder& result )
{
   for ( auto& pOption : mParserDef.mOptions )
      if ( pOption->isRequired() && !pOption->wasAssigned() )
         result.addError( pOption->getHelpName(), MISSING_OPTION );

   for ( auto& pOption : mParserDef.mPositional )
      if ( pOption->needsMoreArguments() )
         result.addError( pOption->getHelpName(), MISSING_ARGUMENT );
}

ARGUMENTUM_INLINE bool argument_parser::hasRequiredArguments() const
{
   for ( auto& pOption : mParserDef.mOptions )
      if ( pOption->isRequired() )
         return true;

   for ( auto& pOption : mParserDef.mPositional )
      if ( pOption->isRequired() )
         return true;

   return false;
}

ARGUMENTUM_INLINE void argument_parser::reportExclusiveViolations( ParseResultBuilder& result )
{
   std::map<std::string, std::vector<std::string>> counts;
   for ( auto& pOption : mParserDef.mOptions ) {
      auto pGroup = pOption->getGroup();
      if ( pGroup && pGroup->isExclusive() && pOption->wasAssignedThroughThisOption() )
         counts[pGroup->getName()].push_back( pOption->getHelpName() );
   }

   for ( auto& c : counts )
      if ( c.second.size() > 1 )
         result.addError( c.second.front(), EXCLUSIVE_OPTION );
}

ARGUMENTUM_INLINE void argument_parser::reportMissingGroups( ParseResultBuilder& result )
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

ARGUMENTUM_INLINE OptionConfig argument_parser::tryAddArgument(
      Option& newOption, std::vector<std::string_view> names )
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

ARGUMENTUM_INLINE OptionConfig argument_parser::addPositional(
      Option&& newOption, const std::vector<std::string_view>& names )
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

ARGUMENTUM_INLINE OptionConfig argument_parser::addOption(
      Option&& newOption, const std::vector<std::string_view>& names )
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

ARGUMENTUM_INLINE void argument_parser::trySetNames(
      Option& option, const std::vector<std::string_view>& names ) const
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

ARGUMENTUM_INLINE void argument_parser::ensureIsNewOption( const std::string& name )
{
   if ( name.empty() )
      return;

   auto pOption = mParserDef.findOption( name );
   if ( pOption ) {
      auto groupName = pOption->getGroup() ? pOption->getGroup()->getName() : "";
      throw DuplicateOption( groupName, name );
   }
}

ARGUMENTUM_INLINE CommandConfig argument_parser::tryAddCommand( Command& command )
{
   if ( command.getName().empty() )
      throw std::invalid_argument( "A command must have a name." );
   if ( !command.hasOptions() && !command.hasFactory() )
      throw std::invalid_argument( "A command must have an options factory." );
   if ( command.getName()[0] == '-' )
      throw std::invalid_argument( "Command name must not start with a dash." );

   ensureIsNewCommand( command.getName() );

   auto pCommand = std::make_shared<Command>( std::move( command ) );
   mParserDef.mCommands.push_back( pCommand );
   return { pCommand };
}

ARGUMENTUM_INLINE void argument_parser::ensureIsNewCommand( const std::string& name )
{
   auto pCommand = mParserDef.findCommand( name );
   if ( pCommand )
      throw DuplicateCommand( name );
}

ARGUMENTUM_INLINE std::shared_ptr<OptionGroup> argument_parser::addGroup(
      std::string name, bool isExclusive )
{
   if ( name.empty() )
      throw std::invalid_argument( "A group must have a name." );

   std::transform( name.begin(), name.end(), name.begin(), tolower );
   assert( mGroups.count( name ) == 0 );

   auto pGroup = std::make_shared<OptionGroup>( name, isExclusive );
   mGroups[name] = pGroup;
   return pGroup;
}

ARGUMENTUM_INLINE std::shared_ptr<OptionGroup> argument_parser::findGroup( std::string name ) const
{
   std::transform( name.begin(), name.end(), name.begin(), tolower );
   auto igrp = mGroups.find( name );
   if ( igrp == mGroups.end() )
      return {};
   return igrp->second;
}

ARGUMENTUM_INLINE void argument_parser::generate_help()
{
   // TODO: The formatter should be configurable
   auto formatter = HelpFormatter();
   auto pStream = mParserDef.getConfig().pOutStream;
   if ( !pStream )
      pStream = &std::cout;

   formatter.format( mParserDef, *pStream );
}

ARGUMENTUM_INLINE void argument_parser::describe_errors( ParseResult& result )
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

}   // namespace argumentum
