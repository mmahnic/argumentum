// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#ifndef ARGUMENTUM_BUILD_MODULE
#include <algorithm>
#endif

namespace argumentum {

ARGUMENTUM_INLINE std::vector<OptionSorter::GroupLimit> OptionSorter::reorderGroups(
      std::vector<ArgumentHelpResult>& args )
{
   // 1. required free group
   // 2. required groups by name
   // 3. optional free group
   // 4. optional groups by name
   // 5. commands

   auto lowerGroup = []( auto&& a, auto&& b ) {
      if ( a.isCommand == b.isCommand ) {
         if ( a.group.isRequired == b.group.isRequired )
            return a.group.name < b.group.name;

         // Required before optional ( false < true => !isRequired(true) < isRequired(true) )
         return !a.group.isRequired < !b.group.isRequired;
      }

      return a.isCommand < b.isCommand;
   };

   std::stable_sort( std::begin( args ), std::end( args ), lowerGroup );

   std::vector<GroupLimit> limits;
   auto iprev = std::begin( args );
   for ( auto icur = iprev; icur != std::end( args ); ++icur ) {
      if ( lowerGroup( *iprev, *icur ) ) {
         limits.emplace_back( iprev, icur );
         iprev = icur;
      }
   }
   if ( iprev != std::end( args ) )
      limits.emplace_back( iprev, std::end( args ) );

   return limits;
}

ARGUMENTUM_INLINE void OptionSorter::reorderOptions( GroupLimit& limit )
{
   limit.iendpos = std::stable_partition(
         limit.ibegin, limit.iend, []( auto&& opt ) { return opt.is_positional(); } );
   limit.iendreq = std::stable_partition(
         limit.iendpos, limit.iend, []( auto&& opt ) { return opt.is_required(); } );
}

}   // namespace argumentum
