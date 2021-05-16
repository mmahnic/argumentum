// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "testutil.h"
#include "vectors.h"

#include <argumentum/argparse.h>

#include <algorithm>
#include <gtest/gtest.h>

using namespace argumentum;
using namespace testing;
using namespace testutil;

// The configuration should always return a stream. If a stream is not
// explicitly configured, std::cout is returned.
TEST( ParserConfig, shouldSetParserOutputToStream )
{
   auto parser = argument_parser{};
   auto pDefaultStream = parser.getConfig().output_stream();
   EXPECT_NE( nullptr, pDefaultStream );
   EXPECT_EQ( &std::cout, pDefaultStream );

   std::stringstream strout;
   parser.config().cout( strout );

   auto pConfiguredStream = parser.getConfig().output_stream();
   EXPECT_NE( nullptr, pConfiguredStream );
   EXPECT_EQ( &strout, pConfiguredStream );
}

TEST( ParserConfig, shouldChangeHelpFormatter )
{
   namespace t = ::testing;

   // NOTE: Could not make MOCK_METHOD work on gcc 7.4/Ubuntu 18.04; this is an
   // approximation.
   class MockFormatter : public IFormatHelp
   {
   public:
      unsigned formatCount = 0;

   public:
      void format( const ParserDefinition&, std::ostream& ) override
      {
         ++formatCount;
      }
   };

   auto pFormatter = std::make_shared<MockFormatter>();
   auto parser = argument_parser{};
   parser.config().help_formatter( pFormatter );

   auto res = parser.parse_args( { "--help" } );
   EXPECT_FALSE( static_cast<bool>( res ) );

   EXPECT_EQ( 1U, pFormatter->formatCount );
}
