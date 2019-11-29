// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "options.h"

#include "exceptions.h"
#include "groups.h"

namespace argparse {

CPPARGPARSE_INLINE void Option::setShortName( std::string_view name )
{
   mShortName = name;
}

CPPARGPARSE_INLINE void Option::setLongName( std::string_view name )
{
   mLongName = name;
}

CPPARGPARSE_INLINE void Option::setMetavar( std::string_view varname )
{
   mMetavar = varname;
}

CPPARGPARSE_INLINE void Option::setHelp( std::string_view help )
{
   mHelp = help;
}

CPPARGPARSE_INLINE void Option::setNArgs( int count )
{
   mMinArgs = std::max( 0, count );
   mMaxArgs = mMinArgs;
}

CPPARGPARSE_INLINE void Option::setMinArgs( int count )
{
   mMinArgs = std::max( 0, count );
   mMaxArgs = -1;
}

CPPARGPARSE_INLINE void Option::setMaxArgs( int count )
{
   mMinArgs = 0;
   mMaxArgs = std::max( 0, count );
}

CPPARGPARSE_INLINE void Option::setRequired( bool isRequired )
{
   mIsRequired = isRequired;
}

CPPARGPARSE_INLINE void Option::setFlagValue( std::string_view value )
{
   mFlagValue = value;
}

CPPARGPARSE_INLINE void Option::setChoices( const std::vector<std::string>& choices )
{
   mChoices = choices;
}

CPPARGPARSE_INLINE void Option::setAction( AssignAction action )
{
   mAssignAction = action;
}

CPPARGPARSE_INLINE void Option::setAssignDefaultAction( AssignDefaultAction action )
{
   mAssignDefaultAction = action;
}

CPPARGPARSE_INLINE void Option::setGroup( const std::shared_ptr<OptionGroup>& pGroup )
{
   mpGroup = pGroup;
}

CPPARGPARSE_INLINE bool Option::isRequired() const
{
   return mIsRequired;
}

CPPARGPARSE_INLINE const std::string& Option::getName() const
{
   return mLongName.empty() ? mShortName : mLongName;
}

CPPARGPARSE_INLINE const std::string& Option::getShortName() const
{
   return mShortName;
}

CPPARGPARSE_INLINE const std::string& Option::getLongName() const
{
   return mLongName;
}

CPPARGPARSE_INLINE std::string Option::getHelpName() const
{
   auto is_positional = mShortName.substr( 0, 1 ) != "-" && mLongName.substr( 0, 1 ) != "-";
   if ( is_positional ) {
      const auto& name = !mMetavar.empty() ? mMetavar : !mLongName.empty() ? mLongName : mShortName;
      return !name.empty() ? name : "ARG";
   }
   return !mLongName.empty() ? mLongName : mShortName;
}

CPPARGPARSE_INLINE bool Option::hasName( std::string_view name ) const
{
   return name == mShortName || name == mLongName;
}

CPPARGPARSE_INLINE const std::string& Option::getRawHelp() const
{
   return mHelp;
}

CPPARGPARSE_INLINE std::string Option::getMetavar() const
{
   if ( !mMetavar.empty() )
      return mMetavar;

   auto& name = getName();
   auto pos = name.find_first_not_of( "-" );
   auto metavar = name.substr( pos );
   auto isPositional = pos == 0;
   std::transform(
         metavar.begin(), metavar.end(), metavar.begin(), isPositional ? tolower : toupper );
   return metavar;
}

CPPARGPARSE_INLINE void Option::setValue( std::string_view value, Environment& env )
{
   ++mCurrentAssignCount;
   ++mTotalAssignCount;

   if ( !mChoices.empty() && std::none_of( mChoices.begin(), mChoices.end(), [&value]( auto v ) {
           return v == value;
        } ) ) {
      mpValue->markBadArgument();
      throw InvalidChoiceError( value );
   }

   // If mAssignAction is not set, mpValue->setValue will try to use a default
   // action.
   mpValue->setValue( value, mAssignAction, env );
}

CPPARGPARSE_INLINE void Option::assignDefault()
{
   if ( mAssignDefaultAction )
      mpValue->setDefault( mAssignDefaultAction );
}

CPPARGPARSE_INLINE bool Option::hasDefault() const
{
   return mAssignDefaultAction != nullptr;
}

CPPARGPARSE_INLINE void Option::resetValue()
{
   mCurrentAssignCount = 0;
   mTotalAssignCount = 0;
   mpValue->reset();
}

CPPARGPARSE_INLINE void Option::onOptionStarted()
{
   mCurrentAssignCount = 0;
   mpValue->onOptionStarted();
}

CPPARGPARSE_INLINE bool Option::acceptsAnyArguments() const
{
   return mMinArgs > 0 || mMaxArgs != 0;
}

CPPARGPARSE_INLINE bool Option::willAcceptArgument() const
{
   return mMaxArgs < 0 || mCurrentAssignCount < mMaxArgs;
}

CPPARGPARSE_INLINE bool Option::needsMoreArguments() const
{
   return mCurrentAssignCount < mMinArgs;
}

CPPARGPARSE_INLINE bool Option::hasVectorValue() const
{
   return mIsVectorValue;
}

CPPARGPARSE_INLINE bool Option::wasAssigned() const
{
   return mpValue->getAssignCount() > 0;
}

CPPARGPARSE_INLINE bool Option::wasAssignedThroughThisOption() const
{
   return mTotalAssignCount > 0;
}

CPPARGPARSE_INLINE const std::string& Option::getFlagValue() const
{
   return mFlagValue;
}

CPPARGPARSE_INLINE std::tuple<int, int> Option::getArgumentCounts() const
{
   return std::make_tuple( mMinArgs, mMaxArgs );
}

CPPARGPARSE_INLINE std::shared_ptr<OptionGroup> Option::getGroup() const
{
   return mpGroup;
}

CPPARGPARSE_INLINE ValueId Option::getValueId() const
{
   if ( mpValue )
      return mpValue->getValueId();

   return {};
}

CPPARGPARSE_INLINE TargetId Option::getTargetId() const
{
   if ( mpValue )
      return mpValue->getTargetId();

   return {};
}

}   // namespace argparse
