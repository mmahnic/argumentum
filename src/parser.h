// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "parserconfig.h"
#include "parserdefinition.h"

#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace argumentum {

class Option;
class Command;
class ParseResultBuilder;
class ArgumentStream;
enum class EArgumentType;

class Parser
{
   const ParserDefinition& mParserDef;
   ParseResultBuilder& mResult;

   bool mIgnoreOptions = false;
   size_t mPosition = 0;
   // The active option will receive additional argument(s)
   Option* mpActiveOption = nullptr;

public:
   Parser( const ParserDefinition& argParser, ParseResultBuilder& result );
   void parse( ArgumentStream& argStream );

private:
   void startOption( std::string_view name );
   bool optionWithNameExists( std::string_view name );
   bool haveActiveOption() const;
   void closeOption();
   void addFreeArgument( std::string_view arg );
   void addError( std::string_view optionName, int errorCode );
   void setValue( Option& option, std::string_view value );

   void parse( ArgumentStream& argStream, unsigned depth );
   void parseCommandArguments(
         Command& command, ArgumentStream& argStream, ParseResultBuilder& result );
   void parseSubstream( std::string_view streamName, unsigned depth );
   EArgumentType getNextArgumentType( std::string_view arg );
};

}   // namespace argumentum
