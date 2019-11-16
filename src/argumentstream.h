// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

namespace argparse {

class ArgumentStream
{
public:
   virtual std::optional<std::string_view> next() = 0;
};

template<typename TIter>
class IteratorArgumentStream : public ArgumentStream
{
   TIter current;
   TIter end;

public:
   IteratorArgumentStream( TIter begin, TIter end )
      : current( begin )
      , end( end )
   {}

   std::optional<std::string_view> next() override
   {
      if ( current == end )
         return {};

      return *current++;
   }
};

}   // namespace argparse
