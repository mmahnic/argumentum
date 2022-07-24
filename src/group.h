// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#ifndef ARGUMENTUM_BUILD_MODULE
#include "nomodule.h"

#include <memory>
#include <string>
#include <string_view>
#endif

namespace argumentum {

// NOTE: An option group with the same name can be defined in multiple
// places.  When it is configured multiple times the last configured values
// will be used, except for setRequired().
ARGUMENTUM_EXPORT
class OptionGroup
{
private:
   std::string mName;
   std::string mTitle;
   std::string mDescription;
   bool mIsRequired = false;
   bool mIsExclusive = false;

public:
   OptionGroup( std::string_view name, bool isExclusive );
   void setTitle( std::string_view title );
   void setDescription( std::string_view description );

   // The required option can be set only when the group is not yet required.
   // Because a group can be defined in multiple places, it is required as
   // soon as it is required in one place.
   void setRequired( bool isRequired );
   const std::string& getName() const;
   const std::string& getTitle() const;
   const std::string& getDescription() const;
   bool isExclusive() const;
   bool isRequired() const;
};

}   // namespace argumentum
