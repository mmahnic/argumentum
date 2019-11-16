// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "../src/argparser.h"
#include "../src/argumentstream.h"

#include <gtest/gtest.h>

using namespace argparse;

TEST( ArgumentStream, shouldUseIteratorsAsAStream )
{
   std::vector<std::string> args{ "one", "two", "three" };

   IteratorArgumentStream stream( args.begin(), args.end() );
   std::vector<std::string> res;
   for ( auto arg = stream.next(); !!arg; arg = stream.next() )
      res.push_back( std::string{ *arg } );

   ASSERT_EQ( 3, res.size() );

   EXPECT_EQ( "one", res[0] );
   EXPECT_EQ( "two", res[1] );
   EXPECT_EQ( "three", res[2] );
}

TEST( ArgumentStream, shouldPeekNextArgumentsInStream )
{
   std::vector<std::string> args{ "one", "two", "three" };

   IteratorArgumentStream stream( args.begin(), args.end() );
   auto arg = stream.next();
   EXPECT_EQ( "one", arg );

   bool foundOne = false;
   bool foundTwo = false;
   bool foundThree = false;
   stream.peek( [&]( auto&& arg ) {
      if ( arg == "two" ) {
         foundTwo = true;
         return ArgumentStream::peekDone;
      }
      else if ( arg == "one" )
         foundTwo = true;
      else if ( arg == "three" )
         foundThree = true;

      return ArgumentStream::peekNext;
   } );

   EXPECT_FALSE( foundOne );
   EXPECT_TRUE( foundTwo );
   EXPECT_FALSE( foundThree );
}

TEST( ArgumentStream, shouldCreateArgumentStreamFromStdStream )
{
   auto pInput = std::make_shared<std::stringstream>( "one\ntwo\nthree" );

   StdStreamArgumentStream stream( pInput );
   std::vector<std::string> res;
   for ( auto arg = stream.next(); !!arg; arg = stream.next() )
      res.push_back( std::string{ *arg } );

   ASSERT_EQ( 3, res.size() );

   EXPECT_EQ( "one", res[0] );
   EXPECT_EQ( "two", res[1] );
   EXPECT_EQ( "three", res[2] );
}
