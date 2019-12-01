// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace argparse {

class CommandOptions;

// An internal definition of a command.
class Command
{
public:
   using options_factory_t =
         std::function<std::shared_ptr<CommandOptions>( std::string_view name )>;

private:
   std::string mName;
   std::shared_ptr<CommandOptions> mpOptions;
   options_factory_t mFactory;
   std::string mHelp;

public:
   Command( std::string_view name, options_factory_t factory );
   void setHelp( std::string_view help );
   const std::string& getName() const;
   bool hasName( std::string_view name ) const;
   bool hasFactory() const;
   const std::string& getHelp() const;
   std::shared_ptr<CommandOptions> getOptions();
};

}   // namespace argparse
