// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <memory>
#include <string_view>

namespace argparse {

class OptionGroup;

class GroupConfig
{
   std::shared_ptr<OptionGroup> mpGroup;

public:
   GroupConfig( std::shared_ptr<OptionGroup> pGroup );

   // Set the title of the group that will be displayed in the generated help.
   GroupConfig& title( std::string_view title );

   // Set the description of the group that will be displayed in the generated
   // help.
   GroupConfig& description( std::string_view description );

   // Set to true if at least one option from the group must be present in the
   // input arguments.
   GroupConfig& required( bool isRequired = true );
};

}   // namespace argparse
