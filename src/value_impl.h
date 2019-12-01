// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "value.h"

#include <functional>
#include <string>

namespace argparse {

CPPARGPARSE_INLINE ValueId Value::getValueId() const
{
   return reinterpret_cast<ValueId>( this );
}

CPPARGPARSE_INLINE TargetId Value::getTargetId() const
{
   return std::make_pair( getValueTypeId(), 0 );
}

CPPARGPARSE_INLINE void Value::setValue(
      std::string_view value, AssignAction action, Environment& env )
{
   ++mAssignCount;
   if ( action == nullptr )
      action = getDefaultAction();
   if ( action )
      action( *this, std::string{ value }, env );
}

CPPARGPARSE_INLINE void Value::setDefault( AssignDefaultAction action )
{
   if ( action ) {
      ++mAssignCount;
      action( *this );
   }
}

CPPARGPARSE_INLINE void Value::markBadArgument()
{
   // Increase the assign count so that flagValue will not be used.
   mHasErrors = true;
}

CPPARGPARSE_INLINE int Value::getAssignCount() const
{
   return mAssignCount;
}

CPPARGPARSE_INLINE void Value::onOptionStarted()
{}

CPPARGPARSE_INLINE void Value::reset()
{
   mAssignCount = 0;
   mHasErrors = false;
   doReset();
}

CPPARGPARSE_INLINE void Value::doReset()
{}

CPPARGPARSE_INLINE uintptr_t VoidValue::getValueTypeId() const
{
   return 0;
}

CPPARGPARSE_INLINE VoidValue* VoidValue::value_cast( Value& value )
{
   if ( value.getValueTypeId() != 0 )
      return nullptr;

   return static_cast<VoidValue*>( &value );
}

CPPARGPARSE_INLINE AssignAction VoidValue::getDefaultAction()
{
   return {};
}

}   // namespace argparse
