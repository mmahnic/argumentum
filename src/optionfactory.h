// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "exceptions.h"
#include "option.h"
#include "value.h"

#include <cassert>
#include <map>
#include <memory>
#include <vector>

namespace argparse {

class OptionFactory
{
   std::map<TargetId, std::shared_ptr<Value>> mValueFromTargetId;

public:
   template<typename TTarget>
   Option createOption( TTarget& value )
   {
      std::shared_ptr<Value> pValue;
      if constexpr ( std::is_base_of<Value, TTarget>::value ) {
         pValue = std::make_shared<TTarget>( value );
      }
      else {
         using wrap_type = ConvertedValue<TTarget>;
         pValue = std::make_shared<wrap_type>( value );
      }

      return Option( getValueForKnownTarget( pValue ), Option::singleValue );
   }

   template<typename TTarget>
   Option createOption( std::vector<TTarget>& value )
   {
      std::shared_ptr<Value> pValue;
      using val_vector = std::vector<TTarget>;
      if constexpr ( std::is_base_of<Value, TTarget>::value ) {
         throw UnsupportedTargetType( "Unsupported target type: vector<Value>." );
      }
      else {
         using wrap_type = ConvertedValue<val_vector>;
         pValue = std::make_shared<wrap_type>( value );
      }

      return Option( getValueForKnownTarget( pValue ), Option::vectorValue );
   }

private:
   std::shared_ptr<Value> getValueForKnownTarget( std::shared_ptr<Value> pValue )
   {
      assert( pValue );
      auto iv = mValueFromTargetId.find( pValue->getTargetId() );
      if ( iv != mValueFromTargetId.end() )
         return iv->second;

      mValueFromTargetId[pValue->getTargetId()] = pValue;
      return pValue;
   }
};

}   // namespace argparse
