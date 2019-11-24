// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "options.h"

#include "exceptions.h"
#include "groups.h"

namespace argparse {

inline void Option::setShortName( std::string_view name )
{
   mShortName = name;
}

inline void Option::setLongName( std::string_view name )
{
   mLongName = name;
}

inline void Option::setMetavar( std::string_view varname )
{
   mMetavar = varname;
}

inline void Option::setHelp( std::string_view help )
{
   mHelp = help;
}

inline void Option::setNArgs( int count )
{
   mMinArgs = std::max( 0, count );
   mMaxArgs = mMinArgs;
}

inline void Option::setMinArgs( int count )
{
   mMinArgs = std::max( 0, count );
   mMaxArgs = -1;
}

inline void Option::setMaxArgs( int count )
{
   mMinArgs = 0;
   mMaxArgs = std::max( 0, count );
}

inline void Option::setRequired( bool isRequired )
{
   mIsRequired = isRequired;
}

inline void Option::setFlagValue( std::string_view value )
{
   mFlagValue = value;
}

inline void Option::setChoices( const std::vector<std::string>& choices )
{
   mChoices = choices;
}

inline void Option::setAction( AssignAction action )
{
   mAssignAction = action;
}

inline void Option::setAssignDefaultAction( AssignDefaultAction action )
{
   mAssignDefaultAction = action;
}

inline void Option::setGroup( const std::shared_ptr<OptionGroup>& pGroup )
{
   mpGroup = pGroup;
}

inline bool Option::isRequired() const
{
   return mIsRequired;
}

inline const std::string& Option::getName() const
{
   return mLongName.empty() ? mShortName : mLongName;
}

inline const std::string& Option::getShortName() const
{
   return mShortName;
}

inline const std::string& Option::getLongName() const
{
   return mLongName;
}

inline std::string Option::getHelpName() const
{
   auto is_positional = mShortName.substr( 0, 1 ) != "-" && mLongName.substr( 0, 1 ) != "-";
   if ( is_positional ) {
      const auto& name = !mMetavar.empty() ? mMetavar : !mLongName.empty() ? mLongName : mShortName;
      return !name.empty() ? name : "ARG";
   }
   return !mLongName.empty() ? mLongName : mShortName;
}

inline bool Option::hasName( std::string_view name ) const
{
   return name == mShortName || name == mLongName;
}

inline const std::string& Option::getRawHelp() const
{
   return mHelp;
}

inline std::string Option::getMetavar() const
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

inline void Option::setValue( std::string_view value, Environment& env )
{
   if ( !mChoices.empty() && std::none_of( mChoices.begin(), mChoices.end(), [&value]( auto v ) {
           return v == value;
        } ) ) {
      mpValue->markBadArgument();
      throw InvalidChoiceError( value );
   }

   // If mAssignAction is not set, mpValue->setValue will try to use a
   // default action.
   mpValue->setValue( value, mAssignAction, env );
}

inline void Option::assignDefault()
{
   if ( mAssignDefaultAction )
      mpValue->setDefault( mAssignDefaultAction );
}

inline bool Option::hasDefault() const
{
   return mAssignDefaultAction != nullptr;
}

inline void Option::resetValue()
{
   mpValue->reset();
}

inline void Option::onOptionStarted()
{
   mpValue->onOptionStarted();
}

inline bool Option::acceptsAnyArguments() const
{
   return mMinArgs > 0 || mMaxArgs != 0;
}

inline bool Option::willAcceptArgument() const
{
   return mMaxArgs < 0 || mpValue->getOptionAssignCount() < mMaxArgs;
}

inline bool Option::needsMoreArguments() const
{
   return mpValue->getOptionAssignCount() < mMinArgs;
}

inline bool Option::hasVectorValue() const
{
   return mIsVectorValue;
}

inline bool Option::wasAssigned() const
{
   return mpValue->getAssignCount() > 0;
}

inline bool Option::wasAssignedThroughThisOption() const
{
   return mpValue->getOptionAssignCount() > 0;
}

inline const std::string& Option::getFlagValue() const
{
   return mFlagValue;
}

inline std::tuple<int, int> Option::getArgumentCounts() const
{
   return std::make_tuple( mMinArgs, mMaxArgs );
}

inline std::shared_ptr<OptionGroup> Option::getGroup() const
{
   return mpGroup;
}

}   // namespace argparse
