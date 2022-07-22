// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#ifndef ARGUMENTUM_BUILD_MODULE
#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#endif

namespace argumentum {

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
   Command( std::string_view name, std::shared_ptr<CommandOptions> pOptions );
   void setHelp( std::string_view help );
   const std::string& getName() const;
   bool hasName( std::string_view name ) const;
   bool hasFactory() const;
   bool hasOptions() const;
   const std::string& getHelp() const;
   std::shared_ptr<CommandOptions> getOptions();
};

}   // namespace argumentum
