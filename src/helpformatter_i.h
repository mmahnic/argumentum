// Copyright (c) 2018, 2019 Marko Mahnič
// License: MIT. See LICENSE in the root of the project.

#pragma once

#include <string>
#include <iostream>

namespace argparse {

class ArgumentParser;

struct ArgumentHelpResult
{
   std::string short_name;
   std::string long_name;
   std::string help;

   bool is_positional() const
   {
      return short_name.substr( 0, 1 ) != "-" && long_name.substr( 0, 1 ) != "-";
   }
};


class HelpFormatter
{
public:
   void format( const ArgumentParser& parser, std::ostream& out );
};

}
