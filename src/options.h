// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "exceptions.h"
#include "groups.h"
#include "values.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace argparse {

class Option
{
private:
   std::unique_ptr<Value> mpValue;
   AssignAction mAssignAction;
   AssignDefaultAction mAssignDefaultAction;
   std::string mShortName;
   std::string mLongName;
   std::string mMetavar;
   std::string mHelp;
   std::string mFlagValue = "1";
   std::vector<std::string> mChoices;
   std::shared_ptr<OptionGroup> mpGroup;
   int mMinArgs = 0;
   int mMaxArgs = 0;
   bool mIsRequired = false;
   bool mIsVectorValue = false;

public:
   template<typename TValue>
   Option( TValue& value )
   {
      if constexpr ( std::is_base_of<Value, TValue>::value ) {
         mpValue = std::make_unique<TValue>( value );
      }
      else {
         using wrap_type = ConvertedValue<TValue>;
         mpValue = std::make_unique<wrap_type>( value );
      }
   }

   template<typename TValue>
   Option( std::vector<TValue>& value )
   {
      using val_vector = std::vector<TValue>;
      if constexpr ( std::is_base_of<Value, TValue>::value ) {
         mpValue = std::make_unique<val_vector>( value );
      }
      else {
         using wrap_type = ConvertedValue<val_vector>;
         mpValue = std::make_unique<wrap_type>( value );
      }

      mIsVectorValue = true;
   }

   void setShortName( std::string_view name )
   {
      mShortName = name;
   }

   void setLongName( std::string_view name )
   {
      mLongName = name;
   }

   void setMetavar( std::string_view varname )
   {
      mMetavar = varname;
   }

   void setHelp( std::string_view help )
   {
      mHelp = help;
   }

   void setNArgs( int count )
   {
      mMinArgs = std::max( 0, count );
      mMaxArgs = mMinArgs;
   }

   void setMinArgs( int count )
   {
      mMinArgs = std::max( 0, count );
      mMaxArgs = -1;
   }

   void setMaxArgs( int count )
   {
      mMinArgs = 0;
      mMaxArgs = std::max( 0, count );
   }

   void setRequired( bool isRequired = true )
   {
      mIsRequired = isRequired;
   }

   void setFlagValue( std::string_view value )
   {
      mFlagValue = value;
   }

   void setChoices( const std::vector<std::string>& choices )
   {
      mChoices = choices;
   }

   void setAction( AssignAction action )
   {
      mAssignAction = action;
   }

   void setAssignDefaultAction( AssignDefaultAction action )
   {
      mAssignDefaultAction = action;
   }

   void setGroup( const std::shared_ptr<OptionGroup>& pGroup )
   {
      mpGroup = pGroup;
   }

   bool isRequired() const
   {
      return mIsRequired;
   }

   const std::string& getName() const
   {
      return mLongName.empty() ? mShortName : mLongName;
   }

   const std::string& getShortName() const
   {
      return mShortName;
   }

   const std::string& getLongName() const
   {
      return mLongName;
   }

   std::string getHelpName() const
   {
      auto is_positional = mShortName.substr( 0, 1 ) != "-" && mLongName.substr( 0, 1 ) != "-";
      if ( is_positional ) {
         const auto& name =
               !mMetavar.empty() ? mMetavar : !mLongName.empty() ? mLongName : mShortName;
         return !name.empty() ? name : "ARG";
      }
      return !mLongName.empty() ? mLongName : mShortName;
   }

   bool hasName( std::string_view name ) const
   {
      return name == mShortName || name == mLongName;
   }

   const std::string& getRawHelp() const
   {
      return mHelp;
   }

   std::string getMetavar() const
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

   void setValue( const std::string& value, Environment& env )
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

   void assignDefault()
   {
      if ( mAssignDefaultAction )
         mpValue->setDefault( mAssignDefaultAction );
   }

   bool hasDefault() const
   {
      return mAssignDefaultAction != nullptr;
   }

   void resetValue()
   {
      mpValue->reset();
   }

   void onOptionStarted()
   {
      mpValue->onOptionStarted();
   }

   bool acceptsAnyArguments() const
   {
      return mMinArgs > 0 || mMaxArgs != 0;
   }

   bool willAcceptArgument() const
   {
      return mMaxArgs < 0 || mpValue->getOptionAssignCount() < mMaxArgs;
   }

   bool needsMoreArguments() const
   {
      return mpValue->getOptionAssignCount() < mMinArgs;
   }

   bool hasVectorValue() const
   {
      return mIsVectorValue;
   }

   /**
       * @returns true if the value was assigned through any option that shares
       * this option's value.
       */
   bool wasAssigned() const
   {
      return mpValue->getAssignCount() > 0;
   }

   bool wasAssignedThroughThisOption() const
   {
      return mpValue->getOptionAssignCount() > 0;
   }

   const std::string& getFlagValue() const
   {
      return mFlagValue;
   }

   std::tuple<int, int> getArgumentCounts() const
   {
      return std::make_tuple( mMinArgs, mMaxArgs );
   }

   std::shared_ptr<OptionGroup> getGroup() const
   {
      return mpGroup;
   }
};

}   // namespace argparse
