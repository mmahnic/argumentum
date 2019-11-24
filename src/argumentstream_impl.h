// Copyright (c) 2018, 2019 Marko Mahnič
//
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "argumentstream.h"

namespace argparse {

inline void ArgumentStream::peek( std::function<EPeekResult( std::string_view )> fnPeek )
{}

inline StdStreamArgumentStream::StdStreamArgumentStream(
      const std::shared_ptr<std::istream>& pStream )
   : mpStream( pStream )
{}

inline std::optional<std::string_view> StdStreamArgumentStream::next()
{
   if ( !mpStream )
      return {};

   if ( !std::getline( *mpStream, mCurrent ) )
      return {};

   return mCurrent;
}

}   // namespace argparse
