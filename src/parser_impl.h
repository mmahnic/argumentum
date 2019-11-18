// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "argparser.h"
#include "argumentstream.h"
#include "commands.h"
#include "options.h"
#include "parser.h"
#include "parseresult.h"

namespace argparse {

inline Parser::Parser( ParserDefinition& parserDef, ParseResultBuilder& result )
   : mParserDef( parserDef )
   , mResult( result )
{}

inline void Parser::parse( ArgumentStream& argStream )
{
   mResult.clear();

   try {
      parse( argStream, 0 );
   }
   catch ( const IncludeDepthExceeded& e ) {
      mResult.addError( e.what(), INCLUDE_TOO_DEEP );
   }

   if ( haveActiveOption() )
      closeOption();
}

inline void Parser::parse( ArgumentStream& argStream, unsigned depth )
{
   for ( auto optArg = argStream.next(); !!optArg; optArg = argStream.next() ) {
      if ( optArg->substr( 0, 1 ) == "@" ) {
         parseSubstream( optArg->substr( 1 ), depth );
         continue;
      }

      if ( *optArg == "--" ) {
         mIgnoreOptions = true;
         continue;
      }

      if ( mIgnoreOptions ) {
         addFreeArgument( *optArg );
         continue;
      }

      auto arg_view = std::string_view( *optArg );
      if ( arg_view.substr( 0, 2 ) == "--" )
         startOption( *optArg );
      else if ( arg_view.substr( 0, 1 ) == "-" ) {
         if ( optArg->size() == 2 )
            startOption( *optArg );
         else {
            auto opt = std::string{ "--" };
            for ( int i = 1; i < arg_view.size(); ++i ) {
               opt[1] = arg_view[i];
               startOption( opt );
            }
         }
      }
      else {
         if ( haveActiveOption() ) {
            auto& option = *mpActiveOption;
            if ( option.willAcceptArgument() ) {
               setValue( option, *optArg );
               if ( !option.willAcceptArgument() )
                  closeOption();
            }
         }
         else {
            auto pCommand = mParserDef.findCommand( *optArg );
            if ( pCommand ) {
               parseCommandArguments( *pCommand, argStream, mResult );
               break;
            }
            else
               addFreeArgument( *optArg );
         }
      }

      if ( mResult.wasExitRequested() )
         break;
   }
}

// Parse the argument stream to determine the help context.
inline std::vector<ParserDefinition> Parser::parse_for_help(
      ArgumentStream& argStream, const std::set<std::string>& helpOptionNames )
{
   std::vector<ParserDefinition> res;
   for ( auto optArg = argStream.next(); !!optArg; optArg = argStream.next() ) {
      if ( optArg->substr( 0, 1 ) == "@" )
         continue;

      if ( helpOptionNames.count( std::string{ *optArg } ) > 0 )
         // TODO: check if the next argument is a command and add its definition
         break;

      if ( optArg->substr( 0, 1 ) == "-" )
         continue;

      auto pCommand = mParserDef.findCommand( *optArg );
      if ( pCommand ) {
         auto parser = argument_parser{};
         auto pCmdOptions = pCommand->getOptions();
         if ( pCmdOptions ) {
            parser.add_arguments( pCmdOptions );
            parser.config().program( pCommand->getName() ).description( pCommand->getHelp() );
         }
         res.push_back( parser.mParserDef );

         ParseResultBuilder result;
         auto subparser = Parser( parser.mParserDef, result );
         auto childs = subparser.parse_for_help( argStream, helpOptionNames );

         for ( auto&& child : childs )
            res.emplace_back( std::move( child ) );

         break;
      }
   }
   return res;
}

inline void Parser::startOption( std::string_view name )
{
   if ( haveActiveOption() )
      closeOption();

   std::string_view arg;
   auto eqpos = name.find( "=" );
   if ( eqpos != std::string::npos ) {
      arg = name.substr( eqpos + 1 );
      name = name.substr( 0, eqpos );
   }

   auto pOption = mParserDef.findOption( name );
   if ( pOption ) {
      auto& option = *pOption;
      option.onOptionStarted();
      if ( option.willAcceptArgument() )
         mpActiveOption = pOption;
      else
         setValue( option, option.getFlagValue() );

      if ( !arg.empty() ) {
         if ( option.willAcceptArgument() )
            setValue( option, std::string{ arg } );
         else
            addError( pOption->getHelpName(), FLAG_PARAMETER );
      }
   }
   else
      addError( name, UNKNOWN_OPTION );
}

inline bool Parser::haveActiveOption() const
{
   return mpActiveOption != nullptr;
}

inline void Parser::closeOption()
{
   if ( haveActiveOption() ) {
      auto& option = *mpActiveOption;
      if ( option.needsMoreArguments() )
         addError( option.getHelpName(), MISSING_ARGUMENT );
      else if ( option.willAcceptArgument() && !option.wasAssignedThroughThisOption() )
         setValue( option, option.getFlagValue() );
   }
   mpActiveOption = nullptr;
}

inline void Parser::addFreeArgument( std::string_view arg )
{
   if ( mPosition < mParserDef.mPositional.size() ) {
      auto& option = *mParserDef.mPositional[mPosition];
      if ( option.willAcceptArgument() ) {
         setValue( option, arg );
         return;
      }
      else {
         ++mPosition;
         while ( mPosition < mParserDef.mPositional.size() ) {
            auto& option = *mParserDef.mPositional[mPosition];
            if ( option.willAcceptArgument() ) {
               setValue( option, arg );
               return;
            }
            ++mPosition;
         }
      }
   }

   mResult.addIgnored( arg );
}

inline void Parser::addError( std::string_view optionName, int errorCode )
{
   mResult.addError( optionName, errorCode );
}

inline void Parser::setValue( Option& option, std::string_view value )
{
   try {
      auto env = Environment{ option, mResult };
      option.setValue( value, env );
   }
   catch ( const InvalidChoiceError& ) {
      addError( option.getHelpName(), INVALID_CHOICE );
   }
   catch ( const std::invalid_argument& ) {
      addError( option.getHelpName(), CONVERSION_ERROR );
   }
   catch ( const std::out_of_range& ) {
      addError( option.getHelpName(), CONVERSION_ERROR );
   }
}

// A parser for command's (sub)options is instantiated only when a command is
// selected by an input argument.
inline void Parser::parseCommandArguments(
      Command& command, ArgumentStream& argStream, ParseResultBuilder& result )
{
   auto parser = argument_parser{};
   auto pCmdOptions = command.getOptions();
   if ( pCmdOptions ) {
      parser.add_arguments( pCmdOptions );
      result.addCommand( pCmdOptions );
   }
   result.addResult( parser.parse_args( argStream ) );
}

inline void Parser::parseSubstream( std::string_view streamName, unsigned depth )
{
   if ( !mParserDef.getConfig().pFilesystem )
      throw MissingFilesystem();

   if ( depth > mParserDef.getConfig().maxIncludeDepth )
      throw IncludeDepthExceeded( std::string{ streamName } );

   auto pSubstream = mParserDef.getConfig().pFilesystem->open( std::string{ streamName } );
   if ( pSubstream )
      parse( *pSubstream, depth + 1 );
}

}   // namespace argparse
