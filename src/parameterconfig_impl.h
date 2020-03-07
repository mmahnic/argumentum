// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "parameterconfig.h"

#include "argparser.h"
#include "command.h"
#include "exceptions.h"
#include "group.h"

namespace argumentum {

ARGUMENTUM_INLINE ParameterConfig::ParameterConfig( argument_parser& parser )
   : mParser( parser )
   , mParserDef( parser.mParserDef )
{}

ARGUMENTUM_INLINE OptionFactory& ParameterConfig::getOptionFactory()
{
   // TODO (mmahnic): We need a single instance or the factory during argument
   // definition.  Afterwards, we do not need it any more.
   return mParser.getOptionFactory();
}

ARGUMENTUM_INLINE CommandConfig ParameterConfig::add_command(
      std::shared_ptr<CommandOptions> pOptions )
{
   if ( !pOptions )
      throw MissingCommandOptions( "<unknown>" );

   auto command = Command( pOptions->getName(), pOptions );
   return tryAddCommand( command );
}

ARGUMENTUM_INLINE void ParameterConfig::add_parameters( std::shared_ptr<Options> pOptions )
{
   if ( pOptions )
      // TODO (mmahnic): use add_parameters; the argument should be *this
      // instead of mParser
      pOptions->add_arguments( mParser );
}

ARGUMENTUM_INLINE VoidOptionConfig ParameterConfig::add_default_help_option()
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

ARGUMENTUM_INLINE VoidOptionConfig ParameterConfig::add_help_option(
      const std::string& name, const std::string& altName )
{
   if ( !name.empty() && name[0] != '-' || !altName.empty() && altName[0] != '-' )
      throw std::invalid_argument( "A help argument must be an option." );

   auto value = VoidValue{};
   auto option = getOptionFactory().createOption( value );
   auto optionConfig =   // (clf)
         VoidOptionConfig( tryAddParameter( option, { name, altName } ) )
               .help( "Display this help message and exit." )
               .action( []( const std::string& optionName, Environment& env ) {
                  auto pFormatter = env.get_help_formatter( optionName );
                  auto pStream = env.get_output_stream();
                  const auto& parserDef = env.get_parser_def();

                  pFormatter->format( parserDef, *pStream );
                  env.notify_help_was_shown();
                  env.exit_parser();
               } );

   if ( !name.empty() )
      mParserDef.mHelpOptionNames.insert( name );

   if ( !altName.empty() )
      mParserDef.mHelpOptionNames.insert( altName );

   return optionConfig;
}

ARGUMENTUM_INLINE GroupConfig ParameterConfig::add_group( const std::string& name )
{
   auto pGroup = mParserDef.findGroup( name );
   if ( pGroup ) {
      if ( pGroup->isExclusive() )
         throw MixingGroupTypes( name );
      mParserDef.mpActiveGroup = pGroup;
   }
   else
      mParserDef.mpActiveGroup = addGroup( name, false );

   return GroupConfig( mParserDef.mpActiveGroup );
}

ARGUMENTUM_INLINE GroupConfig ParameterConfig::add_exclusive_group( const std::string& name )
{
   auto pGroup = mParserDef.findGroup( name );
   if ( pGroup ) {
      if ( !pGroup->isExclusive() )
         throw MixingGroupTypes( name );
      mParserDef.mpActiveGroup = pGroup;
   }
   else
      mParserDef.mpActiveGroup = addGroup( name, true );

   return GroupConfig( mParserDef.mpActiveGroup );
}

ARGUMENTUM_INLINE void ParameterConfig::end_group()
{
   mParserDef.mpActiveGroup = nullptr;
}

ARGUMENTUM_INLINE OptionConfig ParameterConfig::tryAddParameter(
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

ARGUMENTUM_INLINE OptionConfig ParameterConfig::addPositional(
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
   if ( mParserDef.mpActiveGroup && !mParserDef.mpActiveGroup->isExclusive() )
      option.setGroup( mParserDef.mpActiveGroup );

   mParserDef.mPositional.push_back( pOption );
   return { pOption };
}

ARGUMENTUM_INLINE OptionConfig ParameterConfig::addOption(
      Option&& newOption, const std::vector<std::string_view>& names )
{
   trySetNames( newOption, names );
   ensureIsNewOption( newOption.getLongName() );
   ensureIsNewOption( newOption.getShortName() );

   auto pOption = std::make_shared<Option>( std::move( newOption ) );

   if ( mParserDef.mpActiveGroup )
      pOption->setGroup( mParserDef.mpActiveGroup );

   mParserDef.mOptions.push_back( pOption );
   return { pOption };
}

ARGUMENTUM_INLINE void ParameterConfig::trySetNames(
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

ARGUMENTUM_INLINE void ParameterConfig::ensureIsNewOption( const std::string& name )
{
   if ( name.empty() )
      return;

   auto pOption = mParserDef.findOption( name );
   if ( pOption ) {
      auto groupName = pOption->getGroup() ? pOption->getGroup()->getName() : "";
      throw DuplicateOption( groupName, name );
   }
}

ARGUMENTUM_INLINE CommandConfig ParameterConfig::tryAddCommand( Command& command )
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

ARGUMENTUM_INLINE void ParameterConfig::ensureIsNewCommand( const std::string& name )
{
   auto pCommand = mParserDef.findCommand( name );
   if ( pCommand )
      throw DuplicateCommand( name );
}

ARGUMENTUM_INLINE std::shared_ptr<OptionGroup> ParameterConfig::addGroup(
      std::string name, bool isExclusive )
{
   if ( name.empty() )
      throw std::invalid_argument( "A group must have a name." );

   std::transform( name.begin(), name.end(), name.begin(), tolower );
   assert( mParserDef.mGroups.count( name ) == 0 );

   auto pGroup = std::make_shared<OptionGroup>( name, isExclusive );
   mParserDef.mGroups[name] = pGroup;
   return pGroup;
}

}   // namespace argumentum
