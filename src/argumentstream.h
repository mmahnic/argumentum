// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <functional>
#include <istream>
#include <memory>
#include <optional>
#include <string_view>

namespace argparse {

class ArgumentStream
{
public:
   enum EPeekResult { peekDone, peekNext };

public:
   virtual ~ArgumentStream() = default;

   // Returns the argument on the current position in an argument stream and
   // moves to the next position.  Returns nullopt if there are no more
   // arguments.
   virtual std::optional<std::string_view> next() = 0;

   // Executes @p fnPeek on arguments in an argument stream from the current
   // position to the end of the range supported by a concrete ArgumentStream
   // implementation.  The current position does not change.
   //
   // The function @p fnPeek returns peekDone to end peeking or peekNext to
   // analyse more arguments.
   //
   // An implementation may choose to not support peeking.
   virtual void peek( std::function<EPeekResult( std::string_view )> fnPeek );
};

// An implementation of ArgumentStream that reads arguments from a string
// container.
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

   void peek( std::function<EPeekResult( std::string_view )> fnPeek ) override
   {
      if ( !fnPeek )
         return;

      for ( auto iarg = current; iarg != end; ++iarg )
         if ( fnPeek( *iarg ) == peekDone )
            break;
   }
};

// An implementation of ArgumentStream that reads characters from an istream and
// merges them into string arguments.
//
// The default implementation expects one argument per line.
class StdStreamArgumentStream : public ArgumentStream
{
   std::shared_ptr<std::istream> mpStream;
   std::string mCurrent;

public:
   StdStreamArgumentStream( const std::shared_ptr<std::istream>& pStream );
   std::optional<std::string_view> next() override;
};

}   // namespace argparse
