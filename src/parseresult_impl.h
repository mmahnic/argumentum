// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "exceptions.h"
#include "notifier.h"
#include "optionpack.h"

namespace argparse {

ARGUMENTUM_INLINE ParseError::ParseError( std::string_view optionName, int code )
   : option( optionName )
   , errorCode( code )
{}

ARGUMENTUM_INLINE void ParseError::describeError( std::ostream& stream ) const
{
   switch ( errorCode ) {
      case UNKNOWN_OPTION:
         stream << "Error: Unknown option: '" << option << "'\n";
         break;
      case EXCLUSIVE_OPTION:
         stream << "Error: Only one option from an exclusive group can be set. '" << option
                << "'\n";
         break;
      case MISSING_OPTION:
         stream << "Error: A required option is missing: '" << option << "'\n";
         break;
      case MISSING_OPTION_GROUP:
         stream << "Error: A required option from a group is missing: '" << option << "'\n";
         break;
      case MISSING_ARGUMENT:
         stream << "Error: An argument is missing: '" << option << "'\n";
         break;
      case CONVERSION_ERROR:
         stream << "Error: The argument could not be converted: '" << option << "'\n";
         break;
      case INVALID_CHOICE:
         stream << "Error: The value is not in the list of valid values: '" << option << "'\n";
         break;
      case FLAG_PARAMETER:
         stream << "Error: Flag options do not accep parameters: '" << option << "'\n";
         break;
      case EXIT_REQUESTED:
         break;
      case ACTION_ERROR:
         stream << "Error: " << option << "\n";
         break;
      case INVALID_ARGV:
         stream << "Error: Parser input is invalid.\n";
         break;
      case INCLUDE_TOO_DEEP:
         stream << "Include depth exceeded: '" << option << "'\n";
         break;
   }
}

ARGUMENTUM_INLINE ParseResult::RequireCheck::RequireCheck( RequireCheck&& other )
{
   required = other.required;
   other.clear();
}

ARGUMENTUM_INLINE auto ParseResult::RequireCheck::operator=( RequireCheck&& other ) -> RequireCheck&
{
   required = other.required;
   other.clear();
   return *this;
}

ARGUMENTUM_INLINE ParseResult::RequireCheck::RequireCheck( bool require )
   : required( require )
{}

ARGUMENTUM_INLINE ParseResult::RequireCheck::~RequireCheck() noexcept( false )
{
   if ( required ) {
      if ( !std::current_exception() )
         throw UncheckedParseResult();   // lgtm [cpp/throw-in-destructor]
      else
         Notifier::warn( "Unchecked Parse Result." );
   }
}

ARGUMENTUM_INLINE void ParseResult::RequireCheck::activate()
{
   required = true;
}

ARGUMENTUM_INLINE void ParseResult::RequireCheck::clear()
{
   required = false;
}

ARGUMENTUM_INLINE ParseResult::~ParseResult() noexcept( false )
{}

ARGUMENTUM_INLINE bool ParseResult::has_exited() const
{
   return exitRequested;
}

ARGUMENTUM_INLINE bool ParseResult::help_was_shown() const
{
   return helpWasShown;
}

ARGUMENTUM_INLINE bool ParseResult::errors_were_shown() const
{
   return errorsWereShown;
}

ARGUMENTUM_INLINE ParseResult::operator bool() const
{
   mustCheck.clear();
   return errors.empty() && ignoredArguments.empty() && !exitRequested;
}

ARGUMENTUM_INLINE void ParseResult::clear()
{
   ignoredArguments.clear();
   errors.clear();
   mustCheck.clear();
   exitRequested = false;
}

ARGUMENTUM_INLINE std::shared_ptr<CommandOptions> ParseResult::findCommand( std::string_view name )
{
   for ( auto& pCmd : commands ) {
      if ( pCmd && pCmd->getName() == name )
         return pCmd;
   }
   return nullptr;
}

ARGUMENTUM_INLINE void ParseResultBuilder::clear()
{
   mResult.clear();
}

ARGUMENTUM_INLINE bool ParseResultBuilder::wasExitRequested() const
{
   return mResult.exitRequested;
}

ARGUMENTUM_INLINE void ParseResultBuilder::addError( std::string_view optionName, int error )
{
   mResult.errors.emplace_back( optionName, error );
   mResult.mustCheck.activate();
}

ARGUMENTUM_INLINE void ParseResultBuilder::addIgnored( std::string_view arg )
{
   mResult.ignoredArguments.emplace_back( arg );
}

ARGUMENTUM_INLINE void ParseResultBuilder::addCommand(
      const std::shared_ptr<CommandOptions>& pCommand )
{
   mResult.commands.push_back( pCommand );
}

ARGUMENTUM_INLINE void ParseResultBuilder::requestExit()
{
   mResult.exitRequested = true;
   addError( {}, EXIT_REQUESTED );
}

ARGUMENTUM_INLINE void ParseResultBuilder::signalHelpShown()
{
   mResult.helpWasShown = true;
}

ARGUMENTUM_INLINE void ParseResultBuilder::signalErrorsShown()
{
   mResult.errorsWereShown = true;
}

ARGUMENTUM_INLINE ParseResult&& ParseResultBuilder::getResult()
{
   return std::move( mResult );
}

ARGUMENTUM_INLINE bool ParseResultBuilder::hasArgumentProblems() const
{
   return !mResult.errors.empty() || !mResult.ignoredArguments.empty();
}

ARGUMENTUM_INLINE void ParseResultBuilder::addResult( ParseResult&& result )
{
   mResult.exitRequested |= result.exitRequested;
   mResult.helpWasShown |= result.helpWasShown;
   mResult.errorsWereShown |= result.errorsWereShown;

   mResult.mustCheck.required |= result.mustCheck.required;
   result.mustCheck.required = false;

   for ( auto&& error : result.errors )
      mResult.errors.push_back( std::move( error ) );

   for ( auto&& arg : result.ignoredArguments )
      mResult.ignoredArguments.push_back( std::move( arg ) );
}

}   // namespace argparse
