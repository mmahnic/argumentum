// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "exceptions.h"

#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace argparse {

class argument_parser;
class ParseResult;

class Options
{
public:
   virtual void add_arguments( argument_parser& parser ) = 0;
};

class CommandOptions : public Options
{
   std::string mName;

public:
   CommandOptions( std::string_view name )
      : mName( name )
   {}

   const std::string& getName() const
   {
      return mName;
   }

   virtual void execute( const ParseResult& result )
   {}
};

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
   Command( std::string_view name, options_factory_t factory )
      : mName( name )
      , mFactory( factory )
   {}

   void setHelp( std::string_view help )
   {
      mHelp = help;
   }

   const std::string& getName() const
   {
      return mName;
   }

   bool hasName( std::string_view name ) const
   {
      return name == mName;
   }

   bool hasFactory() const
   {
      return mFactory != nullptr;
   }

   const std::string& getHelp() const
   {
      return mHelp;
   }

   std::shared_ptr<CommandOptions> getOptions()
   {
      if ( !mpOptions ) {
         if ( !mFactory )
            throw MissingCommandOptions( mName );

         mpOptions = mFactory( mName );
         if ( !mpOptions )
            throw MissingCommandOptions( mName );
      }
      return mpOptions;
   }
};

}   // namespace argparse
