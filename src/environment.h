// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <string_view>

namespace argparse {

class Option;
class ParseResultBuilder;

class Environment
{
   Option& mOption;
   ParseResultBuilder& mResult;

public:
   Environment( Option& option, ParseResultBuilder& result );
   void exit_parser();
   std::string get_option_name() const;
   void add_error( std::string_view error );
   void notify_help_was_shown();
};

}   // namespace argparse
