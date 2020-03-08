// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include <iostream>
#include <vector>

namespace testing {

template<typename T>
bool vector_eq( const std::vector<T>& values, const std::vector<T>& var )
{
   auto dump = [&var]() {
      std::cout << "Result: ";
      for ( auto& v : var )
         std::cout << "'" << v << "'  ";
      std::cout << "\n";
   };

   if ( values.size() != var.size() ) {
      std::cout << "Size: " << values.size() << "!=" << var.size() << "\n";
      dump();
      return false;
   }

   for ( int i = 0; i < var.size(); ++i )
      if ( values[i] != var[i] ) {
         std::cout << "Value: '" << values[i] << "'!='" << var[i] << "'\n";
         dump();
         return false;
      }

   return true;
}

}   // namespace testing
