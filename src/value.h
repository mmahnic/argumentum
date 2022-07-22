// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "convert.h"
#include "notifier.h"

#ifndef ARGUMENTUM_BUILD_MODULE
#include <functional>
#include <string>
#endif

namespace argumentum {

class Environment;
class Value;

using ValueId = uintptr_t;
using ValueTypeId = uintptr_t;
using TargetId = std::pair<ValueTypeId, uintptr_t>;

/**
 * The assign-action is executed to set the value of a parameter.
 *
 * If an action is not provided with the OptionConfig::action method, a
 * default action will be created and used.
 *
 * Action interfaces are different for different types of Value descendatns.
 * All actions are wrapped into AssignAction interface.
 */
using AssignAction =
      std::function<void( Value& target, const std::string& value, Environment& env )>;

/**
 * The assign-default action is executed when an option with a default
 * (absent) value is not set through arguments.  The default value is
 * captured in the function.
 */
using AssignDefaultAction = std::function<void( Value& target )>;

class Value
{
   int mAssignCount = 0;
   bool mHasErrors = false;

public:
   void setValue( std::string_view value, AssignAction action, Environment& env );
   void setDefault( AssignDefaultAction action );
   void markBadArgument();

   /**
    * The count of assignments through all the options that share this value.
    */
   int getAssignCount() const;

   void onOptionStarted();
   void reset();

   virtual ValueId getValueId() const;
   virtual ValueTypeId getValueTypeId() const = 0;
   virtual TargetId getTargetId() const;

protected:
   virtual AssignAction getDefaultAction() = 0;
   virtual void doReset();
};

class VoidValue : public Value
{
public:
   ValueTypeId getValueTypeId() const override;
   static VoidValue* value_cast( Value& value );

protected:
   AssignAction getDefaultAction() override;
};

template<typename T>
class OptionConfigA;

template<typename TTarget>
class ConvertedValue : public Value
{
   template<typename T>
   friend class ::argumentum::OptionConfigA;

   // Check if std::string can be converted to TVal with argparse::from_string.
   template<typename TVal>
   struct has_from_string
   {
   private:
      typedef char YesType[1];
      typedef char NoType[2];

      template<typename C>
      static YesType& test( decltype( &C::convert ) );
      template<typename C>
      static NoType& test( ... );

   public:
      enum { value = sizeof( test<::argumentum::from_string<TVal>>( 0 ) ) == sizeof( YesType ) };
   };

   // Check if std::string can be converted to TVal with constructors or
   // assignment operators.
   template<class TVal>
   struct can_convert   // (clf)
      : std::integral_constant<bool,   // (clf)
              std::is_constructible<std::string, TVal>::value   // (clf)
                    || std::is_convertible<std::string, TVal>::value   // (clf)
                    || std::is_assignable<TVal, std::string>::value   // (clf)
              >
   {
      template<typename T>
      constexpr static bool has_from_string()
      {
         return true;
      }
   };

protected:
   TTarget& mTarget;

public:
   ConvertedValue( TTarget& value )
      : mTarget( value )
   {}

   ValueTypeId getValueTypeId() const override
   {
      return valueTypeId();
   }

   TargetId getTargetId() const override
   {
      return std::make_pair( getValueTypeId(), reinterpret_cast<uintptr_t>( &mTarget ) );
   }

   static ValueTypeId valueTypeId()
   {
      static char tid = 0;
      return reinterpret_cast<uintptr_t>( &tid );
   }

   static ConvertedValue<TTarget>* value_cast( Value& value )
   {
      if ( value.getValueTypeId() != valueTypeId() )
         return nullptr;

      return static_cast<ConvertedValue<TTarget>*>( &value );
   }

protected:
   AssignAction getDefaultAction() override
   {
      return []( Value& value, const std::string& argument, Environment& ) {
         auto pConverted = ConvertedValue<TTarget>::value_cast( value );
         if ( pConverted )
            pConverted->assign( pConverted->mTarget, argument );
      };
   }

   void doReset() override
   {
      mTarget = TTarget{};
   }

   template<typename TVar>
   void assign( std::vector<TVar>& var, const std::string& value )
   {
      TVar target;
      assign( target, value );
      var.emplace_back( std::move( target ) );
   }

   template<typename TVar>
   void assign( std::optional<TVar>& var, const std::string& value )
   {
      TVar target;
      assign( target, value );
      var = std::move( target );
   }

   template<typename TVar, std::enable_if_t<has_from_string<TVar>::value, int> = 0>
   void assign( TVar& var, const std::string& value )
   {
      var = ::argumentum::from_string<TVar>::convert( value );
   }

   template<typename TVar,
         std::enable_if_t<!has_from_string<TVar>::value && can_convert<TVar>::value, int> = 0>
   void assign( TVar& var, const std::string& value )
   {
      var = TVar{ value };
   }

   template<typename TVar,
         std::enable_if_t<!has_from_string<TVar>::value && !can_convert<TVar>::value, int> = 0>
   void assign( TVar& var, const std::string& value )
   {
      Notifier::warn( "Assignment is not implemented. ('" + value + "')" );
   }
};

}   // namespace argumentum
