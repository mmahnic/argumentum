// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "option.h"

namespace argumentum {

class ParameterConfig;

/**
 * OptionConfig is used to configure an option after an option was created with add_argument.
 */
class OptionConfig
{
   friend class ::argumentum::ParameterConfig;

private:
   std::shared_ptr<Option> mpOption;
   bool mCountWasSet = false;

protected:
   OptionConfig( const OptionConfig& ) = default;
   OptionConfig( OptionConfig&& ) = default;
   OptionConfig( const std::shared_ptr<Option>& pOption );

   Option& getOption() const;
   void markCountWasSet();
   void ensureCountWasNotSet() const;
   void ensureCanBeForwarded() const;
};

template<typename TDerived>
class OptionConfigBaseT : public OptionConfig
{
   friend class ::argumentum::ParameterConfig;

public:
   using this_t = TDerived;

public:
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

   // Define the name of the meta variable of the option that will be used as
   // a placeholder for option values in the generated help.
   //
   // @p varname a string or a vector of strings.
   this_t& metavar( std::string_view varname )
   {
      getOption().setMetavar( { varname } );
      return *static_cast<this_t*>( this );
   }

   // Define the names of the meta variables of the option that will be used as
   // a placeholder for option values in the generated help.
   //
   // @p varname a string or a vector of strings.
   this_t& metavar( const std::vector<std::string_view>& varnames )
   {
      getOption().setMetavar( varnames );
      return *static_cast<this_t*>( this );
   }

   // Define the description of the option that will be displayed in the
   // generated help.
   this_t& help( std::string_view help )
   {
      getOption().setHelp( help );
      return *static_cast<this_t*>( this );
   }

   // Define the exact number of values that an option can accept.
   this_t& nargs( int count )
   {
      ensureCountWasNotSet();
      getOption().setNArgs( count );
      markCountWasSet();
      return *static_cast<this_t*>( this );
   }

   // Define the minimum number of values that an option can accept.
   this_t& minargs( int count )
   {
      ensureCountWasNotSet();
      getOption().setMinArgs( count );
      markCountWasSet();
      return *static_cast<this_t*>( this );
   }

   // Define the maximum number of values that an option can accept.
   this_t& maxargs( int count )
   {
      ensureCountWasNotSet();
      getOption().setMaxArgs( count );
      markCountWasSet();
      return *static_cast<this_t*>( this );
   }

   // Set to true if the option must be present in the input arguments.
   this_t& required( bool isRequired = true )
   {
      getOption().setRequired( isRequired );
      return *static_cast<this_t*>( this );
   }

   // Define the value that will be stored in the target variable.  Only valid
   // if this is a flag option and action is not set.
   this_t& flagValue( std::string_view value )
   {
      getOption().setFlagValue( value );
      return *static_cast<this_t*>( this );
   }

   // Define the values accepted by an option.
   this_t& choices( const std::vector<std::string>& choices )
   {
      getOption().setChoices( choices );
      return *static_cast<this_t*>( this );
   }

   // Set to true if the parameters of this option are forwarded to a
   // subprocess or processed in a different way.  The parameters are a part of
   // this option, they are a comma separated list that is separated from the
   // option name with a comma.
   //
   // Forwarding works only with long options.  The value of this option should
   // be a list of strings that will hold the parameters to forward.
   //
   // @example Store the options --silent and --threads=3 so that they can be
   // forwarded to a subprocess.
   //
   //    --forward,--silent,--threads=3
   this_t& forward( bool isForwarded = true )
   {
      ensureCanBeForwarded();
      getOption().setForwarded( isForwarded );
      return *static_cast<this_t*>( this );
   }

protected:
   using OptionConfig::OptionConfig;

   OptionConfigBaseT( OptionConfig&& wrapped )
      : OptionConfig( std::move( wrapped ) )
   {}
};

template<typename TTarget>
class OptionConfigA final : public OptionConfigBaseT<OptionConfigA<TTarget>>
{
   using this_t = OptionConfigA<TTarget>;
   using assign_action_t = std::function<void( TTarget&, const std::string& )>;
   using assign_action_env_t = std::function<void( TTarget&, const std::string&, Environment& )>;
   using assign_default_action_t = std::function<void( TTarget& )>;

public:
   using OptionConfigBaseT<this_t>::OptionConfigBaseT;

   OptionConfigA( OptionConfig&& wrapped )
      : OptionConfigBaseT<this_t>( std::move( wrapped ) )
   {}

   // Define an action to execute when the option is present in input arguments.
   // The action is executed instead of the default assignment action and can
   // set the value of the target variable associated with the option.
   this_t& action( assign_action_t action )
   {
      if ( action ) {
         auto wrapAction = [=]( Value& value, const std::string& argument, Environment& ) {
            auto pConverted = ConvertedValue<TTarget>::value_cast( value );
            if ( pConverted )
               action( pConverted->mTarget, argument );
         };
         OptionConfig::getOption().setAction( wrapAction );
      }
      else
         OptionConfig::getOption().setAction( nullptr );
      return *this;
   }

   // Define an action to execute when the option is present in input arguments.
   // The action is executed instead of the default assignment action and can
   // set the value of the target variable associated with the option.
   // This version has access to the parsing environment.
   this_t& action( assign_action_env_t action )
   {
      if ( action ) {
         auto wrapAction = [=]( Value& value, const std::string& argument, Environment& env ) {
            auto pConverted = ConvertedValue<TTarget>::value_cast( value );
            if ( pConverted )
               action( pConverted->mTarget, argument, env );
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
   this_t& absent( const TTarget& defaultValue )
   {
      auto wrapDefault = [=]( Value& value ) {
         auto pConverted = ConvertedValue<TTarget>::value_cast( value );
         if ( pConverted )
            pConverted->mTarget = defaultValue;
      };
      OptionConfig::getOption().setAssignDefaultAction( wrapDefault );
      return *this;
   }

   // Define the action that will assign the default value to the target if
   // the option is not present in arguments.  If multiple options that are
   // configured with absent() have the same target, the result is undefined.
   this_t& absent( assign_default_action_t action )
   {
      auto wrapDefault = [=]( Value& value ) {
         auto pConverted = ConvertedValue<TTarget>::value_cast( value );
         if ( pConverted )
            action( pConverted->mTarget );
      };
      OptionConfig::getOption().setAssignDefaultAction( wrapDefault );
      return *this;
   }
};

class VoidOptionConfig final : public OptionConfigBaseT<VoidOptionConfig>
{
public:
   using assign_action_env_t = std::function<void( const std::string&, Environment& )>;

public:
   using OptionConfigBaseT<VoidOptionConfig>::OptionConfigBaseT;
   VoidOptionConfig( OptionConfig&& wrapped );

   // Define an action to execute when the option is present in input arguments.
   // The action is executed instead of the default assignment action and can
   // set the value of the target variable associated with the option.
   VoidOptionConfig& action( assign_action_env_t action );
};

}   // namespace argumentum
