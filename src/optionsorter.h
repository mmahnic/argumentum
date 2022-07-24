// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "iformathelp.h"

#ifndef ARGUMENTUM_BUILD_MODULE
#include "nomodule.h"

#include <vector>
#endif

namespace argumentum {

ARGUMENTUM_EXPORT
class OptionSorter
{
public:
   struct GroupLimit
   {
      using iterator_t = std::vector<ArgumentHelpResult>::iterator;
      // Start of group in the reordered ArgumentHelpResult vector.
      iterator_t ibegin;
      // End of positional parameters.
      iterator_t iendpos;
      // End of requiered parameters.
      iterator_t iendreq;
      // End of optional parameters, end of group.
      iterator_t iend;
      GroupLimit( iterator_t begin, iterator_t end )
         : ibegin( begin )
         , iendpos( begin )
         , iendreq( begin )
         , iend( end )
      {}
   };

   std::vector<GroupLimit> reorderGroups( std::vector<ArgumentHelpResult>& args );
   void reorderOptions( GroupLimit& limit );
};

}   // namespace argumentum
