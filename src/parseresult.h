// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "exceptions.h"
#include <string>
#include <string_view>
#include <vector>

namespace argparse {

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
   INVALID_ARGV
};

struct ParseError
{
   const std::string option;
   const int errorCode;
   ParseError( std::string_view optionName, int code )
      : option( optionName )
      , errorCode( code )
   {}
   ParseError( const ParseError& ) = default;
   ParseError( ParseError&& ) = default;
};

class ParseResult
{
   friend class ParseResultBuilder;

private:
   struct RequireCheck
   {
      bool required = false;

      RequireCheck() = default;
      RequireCheck( RequireCheck&& other )
      {
         required = other.required;
         other.clear();
      }
      RequireCheck& operator=( RequireCheck&& other )
      {
         required = other.required;
         other.clear();
         return *this;
      }
      RequireCheck( bool require )
         : required( require )
      {}

      ~RequireCheck() noexcept( false )
      {
         if ( !std::current_exception() ) {
            if ( required )
               throw UncheckedParseResult();
         }
      }

      void activate()
      {
         required = true;
      }

      void clear()
      {
         required = false;
      }
   };

private:
   bool exitRequested = false;
   bool helpWasShown = false;
   bool errorsWereShown = false;
   mutable RequireCheck mustCheck;

public:
   std::vector<std::string> ignoredArguments;
   std::vector<ParseError> errors;

public:
   ParseResult() = default;
   ParseResult( ParseResult&& ) = default;
   ParseResult& operator=( ParseResult&& ) = default;

   ~ParseResult() noexcept( false )
   {}

   bool has_exited() const
   {
      return exitRequested;
   }

   bool help_was_shown() const
   {
      return helpWasShown;
   }

   bool errors_were_shown() const
   {
      return errorsWereShown;
   }

   operator bool() const
   {
      mustCheck.clear();
      return errors.empty() && ignoredArguments.empty() && !exitRequested;
   }

private:
   void clear()
   {
      ignoredArguments.clear();
      errors.clear();
      mustCheck.clear();
      exitRequested = false;
   }
};

class ParseResultBuilder
{
   ParseResult result;

public:
   void clear()
   {
      result.clear();
   }

   bool wasExitRequested() const
   {
      return result.exitRequested;
   }

   void addError( std::string_view optionName, int error )
   {
      result.errors.emplace_back( optionName, error );
      result.mustCheck.activate();
   }

   void addIgnored( const std::string& arg )
   {
      result.ignoredArguments.push_back( arg );
   }

   void requestExit()
   {
      result.exitRequested = true;
      result.mustCheck.activate();
   }

   void signalHelpShown()
   {
      result.helpWasShown = true;
   }

   void signalErrorsShown()
   {
      result.errorsWereShown = true;
   }

   ParseResult&& getResult()
   {
      return std::move( result );
   }

   bool hasArgumentProblems() const
   {
      return !result.errors.empty() || !result.ignoredArguments.empty();
   }
};

}   // namespace argparse
