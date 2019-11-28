// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "options.h"
#include "values.h"

#include <memory>
#include <vector>

namespace argparse {

class OptionFactory
{
public:
   template<typename TTarget>
   Option createOption( TTarget& value )
   {
      std::unique_ptr<Value> pValue;
      if constexpr ( std::is_base_of<Value, TTarget>::value ) {
         pValue = std::make_unique<TTarget>( value );
      }
      else {
         using wrap_type = ConvertedValue<TTarget>;
         pValue = std::make_unique<wrap_type>( value );
      }

      return Option( std::move( pValue ), Option::singleValue );
   }

   template<typename TTarget>
   Option createOption( std::vector<TTarget>& value )
   {
      std::unique_ptr<Value> pValue;
      using val_vector = std::vector<TTarget>;
      if constexpr ( std::is_base_of<Value, TTarget>::value ) {
         pValue = std::make_unique<val_vector>( value );
      }
      else {
         using wrap_type = ConvertedValue<val_vector>;
         pValue = std::make_unique<wrap_type>( value );
      }

      return Option( std::move( pValue ), Option::vectorValue );
   }
};

}   // namespace argparse
