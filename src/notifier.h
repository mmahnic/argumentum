// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <iostream>

namespace argumentum {

class Notifier
{
public:
   static void warn( std::string_view text )
   {
      std::cerr << "** " << text << "\n";
   }
};

}   // namespace argumentum
