// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "value.h"

#include <functional>
#include <string>

namespace argumentum {

ARGUMENTUM_INLINE ValueId Value::getValueId() const
{
   return reinterpret_cast<ValueId>( this );
}

ARGUMENTUM_INLINE TargetId Value::getTargetId() const
{
   return std::make_pair( getValueTypeId(), 0 );
}

ARGUMENTUM_INLINE void Value::setValue(
      std::string_view value, AssignAction action, Environment& env )
{
   ++mAssignCount;
   if ( action == nullptr )
      action = getDefaultAction();
   if ( action )
      action( *this, std::string{ value }, env );
}

ARGUMENTUM_INLINE void Value::setDefault( AssignDefaultAction action )
{
   if ( action ) {
      ++mAssignCount;
      action( *this );
   }
}

ARGUMENTUM_INLINE void Value::setMissingValue( std::string_view flagValue, Environment& env )
{
   auto action = getMissingValueAction();
   if ( action ) {
      ++mAssignCount;
      std::string fv{ flagValue };
      action( *this, fv, env );
   }
}

ARGUMENTUM_INLINE void Value::markBadArgument()
{
   // Increase the assign count so that flagValue will not be used.
   mHasErrors = true;
}

ARGUMENTUM_INLINE int Value::getAssignCount() const
{
   return mAssignCount;
}

ARGUMENTUM_INLINE void Value::onOptionStarted()
{}

ARGUMENTUM_INLINE void Value::reset()
{
   mAssignCount = 0;
   mHasErrors = false;
   doReset();
}

ARGUMENTUM_INLINE void Value::doReset()
{}

ARGUMENTUM_INLINE uintptr_t VoidValue::getValueTypeId() const
{
   return 0;
}

ARGUMENTUM_INLINE VoidValue* VoidValue::value_cast( Value& value )
{
   if ( value.getValueTypeId() != 0 )
      return nullptr;

   return static_cast<VoidValue*>( &value );
}

ARGUMENTUM_INLINE AssignAction VoidValue::getDefaultAction()
{
   return {};
}

ARGUMENTUM_INLINE AssignAction VoidValue::getMissingValueAction()
{
   return {};
}

}   // namespace argumentum
