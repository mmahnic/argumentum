// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "convert.h"
#include "notifier.h"
#include "values.h"
#include <functional>
#include <string>

namespace argparse {

inline void Value::setValue( std::string_view value, AssignAction action, Environment& env )
{
   ++mAssignCount;
   ++mOptionAssignCount;
   if ( action == nullptr )
      action = getDefaultAction();
   if ( action )
      action( *this, std::string{ value }, env );
}

inline void Value::setDefault( AssignDefaultAction action )
{
   if ( action ) {
      ++mAssignCount;
      action( *this );
   }
}

inline void Value::markBadArgument()
{
   // Increase the assign count so that flagValue will not be used.
   ++mOptionAssignCount;
   mHasErrors = true;
}

inline int Value::getAssignCount() const
{
   return mAssignCount;
}

inline int Value::getOptionAssignCount() const
{
   return mOptionAssignCount;
}

inline void Value::onOptionStarted()
{
   mOptionAssignCount = 0;
}

inline void Value::reset()
{
   mAssignCount = 0;
   mOptionAssignCount = 0;
   mHasErrors = false;
   doReset();
}

inline void Value::doReset()
{}

inline AssignAction VoidValue::getDefaultAction()
{
   return {};
}

}   // namespace argparse
