// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "argdescriber.h"

#include "command.h"
#include "group.h"
#include "option.h"
#include "parser.h"

#include <string>

namespace argumentum {

ARGUMENTUM_INLINE ArgumentHelpResult ArgumentDescriber::describe_argument(
      const ParserDefinition& parserDef, std::string_view name ) const
{
   bool isPositional = name.substr( 0, 1 ) != "-";
   const auto& args = isPositional ? parserDef.mPositional : parserDef.mOptions;
   for ( auto& pOpt : args )
      if ( pOpt->hasName( name ) )
         return describeOption( *pOpt );

   throw std::invalid_argument( "Unknown option." );
}

ARGUMENTUM_INLINE std::vector<ArgumentHelpResult> ArgumentDescriber::describe_arguments(
      const ParserDefinition& parserDef ) const
{
   std::vector<ArgumentHelpResult> descriptions;

   for ( auto& pOpt : parserDef.mOptions )
      descriptions.push_back( describeOption( *pOpt ) );

   for ( auto& pOpt : parserDef.mPositional )
      descriptions.push_back( describeOption( *pOpt ) );

   for ( auto& pCmd : parserDef.mCommands )
      descriptions.push_back( describeCommand( *pCmd ) );

   return descriptions;
}

ARGUMENTUM_INLINE ArgumentHelpResult ArgumentDescriber::describeOption( const Option& option ) const
{
   ArgumentHelpResult help;
   help.help_name = option.getHelpName();
   help.short_name = option.getShortName();
   help.long_name = option.getLongName();
   help.metavar = option.getMetavar();
   help.help = option.getRawHelp();
   help.isRequired = option.isRequired();

   if ( option.acceptsAnyArguments() )
      help.arguments = describeArguments( option, help.metavar );

   auto pGroup = option.getGroup();
   if ( pGroup ) {
      help.group.name = pGroup->getName();
      help.group.title = pGroup->getTitle();
      help.group.description = pGroup->getDescription();
      help.group.isExclusive = pGroup->isExclusive();
      help.group.isRequired = pGroup->isRequired();
   }

   return help;
}

ARGUMENTUM_INLINE ArgumentHelpResult ArgumentDescriber::describeCommand(
      const Command& command ) const
{
   ArgumentHelpResult help;
   help.isCommand = true;
   help.help_name = command.getName();
   help.long_name = command.getName();
   help.help = command.getHelp();

   return help;
}

ARGUMENTUM_INLINE std::string ArgumentDescriber::describeArguments(
      const Option& option, const std::vector<std::string>& metavars ) const
{
   std::string res;
   auto getMetavar( [&]( unsigned ivar ) {
      if ( metavars.empty() )
         return option.getHelpName();

      if ( ivar >= metavars.size() )
         return metavars.back();
      return metavars[ivar];
   } );

   unsigned closecount = 0;
   auto getOpenBracket( [&closecount]( const std::string& res ) {
      ++closecount;
      return res.empty() ? "[" : " [";
   } );

   unsigned ivar = 0;
   auto [mmin, mmax] = option.getArgumentCounts();
   if ( mmin < 0 )
      mmin = 0;

   if ( mmin > 0 ) {
      // Mandatory parameters
      res = getMetavar( 0 );
      for ( ivar = 1; ivar < unsigned( mmin ); ++ivar )
         res = res + " " + getMetavar( ivar );
   }

   if ( mmax < mmin ) {
      // Optional parameters, unlimited
      while ( ivar < metavars.size() - 1 ) {
         auto opt = getOpenBracket( res ) + getMetavar( ivar );
         res += opt;
         ++ivar;
      }
      auto opt = getOpenBracket( res ) + getMetavar( ivar ) + " ...";
      res += opt;
   }
   else {
      // Optional parameters, limited
      if ( mmax == 1 )
         res += getOpenBracket( res ) + getMetavar( ivar );
      else if ( mmax > mmin ) {
         auto limit = std::min<size_t>( mmax - 1, metavars.size() - 1 );
         while ( ivar < limit ) {
            auto opt = getOpenBracket( res ) + getMetavar( ivar );
            res += opt;
            ++ivar;
         }
         auto remaining = mmax - limit;
         if ( remaining == 1 )
            res += getOpenBracket( res ) + getMetavar( ivar );
         else {
            auto opt = getOpenBracket( res ) + getMetavar( ivar ) + " {0.."
                  + std::to_string( remaining ) + "}";
            res += opt;
         }
      }
   }

   if ( closecount > 0 )
      res += std::string( closecount, ']' );

   return res;
}

}   // namespace argumentum
