// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "value.h"

#include <cassert>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace argumentum {

class OptionGroup;

class Option
{
   friend class OptionFactory;

public:
   enum Kind { singleValue, vectorValue };

private:
   std::shared_ptr<Value> mpValue;
   AssignAction mAssignAction;
   AssignDefaultAction mAssignDefaultAction;
   std::string mShortName;
   std::string mLongName;
   std::vector<std::string> mMetavar;
   std::string mHelp;
   std::string mFlagValue = "1";
   std::vector<std::string> mChoices;
   std::shared_ptr<OptionGroup> mpGroup;
   int mMinArgs = 0;
   int mMaxArgs = 0;
   bool mIsRequired = false;
   bool mIsVectorValue = false;

   // The parameter of this option is forwarded.  The parameter is defined after a comma.
   bool mIsForwarded = false;

   // The number of asignments through the option that is currently active in
   // the parser.
   int mCurrentAssignCount = 0;

   // The total number of assignments through this option.
   int mTotalAssignCount = 0;

public:
   void setShortName( std::string_view name );
   void setLongName( std::string_view name );
   void setMetavar( const std::vector<std::string_view>& varnames );
   void setHelp( std::string_view help );
   void setNArgs( int count );
   void setMinArgs( int count );
   void setMaxArgs( int count );
   void setRequired( bool isRequired = true );
   void setFlagValue( std::string_view value );
   void setChoices( const std::vector<std::string>& choices );
   void setAction( AssignAction action );
   void setAssignDefaultAction( AssignDefaultAction action );
   void setGroup( const std::shared_ptr<OptionGroup>& pGroup );
   void setForwarded( bool isForwarded = true );
   bool isRequired() const;
   bool isPositional() const;
   bool isShortNumeric() const;
   const std::string& getName() const;
   const std::string& getShortName() const;
   const std::string& getLongName() const;
   std::string getHelpName() const;
   bool hasName( std::string_view name ) const;
   const std::string& getRawHelp() const;
   std::vector<std::string> getMetavar() const;
   void setValue( std::string_view value, Environment& env );

   /**
    * Called when an option was started but no values followed.
    */
   void autoSetMissingValue( Environment& env );
   void assignDefault();
   bool hasDefault() const;
   void resetValue();
   void onOptionStarted();
   bool acceptsAnyArguments() const;
   bool willAcceptArgument() const;
   bool needsMoreArguments() const;
   bool hasVectorValue() const;
   bool isForwarded() const;

   /**
    * @returns true if the value was assigned through any option that shares
    * this option's value.
    */
   bool wasAssigned() const;

   bool wasAssignedThroughThisOption() const;
   const std::string& getFlagValue() const;
   std::tuple<int, int> getArgumentCounts() const;
   std::shared_ptr<OptionGroup> getGroup() const;

   ValueId getValueId() const;
   TargetId getTargetId() const;

private:
   Option( std::shared_ptr<Value>&& pValue, Kind kind )
      : mpValue( std::move( pValue ) )
      , mIsVectorValue( kind == Option::vectorValue )
      , mMinArgs(  kind == Option::vectorValue  ? 1 : 0 )
      , mMaxArgs(  kind == Option::vectorValue  ? -1 : 0 )
   {
      // TODO: add getValue() that checks if it is nullptr and throws; returns *mpValue.
      assert( mpValue != nullptr );
   }
};

}   // namespace argumentum
