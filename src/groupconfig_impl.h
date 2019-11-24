// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "groupconfig.h"

#include "groups.h"

namespace argparse {

CPPARGPARSE_INLINE GroupConfig::GroupConfig( std::shared_ptr<OptionGroup> pGroup )
   : mpGroup( pGroup )
{}

CPPARGPARSE_INLINE GroupConfig& GroupConfig::title( std::string_view title )
{
   mpGroup->setTitle( title );
   return *this;
}

CPPARGPARSE_INLINE GroupConfig& GroupConfig::description( std::string_view description )
{
   mpGroup->setDescription( description );
   return *this;
}

CPPARGPARSE_INLINE GroupConfig& GroupConfig::required( bool isRequired )
{
   mpGroup->setRequired( isRequired );
   return *this;
}

}   // namespace argparse
