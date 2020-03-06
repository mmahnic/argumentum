// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "parserconfig.h"

#include <string_view>

namespace argumentum {

class Option;
class ParseResultBuilder;
class ParserDefinition;

class Environment
{
   const Option& mOption;
   ParseResultBuilder& mResult;
   const ParserDefinition& mParserDef;

public:
   Environment( Option& option, ParseResultBuilder& result, const ParserDefinition& parserDef );
   const ParserConfig::Data& get_config() const;
   const ParserDefinition& get_parser_def() const;
   std::unique_ptr<HelpFormatter> get_help_formatter( const std::string& optionName ) const;
   std::ostream* get_output_stream() const;
   void exit_parser();
   std::string get_option_name() const;
   void add_error( std::string_view error );
   void notify_help_was_shown();
};

}   // namespace argumentum
