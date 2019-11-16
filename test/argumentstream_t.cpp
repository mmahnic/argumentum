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

   ASSERT_EQ( args.size(), res.size() );

   for ( unsigned i = 0; i < args.size(); ++i )
      EXPECT_EQ( args[i], res[i] );
}
