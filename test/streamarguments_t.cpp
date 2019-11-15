// Copyright (c) 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "../src/argparser.h"

#include <algorithm>
#include <gtest/gtest.h>

using namespace argparse;

TEST( StreamArguments, shouldReadArgumentsFromFilesystem )
{
   auto pfs = std::make_shared<TestFilesystem>;
   pfs->addFile( "a.opt", "--alpha --beta" );
   pfs->addFile( "b.opt", "--three --four" );

   auto parser = argument_parser{};
   parser.config().filesystem( pfs );

   std::array<bool, 6> v{ false, false, false, false, false, false };
   parser.add_argument( v[0], "--alpha" ).nargs( 0 );
   parser.add_argument( v[1], "--beta" ).nargs( 0 );
   parser.add_argument( v[2], "--three" ).nargs( 0 );
   parser.add_argument( v[3], "--four" ).nargs( 0 );
   parser.add_argument( v[4], "--alice" ).nargs( 0 );
   parser.add_argument( v[5], "--bob" ).nargs( 0 );

   parser.parse_args( { "--alice", "@a.opt", "@b.opt", "--bob" } );

   for ( auto flag : v )
      EXPECT_TRUE( flag );
}
