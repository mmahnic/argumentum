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

/**
 * OptionConfig is used to configure an option after an option was created with add_argument.
 */
class OptionConfig
{
   std::shared_ptr<Option> mpOption;
   bool mCountWasSet = false;

public:
   OptionConfig( const std::shared_ptr<Option>& pOption )
      : mpOption( pOption )
   {
      assert( pOption );
      if ( !mpOption )
         throw std::invalid_argument( "OptionConfig requires an option." );
   }

   OptionConfig& action( AssignAction action )
   {
      getOption().setAction( action );
      return *this;
   }

protected:
   Option& getOption()
   {
      return *mpOption;
   }

   void markCountWasSet()
   {
      mCountWasSet = true;
   }

   void ensureCountWasNotSet() const
   {
      if ( mCountWasSet )
         throw std::invalid_argument( "Only one of nargs, minargs and maxargs can be used." );
   }
};

template<typename TDerived>
class OptionConfigBaseT : public OptionConfig
{
public:
   using this_t = TDerived;

public:
   using OptionConfig::OptionConfig;

   OptionConfigBaseT( OptionConfig&& wrapped )
      : OptionConfig( std::move( wrapped ) )
   {}

   this_t& setShortName( std::string_view name )
   {
      getOption().setShortName( name );
      return *static_cast<this_t*>( this );
   }

   this_t& setLongName( std::string_view name )
   {
      getOption().setLongName( name );
      return *static_cast<this_t*>( this );
   }

   this_t& metavar( std::string_view varname )
   {
      getOption().setMetavar( varname );
      return *static_cast<this_t*>( this );
   }

   this_t& help( std::string_view help )
   {
      getOption().setHelp( help );
      return *static_cast<this_t*>( this );
   }

   this_t& nargs( int count )
   {
      ensureCountWasNotSet();
      getOption().setNArgs( count );
      markCountWasSet();
      return *static_cast<this_t*>( this );
   }

   this_t& minargs( int count )
   {
      ensureCountWasNotSet();
      getOption().setMinArgs( count );
      markCountWasSet();
      return *static_cast<this_t*>( this );
   }

   this_t& maxargs( int count )
   {
      ensureCountWasNotSet();
      getOption().setMaxArgs( count );
      markCountWasSet();
      return *static_cast<this_t*>( this );
   }

   this_t& required( bool isRequired = true )
   {
      getOption().setRequired( isRequired );
      return *static_cast<this_t*>( this );
   }

   this_t& flagValue( std::string_view value )
   {
      getOption().setFlagValue( value );
      return *static_cast<this_t*>( this );
   }

   this_t& choices( const std::vector<std::string>& choices )
   {
      getOption().setChoices( choices );
      return *static_cast<this_t*>( this );
   }
};

template<typename TValue>
class OptionConfigA : public OptionConfigBaseT<OptionConfigA<TValue>>
{
   using this_t = OptionConfigA<TValue>;
   using assign_action_t = std::function<void( TValue&, const std::string& )>;
   using assign_action_env_t = std::function<void( TValue&, const std::string&, Environment& )>;
   using assign_default_action_t = std::function<void( TValue& )>;

public:
   using OptionConfigBaseT<this_t>::OptionConfigBaseT;

   OptionConfigA( OptionConfig&& wrapped )
      : OptionConfigBaseT<this_t>( std::move( wrapped ) )
   {}

   this_t& action( assign_action_t action )
   {
      if ( action ) {
         auto wrapAction = [=]( Value& target, const std::string& value, Environment& ) {
            auto pConverted = dynamic_cast<ConvertedValue<TValue>*>( &target );
            if ( pConverted )
               action( pConverted->mValue, value );
         };
         OptionConfig::getOption().setAction( wrapAction );
      }
      else
         OptionConfig::getOption().setAction( nullptr );
      return *this;
   }

   this_t& action( assign_action_env_t action )
   {
      if ( action ) {
         auto wrapAction = [=]( Value& target, const std::string& value, Environment& env ) {
            auto pConverted = dynamic_cast<ConvertedValue<TValue>*>( &target );
            if ( pConverted )
               action( pConverted->mValue, value, env );
         };
         OptionConfig::getOption().setAction( wrapAction );
      }
      else
         OptionConfig::getOption().setAction( nullptr );
      return *this;
   }

   // Define the value that will be assigned to the target if the option is
   // not present in arguments.  If multiple options that are configured with
   // absent() have the same target, the result is undefined.
   this_t& absent( const TValue& defaultValue )
   {
      auto wrapDefault = [=]( Value& target ) {
         auto pConverted = dynamic_cast<ConvertedValue<TValue>*>( &target );
         if ( pConverted )
            pConverted->mValue = defaultValue;
      };
      OptionConfig::getOption().setAssignDefaultAction( wrapDefault );
      return *this;
   }

   // Define the action that will assign the default value to the target if
   // the option is not present in arguments.  If multiple options that are
   // configured with absent() have the same target, the result is undefined.
   this_t& absent( assign_default_action_t action )
   {
      auto wrapDefault = [=]( Value& target ) {
         auto pConverted = dynamic_cast<ConvertedValue<TValue>*>( &target );
         if ( pConverted )
            action( pConverted->mValue );
      };
      OptionConfig::getOption().setAssignDefaultAction( wrapDefault );
      return *this;
   }
};

class VoidOptionConfig : public OptionConfigBaseT<VoidOptionConfig>
{
public:
   using this_t = VoidOptionConfig;
   using assign_action_env_t = std::function<void( const std::string&, Environment& )>;

public:
   using OptionConfigBaseT<this_t>::OptionConfigBaseT;

   VoidOptionConfig( OptionConfig&& wrapped )
      : OptionConfigBaseT<this_t>( std::move( wrapped ) )
   {}

   this_t& action( assign_action_env_t action )
   {
      if ( action ) {
         auto wrapAction = [&]( Value& target, const std::string& value,
                                 Environment& env ) -> std::optional<std::string> {
            auto pv = dynamic_cast<VoidValue*>( &target );
            if ( pv )
               action( value, env );
            return {};
         };
         OptionConfig::getOption().setAction( wrapAction );
      }
      else
         OptionConfig::getOption().setAction( nullptr );
      return *this;
   }
};

}   // namespace argparse
