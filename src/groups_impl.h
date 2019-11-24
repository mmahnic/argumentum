// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "groups.h"

namespace argparse {

inline OptionGroup::OptionGroup( std::string_view name, bool isExclusive )
   : mName( name )
   , mTitle( name )
   , mIsExclusive( isExclusive )
{}

inline void OptionGroup::setTitle( std::string_view title )
{
   mTitle = title;
}

inline void OptionGroup::setDescription( std::string_view description )
{
   mDescription = description;
}

// The required option can be set only when the group is not yet required.
// Because a group can be defined in multiple places, it is required as
// soon as it is required in one place.
inline void OptionGroup::setRequired( bool isRequired )
{
   if ( !mIsRequired )
      mIsRequired = isRequired;
}

inline const std::string& OptionGroup::getName() const
{
   return mName;
}

inline const std::string& OptionGroup::getTitle() const
{
   return mTitle;
}

inline const std::string& OptionGroup::getDescription() const
{
   return mDescription;
}

inline bool OptionGroup::isExclusive() const
{
   return mIsExclusive;
}

inline bool OptionGroup::isRequired() const
{
   return mIsRequired;
}

}   // namespace argparse
