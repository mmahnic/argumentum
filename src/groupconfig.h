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

   // Set the title of the group that will be displayed in the generated help.
   GroupConfig& title( std::string_view title )
   {
      mpGroup->setTitle( title );
      return *this;
   }

   // Set the description of the group that will be displayed in the generated
   // help.
   GroupConfig& description( std::string_view description )
   {
      mpGroup->setDescription( description );
      return *this;
   }

   // Set to true if at least one option from the group must be present in the
   // input arguments.
   GroupConfig& required( bool isRequired = true )
   {
      mpGroup->setRequired( isRequired );
      return *this;
   }
};

}   // namespace argparse
