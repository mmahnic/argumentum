// Copyright (c) 2018, 2019, 2020 Marko Mahnič
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
         ParseResultBuilder result;

         auto config = getConfig();
         auto pFormatter = config.help_formatter( "" );
         auto pStream = config.output_stream();
         assert( pFormatter && pStream );

         pFormatter->format( mParserDef, *pStream );
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
   if ( mParserDef.mHelpOptionNames.empty() ) {
      params().end_group();
      try {
         params().add_default_help_option();
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
      // A positional option must have enough arguments.
      if ( pOption->needsMoreArguments() )
         // If it is optional, it may have no arguments.
         if ( pOption->isRequired() || pOption->wasAssigned() )
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

ARGUMENTUM_INLINE void argument_parser::describe_errors( ParseResult& result )
{
   auto pStream = mParserDef.getConfig().output_stream();
   assert( pStream );

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
