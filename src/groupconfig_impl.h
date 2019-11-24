// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "groupconfig.h"

#include "groups.h"

namespace argparse {

inline GroupConfig::GroupConfig( std::shared_ptr<OptionGroup> pGroup )
   : mpGroup( pGroup )
{}

inline GroupConfig& GroupConfig::title( std::string_view title )
{
   mpGroup->setTitle( title );
   return *this;
}

inline GroupConfig& GroupConfig::description( std::string_view description )
{
   mpGroup->setDescription( description );
   return *this;
}

inline GroupConfig& GroupConfig::required( bool isRequired )
{
   mpGroup->setRequired( isRequired );
   return *this;
}

}   // namespace argparse
