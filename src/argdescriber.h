// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "commands.h"
#include "helpformatter.h"
#include "options.h"
#include "parser.h"

#include <string>

namespace argparse {

class ArgumentDescriber
{
public:
   ArgumentHelpResult describe_argument(
         const ParserDefinition& parserDef, std::string_view name ) const
   {
      bool isPositional = name.substr( 0, 1 ) != "-";
      const auto& args = isPositional ? parserDef.mPositional : parserDef.mOptions;
      for ( auto& opt : args )
         if ( opt.hasName( name ) )
            return describeOption( opt );

      throw std::invalid_argument( "Unknown option." );
   }

   std::vector<ArgumentHelpResult> describe_arguments( const ParserDefinition& parserDef ) const
   {
      std::vector<ArgumentHelpResult> descriptions;

      for ( auto& opt : parserDef.mOptions )
         descriptions.push_back( describeOption( opt ) );

      for ( auto& opt : parserDef.mPositional )
         descriptions.push_back( describeOption( opt ) );

      for ( auto& cmd : parserDef.mCommands )
         descriptions.push_back( describeCommand( cmd ) );

      return descriptions;
   }

   ArgumentHelpResult describeOption( const Option& option ) const
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

   ArgumentHelpResult describeCommand( const Command& command ) const
   {
      ArgumentHelpResult help;
      help.isCommand = true;
      help.help_name = command.getName();
      help.long_name = command.getName();
      help.help = command.getHelp();

      return help;
   }

private:
   std::string describeArguments( const Option& option, const std::string& metavar ) const
   {
      std::string res;
      auto [mmin, mmax] = option.getArgumentCounts();
      if ( mmin > 0 ) {
         res = metavar;
         for ( int i = 1; i < mmin; ++i )
            res = res + " " + metavar;
      }
      if ( mmax < mmin ) {
         auto opt = ( res.empty() ? "[" : " [" ) + metavar + " ...]";
         res += opt;
      }
      else if ( mmax - mmin == 1 )
         res += "[" + metavar + "]";
      else if ( mmax > mmin ) {
         auto opt = ( res.empty() ? "[" : " [" ) + metavar + " {0.." + std::to_string( mmax - mmin )
               + "}]";
         res += opt;
      }

      return res;
   }
};

}   // namespace argparse
