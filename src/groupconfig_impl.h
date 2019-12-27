// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "groupconfig.h"

#include "group.h"

namespace argumentum {

ARGUMENTUM_INLINE GroupConfig::GroupConfig( std::shared_ptr<OptionGroup> pGroup )
   : mpGroup( pGroup )
{}

ARGUMENTUM_INLINE GroupConfig& GroupConfig::title( std::string_view title )
{
   mpGroup->setTitle( title );
   return *this;
}

ARGUMENTUM_INLINE GroupConfig& GroupConfig::description( std::string_view description )
{
   mpGroup->setDescription( description );
   return *this;
}

ARGUMENTUM_INLINE GroupConfig& GroupConfig::required( bool isRequired )
{
   mpGroup->setRequired( isRequired );
   return *this;
}

}   // namespace argumentum
