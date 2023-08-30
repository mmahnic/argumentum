// Copyright (c) 2018-2021 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "option.h"

#include "exceptions.h"
#include "group.h"

#include <cstdarg>

namespace argumentum {

ARGUMENTUM_INLINE void Option::setShortName( std::string_view name )
{
   mShortName = name;
}

ARGUMENTUM_INLINE void Option::setLongName( std::string_view name )
{
   mLongName = name;
}

ARGUMENTUM_INLINE void Option::setMetavar( const std::vector<std::string_view>& varnames )
{
   auto cleanVarName( []( std::string_view v ) -> std::string {
      size_t b = 0;
      size_t e = v.size();
      while ( b < e && ( std::isspace( v[b] ) || v[b] == '-' ) )
         ++b;
      while ( b < e && std::isspace( v[e - 1] ) )
         --e;

      if ( b >= e )
         return {};

      std::string res;
      res.reserve( e - b );
      std::transform( v.begin() + b, v.begin() + e, std::back_inserter( res ), []( char c ) {
         return std::isspace( c ) ? '_' : c;
      } );
      return res;
   } );

   mMetavar.clear();
   for ( const auto& v : varnames ) {
      auto cv = cleanVarName( v );
      if ( !cv.empty() )
         mMetavar.push_back( std::move( cv ) );
   }
}

ARGUMENTUM_INLINE void Option::setHelp( std::string_view help )
{
   mHelp = help;
}

ARGUMENTUM_INLINE void Option::setNArgs( int count )
{
   mMinArgs = std::max( 0, count );
   mMaxArgs = mMinArgs;
}

ARGUMENTUM_INLINE void Option::setMinArgs( int count )
{
   mMinArgs = std::max( 0, count );
   mMaxArgs = -1;
}

ARGUMENTUM_INLINE void Option::setMaxArgs( int count )
{
   mMinArgs = 0;
   mMaxArgs = std::max( 0, count );
}

ARGUMENTUM_INLINE void Option::setRequired( bool isRequired )
{
   mIsRequired = isRequired;
}

ARGUMENTUM_INLINE void Option::setFlagValue( std::string_view value )
{
   mFlagValue = value;
}

ARGUMENTUM_INLINE void Option::setChoices( const std::vector<std::string>& choices )
{
   mChoices = choices;
}

ARGUMENTUM_INLINE void Option::setAction( AssignAction action )
{
   mAssignAction = action;
}

ARGUMENTUM_INLINE void Option::setAssignDefaultAction( AssignDefaultAction action )
{
   mAssignDefaultAction = action;
}

ARGUMENTUM_INLINE void Option::setGroup( const std::shared_ptr<OptionGroup>& pGroup )
{
   mpGroup = pGroup;
}

ARGUMENTUM_INLINE void Option::setForwarded( bool isForwarded )
{
   mIsForwarded = isForwarded;
}

ARGUMENTUM_INLINE bool Option::isForwarded() const
{
   return mIsForwarded;
}

ARGUMENTUM_INLINE bool Option::isRequired() const
{
   return mIsRequired;
}

ARGUMENTUM_INLINE bool Option::isPositional() const
{
   return mShortName.substr( 0, 1 ) != "-" && mLongName.substr( 0, 1 ) != "-";
}

ARGUMENTUM_INLINE bool Option::isShortNumeric() const
{
   return mShortName.size() == 2 && mShortName[0] == '-' && isdigit( mShortName[1] );
}

ARGUMENTUM_INLINE const std::string& Option::getName() const
{
   return mLongName.empty() ? mShortName : mLongName;
}

ARGUMENTUM_INLINE const std::string& Option::getShortName() const
{
   return mShortName;
}

ARGUMENTUM_INLINE const std::string& Option::getLongName() const
{
   return mLongName;
}

ARGUMENTUM_INLINE std::string Option::getHelpName() const
{
   if ( isPositional() ) {
      // TODO: mMetavar should no longer be used as helpName since we do not
      // know which name to choose.  Maybe help name should be set separately (.helpname).
      const auto& name =
            !mMetavar.empty() ? mMetavar[0] : ( !mLongName.empty() ? mLongName : mShortName );
      return !name.empty() ? name : "ARG";
   }
   return !mLongName.empty() ? mLongName : mShortName;
}

ARGUMENTUM_INLINE bool Option::hasName( std::string_view name ) const
{
   return name == mShortName || name == mLongName;
}

ARGUMENTUM_INLINE const std::string& Option::getRawHelp() const
{
   return mHelp;
}

ARGUMENTUM_INLINE std::vector<std::string> Option::getMetavar() const
{
   if ( !mMetavar.empty() )
      return mMetavar;

   auto& name = getName();
   auto pos = name.find_first_not_of( "-" );
   auto metavar = name.substr( pos );
   auto isPositional = pos == 0;
   std::transform( metavar.begin(), metavar.end(), metavar.begin(), [&]( char ch ) {
      return char( isPositional ? tolower( ch ) : toupper( ch ) );
   } );
   return { metavar };
}

ARGUMENTUM_INLINE void Option::setValue( std::string_view value, Environment& env )
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

ARGUMENTUM_INLINE void Option::autoSetMissingValue( Environment& env )
{
   ++mCurrentAssignCount;
   ++mTotalAssignCount;

   mpValue->setMissingValue( getFlagValue(), env );
}

ARGUMENTUM_INLINE void Option::assignDefault()
{
   if ( mAssignDefaultAction )
      mpValue->setDefault( mAssignDefaultAction );
}

ARGUMENTUM_INLINE bool Option::hasDefault() const
{
   return mAssignDefaultAction != nullptr;
}

ARGUMENTUM_INLINE void Option::resetValue()
{
   mCurrentAssignCount = 0;
   mTotalAssignCount = 0;
   mpValue->reset();
}

ARGUMENTUM_INLINE void Option::onOptionStarted()
{
   mCurrentAssignCount = 0;
   mpValue->onOptionStarted();
}

ARGUMENTUM_INLINE bool Option::acceptsAnyArguments() const
{
   return mMinArgs > 0 || mMaxArgs != 0;
}

ARGUMENTUM_INLINE bool Option::willAcceptArgument() const
{
   return mMaxArgs < 0 || mCurrentAssignCount < mMaxArgs;
}

ARGUMENTUM_INLINE bool Option::needsMoreArguments() const
{
   return mCurrentAssignCount < mMinArgs;
}

ARGUMENTUM_INLINE bool Option::hasVectorValue() const
{
   return mIsVectorValue;
}

ARGUMENTUM_INLINE bool Option::wasAssigned() const
{
   return mpValue->getAssignCount() > 0;
}

ARGUMENTUM_INLINE bool Option::wasAssignedThroughThisOption() const
{
   return mTotalAssignCount > 0;
}

ARGUMENTUM_INLINE const std::string& Option::getFlagValue() const
{
   return mFlagValue;
}

ARGUMENTUM_INLINE std::tuple<int, int> Option::getArgumentCounts() const
{
   return std::make_tuple( mMinArgs, mMaxArgs );
}

ARGUMENTUM_INLINE std::shared_ptr<OptionGroup> Option::getGroup() const
{
   return mpGroup;
}

ARGUMENTUM_INLINE ValueId Option::getValueId() const
{
   if ( mpValue )
      return mpValue->getValueId();

   return {};
}

ARGUMENTUM_INLINE TargetId Option::getTargetId() const
{
   if ( mpValue )
      return mpValue->getTargetId();

   return {};
}

}   // namespace argumentum
