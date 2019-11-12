// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "argparser.h"
#include "commands.h"
#include "options.h"
#include "parser.h"
#include "parseresult.h"

namespace argparse {

inline Parser::Parser( ParserDefinition& parserDef, ParseResultBuilder& result )
   : mParserDef( parserDef )
   , mResult( result )
{}

inline void Parser::parse( std::vector<std::string>::const_iterator ibegin,
      std::vector<std::string>::const_iterator iend )
{
   mResult.clear();
   for ( auto iarg = ibegin; iarg != iend; ++iarg ) {
      if ( *iarg == "--" ) {
         mIgnoreOptions = true;
         continue;
      }

      if ( mIgnoreOptions ) {
         addFreeArgument( *iarg );
         continue;
      }

      auto arg_view = std::string_view( *iarg );
      if ( arg_view.substr( 0, 2 ) == "--" )
         startOption( *iarg );
      else if ( arg_view.substr( 0, 1 ) == "-" ) {
         if ( iarg->size() == 2 )
            startOption( *iarg );
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
               setValue( option, *iarg );
               if ( !option.willAcceptArgument() )
                  closeOption();
            }
         }
         else {
            auto pCommand = mParserDef.findCommand( *iarg );
            if ( pCommand ) {
               parseCommandArguments( *pCommand, iarg, iend, mResult );
               break;
            }
            else
               addFreeArgument( *iarg );
         }
      }

      if ( mResult.wasExitRequested() )
         break;
   }

   if ( haveActiveOption() )
      closeOption();
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

inline void Parser::addFreeArgument( const std::string& arg )
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

inline void Parser::setValue( Option& option, const std::string& value )
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
inline void Parser::parseCommandArguments( Command& command,
      std::vector<std::string>::const_iterator ibegin,
      std::vector<std::string>::const_iterator iend, ParseResultBuilder& result )
{
   auto parser = argument_parser{};
   parser.add_arguments( command.createOptions() );
   parser.parse_args( ibegin, iend, result );
}

}   // namespace argparse
