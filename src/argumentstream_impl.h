// Copyright (c) 2018, 2019, 2020 Marko Mahnič
//
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "argumentstream.h"

namespace argumentum {

ARGUMENTUM_INLINE void ArgumentStream::peek( std::function<EPeekResult( std::string_view )> )
{}

ARGUMENTUM_INLINE StdStreamArgumentStream::StdStreamArgumentStream(
      const std::shared_ptr<std::istream>& pStream )
   : mpStream( pStream )
{}

ARGUMENTUM_INLINE std::optional<std::string_view> StdStreamArgumentStream::next()
{
   if ( !mpStream )
      return {};

   if ( !std::getline( *mpStream, mCurrent ) )
      return {};

   return mCurrent;
}

}   // namespace argumentum
