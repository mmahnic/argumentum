// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "environment.h"

#include "option.h"
#include "parseresult.h"

namespace argparse {

CPPARGPARSE_INLINE Environment::Environment( Option& option, ParseResultBuilder& result )
   : mOption( option )
   , mResult( result )
{}

CPPARGPARSE_INLINE void Environment::exit_parser()
{
   mResult.requestExit();
}

CPPARGPARSE_INLINE std::string Environment::get_option_name() const
{
   return mOption.getHelpName();
}

CPPARGPARSE_INLINE void Environment::add_error( std::string_view error )
{
   if ( error.empty() )
      mResult.addError( get_option_name(), ACTION_ERROR );
   else
      mResult.addError( get_option_name() + ": " + std::string( error ), ACTION_ERROR );
}

CPPARGPARSE_INLINE void Environment::notify_help_was_shown()
{
   mResult.signalHelpShown();
}

}   // namespace argparse
