// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#ifndef ARGUMENTUM_BUILD_MODULE
#include <iostream>
#endif

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
