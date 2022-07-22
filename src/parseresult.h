// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "command.h"

#ifndef ARGUMENTUM_BUILD_MODULE
#include <string>
#include <string_view>
#include <vector>
#endif

namespace argumentum {

// Errors known by the parser
enum EError {
   // The option is not known by the argument parser.
   UNKNOWN_OPTION,
   // Multiple options from an exclusive group are present.
   EXCLUSIVE_OPTION,
   // A required option is missing.
   MISSING_OPTION,
   // An option from a required (exclusive) group is missing.
   MISSING_OPTION_GROUP,
   // An required argument is missing.
   MISSING_ARGUMENT,
   // The input argument could not be converted.
   CONVERSION_ERROR,
   // The argument value is not in the set of valid argument values.
   INVALID_CHOICE,
   // Flags do not accept parameters.
   FLAG_PARAMETER,
   // Signal that exit was requested by an action.
   EXIT_REQUESTED,
   // An error signalled by an action.
   ACTION_ERROR,
   // The parser received invalid argv input.
   INVALID_ARGV,
   // The argument stream include depth was exceeded.
   INCLUDE_TOO_DEEP
};

struct ParseError
{
   const std::string option;
   const int errorCode;
   ParseError( std::string_view optionName, int code );
   ParseError( const ParseError& ) = default;
   ParseError( ParseError&& ) = default;
   ParseError& operator=( const ParseError& ) = default;
   ParseError& operator=( ParseError&& ) = default;

   void describeError( std::ostream& stream ) const;
};

class ParseResult
{
   friend class ParseResultBuilder;

private:
   // The parse result must be checked. If it is not, the destructor will throw
   // if no exception is currently being handled.
   struct RequireCheck
   {
      bool required = false;

      RequireCheck() = default;
      RequireCheck( RequireCheck&& other );
      RequireCheck( bool require );
      ~RequireCheck() noexcept( false );

      RequireCheck& operator=( RequireCheck&& other );
      void activate();
      void clear();
   };

private:
   bool exitRequested = false;
   bool helpWasShown = false;
   bool errorsWereShown = false;
   mutable RequireCheck mustCheck;

public:
   std::vector<std::string> ignoredArguments;
   std::vector<ParseError> errors;
   std::vector<std::shared_ptr<CommandOptions>> commands;

public:
   ParseResult() = default;
   ParseResult( ParseResult&& ) = default;
   ParseResult& operator=( ParseResult&& ) = default;

   ~ParseResult() noexcept( false );
   bool has_exited() const;
   bool help_was_shown() const;
   bool errors_were_shown() const;
   operator bool() const;

   std::shared_ptr<CommandOptions> findCommand( std::string_view name );

private:
   void clear();
};

class ParseResultBuilder
{
   ParseResult mResult;

public:
   void clear();
   bool wasExitRequested() const;
   void addError( std::string_view optionName, int error );
   void addIgnored( std::string_view arg );
   void addCommand( const std::shared_ptr<CommandOptions>& pCommand );
   void requestExit();
   void signalHelpShown();
   void signalErrorsShown();
   ParseResult&& getResult();
   bool hasArgumentProblems() const;
   void addResult( ParseResult&& result );
};

}   // namespace argumentum
