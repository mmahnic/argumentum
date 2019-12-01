// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "exceptions.h"
#include "optionpack.h"

namespace argparse {

CPPARGPARSE_INLINE CommandOptions::CommandOptions( std::string_view name )
   : mName( name )
{}

CPPARGPARSE_INLINE const std::string& CommandOptions::getName() const
{
   return mName;
}

CPPARGPARSE_INLINE void CommandOptions::execute( const ParseResult& result )
{}

CPPARGPARSE_INLINE Command::Command( std::string_view name, options_factory_t factory )
   : mName( name )
   , mFactory( factory )
{}

CPPARGPARSE_INLINE void Command::setHelp( std::string_view help )
{
   mHelp = help;
}

CPPARGPARSE_INLINE const std::string& Command::getName() const
{
   return mName;
}

CPPARGPARSE_INLINE bool Command::hasName( std::string_view name ) const
{
   return name == mName;
}

CPPARGPARSE_INLINE bool Command::hasFactory() const
{
   return mFactory != nullptr;
}

CPPARGPARSE_INLINE const std::string& Command::getHelp() const
{
   return mHelp;
}

CPPARGPARSE_INLINE std::shared_ptr<CommandOptions> Command::getOptions()
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
