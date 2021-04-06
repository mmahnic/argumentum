// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "optionconfig.h"

#include <cassert>

namespace argumentum {

ARGUMENTUM_INLINE OptionConfig::OptionConfig( const std::shared_ptr<Option>& pOption )
   : mpOption( pOption )
{
   assert( pOption );
   if ( !mpOption )
      throw std::invalid_argument( "OptionConfig requires an option." );
}

ARGUMENTUM_INLINE Option& OptionConfig::getOption() const
{
   return *mpOption;
}

ARGUMENTUM_INLINE void OptionConfig::markCountWasSet()
{
   mCountWasSet = true;
}

ARGUMENTUM_INLINE void OptionConfig::ensureCountWasNotSet() const
{
   if ( mCountWasSet )
      throw std::invalid_argument( "Only one of nargs, minargs and maxargs can be used." );
}

ARGUMENTUM_INLINE void OptionConfig::ensureCanBeForwarded() const
{
   if ( !getOption().getShortName().empty() )
      throw std::invalid_argument( "Only long options can be used for forwarding parameters." );
}

ARGUMENTUM_INLINE VoidOptionConfig::VoidOptionConfig( OptionConfig&& wrapped )
   : OptionConfigBaseT<VoidOptionConfig>( std::move( wrapped ) )
{}

ARGUMENTUM_INLINE VoidOptionConfig& VoidOptionConfig::action( assign_action_env_t action )
{
   if ( action ) {
      auto wrapAction = [=]( Value& value, const std::string& argument,
                              Environment& env ) -> std::optional<std::string> {
         auto pVoid = VoidValue::value_cast( value );
         if ( pVoid )
            action( argument, env );
         return {};
      };
      OptionConfig::getOption().setAction( wrapAction );
   }
   else
      OptionConfig::getOption().setAction( nullptr );

   return *this;
}

}   // namespace argumentum
