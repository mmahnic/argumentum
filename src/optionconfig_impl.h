// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "optionconfig.h"

#include <cassert>

namespace argparse {

CPPARGPARSE_INLINE OptionConfig::OptionConfig( const std::shared_ptr<Option>& pOption )
   : mpOption( pOption )
{
   assert( pOption );
   if ( !mpOption )
      throw std::invalid_argument( "OptionConfig requires an option." );
}

CPPARGPARSE_INLINE OptionConfig& OptionConfig::action( AssignAction action )
{
   getOption().setAction( action );
   return *this;
}

CPPARGPARSE_INLINE Option& OptionConfig::getOption()
{
   return *mpOption;
}

CPPARGPARSE_INLINE void OptionConfig::markCountWasSet()
{
   mCountWasSet = true;
}

CPPARGPARSE_INLINE void OptionConfig::ensureCountWasNotSet() const
{
   if ( mCountWasSet )
      throw std::invalid_argument( "Only one of nargs, minargs and maxargs can be used." );
}

CPPARGPARSE_INLINE VoidOptionConfig::VoidOptionConfig( OptionConfig&& wrapped )
   : OptionConfigBaseT<VoidOptionConfig>( std::move( wrapped ) )
{}

CPPARGPARSE_INLINE VoidOptionConfig& VoidOptionConfig::action( assign_action_env_t action )
{
   if ( action ) {
      auto wrapAction = [=]( Value& target, const std::string& value,
                              Environment& env ) -> std::optional<std::string> {
         auto pv = dynamic_cast<VoidValue*>( &target );
         if ( pv )
            action( value, env );
         return {};
      };
      OptionConfig::getOption().setAction( wrapAction );
   }
   else
      OptionConfig::getOption().setAction( nullptr );

   return *this;
}

}   // namespace argparse
