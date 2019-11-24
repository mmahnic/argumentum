// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "exceptions.h"

namespace argparse {

inline CommandOptions::CommandOptions( std::string_view name )
   : mName( name )
{}

inline const std::string& CommandOptions::getName() const
{
   return mName;
}

inline void CommandOptions::execute( const ParseResult& result )
{}

inline Command::Command( std::string_view name, options_factory_t factory )
   : mName( name )
   , mFactory( factory )
{}

inline void Command::setHelp( std::string_view help )
{
   mHelp = help;
}

inline const std::string& Command::getName() const
{
   return mName;
}

inline bool Command::hasName( std::string_view name ) const
{
   return name == mName;
}

inline bool Command::hasFactory() const
{
   return mFactory != nullptr;
}

inline const std::string& Command::getHelp() const
{
   return mHelp;
}

inline std::shared_ptr<CommandOptions> Command::getOptions()
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

}   // namespace argparse
