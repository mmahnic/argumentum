// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "parserconfig.h"

#ifndef ARGUMENTUM_BUILD_MODULE
#include "nomodule.h"

#include <memory>
#include <string_view>
#endif

namespace argumentum {

class Option;
class ParseResultBuilder;
class ParserDefinition;

ARGUMENTUM_EXPORT
class Environment
{
   const Option& mOption;
   ParseResultBuilder& mResult;
   const ParserDefinition& mParserDef;

public:
   Environment( Option& option, ParseResultBuilder& result, const ParserDefinition& parserDef );
   const ParserConfig::Data& get_config() const;
   const ParserDefinition& get_parser_def() const;
   std::shared_ptr<IFormatHelp> get_help_formatter( const std::string& optionName ) const;
   std::ostream* get_output_stream() const;
   void exit_parser();
   std::string get_option_name() const;
   void add_error( std::string_view error );
   void notify_help_was_shown();
};

}   // namespace argumentum
