// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "argparser.h"
#include "argumentstream.h"
#include "command.h"
#include "option.h"
#include "parser.h"
#include "parseresult.h"

#include <regex>

namespace argumentum {

ARGUMENTUM_INLINE Parser::Parser( const ParserDefinition& parserDef, ParseResultBuilder& result )
   : mParserDef( parserDef )
   , mResult( result )
{}

ARGUMENTUM_INLINE void Parser::parse( ArgumentStream& argStream )
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

enum class EArgumentType {
   // A free argument is not an option or an option value.
   freeArgument,

   // Include the contents of a file as options.
   include,

   // Treat the rest of the arguments as free argumetns.
   endOfOptions,

   // An option with a long name, currently identified with '--' prefix.
   longOption,

   // An option with a sinble character name, currently identified with '-' prefix.
   shortOption,

   // Short options can be combined in a single argument prefixed with '-'.
   multiOption,

   // A value of an option that accepts one or more valuers.
   optionValue,

   // The name of a command.
   commandName
};

namespace {
ARGUMENTUM_INLINE bool isNumberLike( std::string_view arg )
{
   static auto rxNumber = std::regex(
         "^0b[01]+$"
         "|"
         "^0o[0-7]+$"
         "|"
         "^(0d)?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?$"
         "|"
         "^0x[0-9a-fA-F]*\\.?[0-9a-fA-F]+([pP][-+]?[0-9a-fA-F]+)?$" );
   return std::regex_match( std::begin( arg ), std::end( arg ), rxNumber );
}
}   // namespace

ARGUMENTUM_INLINE bool Parser::optionWithNameExists( std::string_view name )
{
   return mParserDef.findOption( name ) != nullptr;
}

ARGUMENTUM_INLINE EArgumentType Parser::getNextArgumentType( std::string_view arg )
{
   if ( mIgnoreOptions )
      return EArgumentType::freeArgument;

   if ( arg.substr( 0, 1 ) == "@" )
      return EArgumentType::include;

   if ( arg == "--" )
      return EArgumentType::endOfOptions;

   if ( arg.substr( 0, 2 ) == "--" )
      return EArgumentType::longOption;

   // TODO: negativeMode should be a global parser setting.
   //   - argparse mode: if a -N option exists, treat -M args as options
   //   - argumentum mode:
   //      - options that take arguments will accept -M as a value; to treat it
   //        as an option, change the order of arguments;
   //      - positionals will accept -M as a value if -M is not an option; to
   //        treat it as a value, put it after --.

   enum class ENegativeMode { argparse, argumentum };
   const auto negativeMode = ENegativeMode::argumentum;

   if ( arg.substr( 0, 1 ) == "-" ) {
      if ( isNumberLike( arg.substr( 1 ) ) ) {
         if ( negativeMode == ENegativeMode::argparse ) {
            if ( !mParserDef.hasNumericOptions() )
               return haveActiveOption() ? EArgumentType::optionValue : EArgumentType::freeArgument;
         }
         else {
            if ( haveActiveOption() ) {
               if ( mpActiveOption->willAcceptArgument() && !mpActiveOption->isPositional() )
                  return EArgumentType::optionValue;
            }
            else if ( !optionWithNameExists( arg.substr( 0, 2 ) ) )
               return EArgumentType::freeArgument;
         }
      }

      if ( arg.size() == 2 )
         return EArgumentType::shortOption;

      if ( arg.size() > 2 )
         return EArgumentType::multiOption;
   }

   if ( haveActiveOption() ) {
      if ( mpActiveOption->willAcceptArgument() )
         return EArgumentType::optionValue;
   }

   auto pCommand = mParserDef.findCommand( arg );
   if ( pCommand )
      return EArgumentType::commandName;

   return EArgumentType::freeArgument;
}

ARGUMENTUM_INLINE void Parser::parse( ArgumentStream& argStream, unsigned depth )
{
   for ( auto optArg = argStream.next(); !!optArg; optArg = argStream.next() ) {
      switch ( getNextArgumentType( *optArg ) ) {
         case EArgumentType::include:
            parseSubstream( optArg->substr( 1 ), depth );
            continue;

         case EArgumentType::endOfOptions:
            mIgnoreOptions = true;
            continue;

         case EArgumentType::freeArgument:
            addFreeArgument( *optArg );
            continue;

         case EArgumentType::longOption:
         case EArgumentType::shortOption:
            startOption( *optArg );
            break;

         case EArgumentType::multiOption: {
            auto arg_view = std::string_view( *optArg );
            auto opt = std::string{ "--" };
            for ( unsigned i = 1; i < arg_view.size(); ++i ) {
               opt[1] = arg_view[i];
               startOption( opt );
            }
            break;
         }

         case EArgumentType::optionValue:
            assert( mpActiveOption != nullptr );
            setValue( *mpActiveOption, *optArg );
            if ( !mpActiveOption->willAcceptArgument() )
               closeOption();
            break;

         case EArgumentType::commandName: {
            auto pCommand = mParserDef.findCommand( *optArg );
            if ( pCommand ) {
               parseCommandArguments( *pCommand, argStream, mResult );
               return;
            }
            break;
         }
      }

      if ( mResult.wasExitRequested() )
         return;
   }
}

ARGUMENTUM_INLINE void Parser::startOption( std::string_view optionStr )
{
   if ( haveActiveOption() )
      closeOption();

   std::string_view name;
   std::string_view arg;

   // A comma in the option may start a list of forwarded arguments.
   auto commapos = optionStr.find( "," );
   if ( commapos != std::string::npos ) {
      name = optionStr.substr( 0, commapos );
      arg = optionStr.substr( commapos + 1 );

      auto pOption = mParserDef.findOption( name );
      if ( pOption && pOption->isForwarded() && !arg.empty() ) {
         pOption->onOptionStarted();
         parseForwardedArguments( *pOption, arg );
         return;
      }
   }

   auto eqpos = optionStr.find( "=" );
   if ( eqpos != std::string::npos ) {
      name = optionStr.substr( 0, eqpos );
      arg = optionStr.substr( eqpos + 1 );
   }
   else
      name = optionStr;

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

ARGUMENTUM_INLINE void Parser::parseForwardedArguments( Option& option, std::string_view args )
{
   // Forwarded arguments are a comma delimited list.  Split it and add each
   // argument as an option value.

   auto addArg( [this, &option]( const std::string& str ) {
      if ( !str.empty() )
         setValue( option, str );
   } );

   std::stringstream ssarg;
   auto it = args.begin();

   // The parameter args is the part of the opition after the comma, so the
   // first comma of args is always escaped.
   if ( it != args.end() && *it == ',' ) {
      ssarg << ',';
      ++it;
   }

   for ( ; it != args.end(); ++it ) {
      if ( *it == ',' ) {
         auto inext = it + 1;
         if ( inext != args.end() && *inext == ',' ) {
            ssarg << ',';
            it = inext;
         }
         else {
            addArg( ssarg.str() );
            ssarg.str( "" );
         }
         continue;
      }

      ssarg << *it;
   }

   addArg( ssarg.str() );
}

ARGUMENTUM_INLINE bool Parser::haveActiveOption() const
{
   return mpActiveOption != nullptr;
}

ARGUMENTUM_INLINE void Parser::closeOption()
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

ARGUMENTUM_INLINE void Parser::addFreeArgument( std::string_view arg )
{
   while ( mPosition < mParserDef.mPositional.size() ) {
      auto& option = *mParserDef.mPositional[mPosition];
      if ( option.willAcceptArgument() ) {
         setValue( option, arg );
         return;
      }
      ++mPosition;
   }

   mResult.addIgnored( arg );
}

ARGUMENTUM_INLINE void Parser::addError( std::string_view optionName, int errorCode )
{
   mResult.addError( optionName, errorCode );
}

ARGUMENTUM_INLINE void Parser::setValue( Option& option, std::string_view value )
{
   try {
      auto env = Environment{ option, mResult, mParserDef };
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
ARGUMENTUM_INLINE void Parser::parseCommandArguments(
      Command& command, ArgumentStream& argStream, ParseResultBuilder& result )
{
   auto parser = argument_parser::createSubParser();
   auto commandpath = mParserDef.getConfig().program() + " " + command.getName();
   parser.config().program( commandpath ).description( command.getHelp() );

   auto pcout = mParserDef.getConfig().output_stream();
   assert( pcout );
   parser.config().cout( *pcout );

   auto pCmdOptions = command.getOptions();
   if ( pCmdOptions ) {
      parser.params().add_parameters( pCmdOptions );
      result.addCommand( pCmdOptions );
   }
   result.addResult( parser.parse_args( argStream ) );
}

ARGUMENTUM_INLINE void Parser::parseSubstream( std::string_view streamName, unsigned depth )
{
   if ( !mParserDef.getConfig().filesystem() )
      throw MissingFilesystem();

   if ( depth > mParserDef.getConfig().max_include_depth() )
      throw IncludeDepthExceeded( std::string{ streamName } );

   auto pFilesystem = mParserDef.getConfig().filesystem();
   assert( pFilesystem );

   auto pSubstream = pFilesystem->open( std::string{ streamName } );
   if ( pSubstream )
      parse( *pSubstream, depth + 1 );
}

}   // namespace argumentum
