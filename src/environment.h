// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "options.h"
#include "parseresult.h"

namespace argparse {

class Environment
{
   Option& mOption;
   ParseResultBuilder& mResult;

public:
   Environment( Option& option, ParseResultBuilder& result )
      : mOption( option )
      , mResult( result )
   {}

   void exit_parser()
   {
      mResult.requestExit();
   }

   std::string get_option_name() const
   {
      return mOption.getHelpName();
   }

   void add_error( std::string_view error )
   {
      if ( error.empty() )
         mResult.addError( get_option_name(), ACTION_ERROR );
      else
         mResult.addError( get_option_name() + ": " + std::string( error ), ACTION_ERROR );
   }
};

}   // namespace argparse
