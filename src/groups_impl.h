// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "groups.h"

namespace argparse {

CPPARGPARSE_INLINE OptionGroup::OptionGroup( std::string_view name, bool isExclusive )
   : mName( name )
   , mTitle( name )
   , mIsExclusive( isExclusive )
{}

CPPARGPARSE_INLINE void OptionGroup::setTitle( std::string_view title )
{
   mTitle = title;
}

CPPARGPARSE_INLINE void OptionGroup::setDescription( std::string_view description )
{
   mDescription = description;
}

// The required option can be set only when the group is not yet required.
// Because a group can be defined in multiple places, it is required as
// soon as it is required in one place.
CPPARGPARSE_INLINE void OptionGroup::setRequired( bool isRequired )
{
   if ( !mIsRequired )
      mIsRequired = isRequired;
}

CPPARGPARSE_INLINE const std::string& OptionGroup::getName() const
{
   return mName;
}

CPPARGPARSE_INLINE const std::string& OptionGroup::getTitle() const
{
   return mTitle;
}

CPPARGPARSE_INLINE const std::string& OptionGroup::getDescription() const
{
   return mDescription;
}

CPPARGPARSE_INLINE bool OptionGroup::isExclusive() const
{
   return mIsExclusive;
}

CPPARGPARSE_INLINE bool OptionGroup::isRequired() const
{
   return mIsRequired;
}

}   // namespace argparse
