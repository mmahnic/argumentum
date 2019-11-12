// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "options.h"

namespace argparse {

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
