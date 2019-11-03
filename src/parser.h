// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "commands.h"
#include "options.h"

#include <string>
#include <string_view>
#include <vector>

namespace argparse {

class Command;
class Option;
class ParseResultBuilder;

struct ParserDefinition
{
   std::vector<Command> mCommands;
   std::vector<Option> mOptions;
   std::vector<Option> mPositional;

   Option* findOption( std::string_view optionName )
   {
      for ( auto& option : mOptions )
         if ( option.hasName( optionName ) )
            return &option;

      return nullptr;
   }

   Command* findCommand( std::string_view commandName )
   {
      for ( auto& command : mCommands )
         if ( command.hasName( commandName ) )
            return &command;

      return nullptr;
   }
};

class Parser
{
   ParserDefinition& mParserDef;
   ParseResultBuilder& mResult;

   bool mIgnoreOptions = false;
   int mPosition = 0;
   // The active option will receive additional argument(s)
   Option* mpActiveOption = nullptr;

public:
   Parser( ParserDefinition& argParser, ParseResultBuilder& result );
   void parse( std::vector<std::string>::const_iterator ibegin,
         std::vector<std::string>::const_iterator iend );

private:
   void startOption( std::string_view name );
   bool haveActiveOption() const;
   void closeOption();
   void addFreeArgument( const std::string& arg );
   void addError( std::string_view optionName, int errorCode );
   void setValue( Option& option, const std::string& value );
   void parseCommandArguments( Command& command, std::vector<std::string>::const_iterator ibegin,
         std::vector<std::string>::const_iterator iend, ParseResultBuilder& result );
};

}   // namespace argparse
