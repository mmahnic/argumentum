// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "groups.h"

namespace argparse {

class GroupConfig
{
   std::shared_ptr<OptionGroup> mpGroup;

public:
   GroupConfig( std::shared_ptr<OptionGroup> pGroup )
      : mpGroup( pGroup )
   {}

   GroupConfig& title( std::string_view title )
   {
      mpGroup->setTitle( title );
      return *this;
   }

   GroupConfig& description( std::string_view description )
   {
      mpGroup->setDescription( description );
      return *this;
   }

   GroupConfig& required( bool isRequired = true )
   {
      mpGroup->setRequired( isRequired );
      return *this;
   }
};

}   // namespace argparse
