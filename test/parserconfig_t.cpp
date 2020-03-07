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

TEST( ParserConfig, shouldSetParserOutputToStream )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   EXPECT_EQ( nullptr, parser.getConfig().output_stream() );

   parser.config().cout( strout );

   // NOTE: EXPECT_NE fails to compile with MSVC 2017, 15.9.16
   EXPECT_TRUE( nullptr != parser.getConfig().output_stream() );
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

   EXPECT_EQ( 1, pFormatter->formatCount );
}
