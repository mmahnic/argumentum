// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <memory>
#include <string>
#include <string_view>

namespace argparse {

// NOTE: An option group with the same name can be defined in multiple
// places.  When it is configured multiple times the last configured values
// will be used, except for setRequired().
class OptionGroup
{
private:
   std::string mName;
   std::string mTitle;
   std::string mDescription;
   bool mIsRequired = false;
   bool mIsExclusive = false;

public:
   OptionGroup( std::string_view name, bool isExclusive )
      : mName( name )
      , mTitle( name )
      , mIsExclusive( isExclusive )
   {}

   void setTitle( std::string_view title )
   {
      mTitle = title;
   }

   void setDescription( std::string_view description )
   {
      mDescription = description;
   }

   // The required option can be set only when the group is not yet required.
   // Because a group can be defined in multiple places, it is required as
   // soon as it is required in one place.
   void setRequired( bool isRequired )
   {
      if ( !mIsRequired )
         mIsRequired = isRequired;
   }

   const std::string& getName() const
   {
      return mName;
   }

   const std::string& getTitle() const
   {
      return mTitle;
   }

   const std::string& getDescription() const
   {
      return mDescription;
   }

   const bool isExclusive() const
   {
      return mIsExclusive;
   }

   bool isRequired() const
   {
      return mIsRequired;
   }
};

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
