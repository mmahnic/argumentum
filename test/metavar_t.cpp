// Copyright (c) 2018-2021 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "testutil.h"

#include <argumentum/argparse.h>

#include <algorithm>
#include <gtest/gtest.h>
#include <sstream>

using namespace argumentum;
using namespace testutil;

namespace {
std::optional<std::string> getMetavarHelpLine(
      int minargs, int maxargs, std::vector<std::string_view> metavarDef )
{
   std::string str;
   auto parser = argument_parser{};
   auto params = parser.params();
   auto& bees = params.add_parameter( str, "--bees" ).metavar( metavarDef );
   if ( minargs < maxargs )
      // only one of minargs, maxargs or nargs can be used
      bees.maxargs( maxargs );
   else if ( minargs == maxargs )
      bees.nargs( minargs );
   else
      bees.minargs( minargs );

   auto formatter = HelpFormatter();
   formatter.setTextWidth( 80 );
   formatter.setMaxDescriptionIndent( 20 );
   auto help = getTestHelp( parser, formatter );
   auto lines = splitLines( help, KEEPEMPTY );

   for ( auto line : lines ) {
      auto optpos = line.find( "--bees" );
      if ( optpos != std::string::npos )
         return std::string( line );
   }

   return {};
}
}   // namespace

TEST( HelpMetavar, shouldUseDefaultMetavarName )
{
   // .minargs(1)
   auto oLine = getMetavarHelpLine( 1, -1, {} );

   ASSERT_TRUE( oLine.has_value() );
   auto argspos = oLine->find( "BEES [BEES ...]" );
   ASSERT_NE( std::string::npos, argspos );
   EXPECT_LT( oLine->find( "--bees" ), argspos );
}

TEST( HelpMetavar, shouldChangeOptionMetavarName )
{
   // .minargs(1)
   auto oLine = getMetavarHelpLine( 1, -1, { "WORK" } );

   ASSERT_TRUE( oLine.has_value() );
   auto argspos = oLine->find( "WORK [WORK ...]" );
   ASSERT_NE( std::string::npos, argspos );
   EXPECT_LT( oLine->find( "--bees" ), argspos );
}

TEST( HelpMetavar, shouldSupportMultipleMetavarsInOption )
{
   // .nargs(2)
   auto oLine = getMetavarHelpLine( 2, 2, { "FLY", "WORK" } );

   ASSERT_TRUE( oLine.has_value() );
   auto argspos = oLine->find( "FLY WORK" );
   ASSERT_NE( std::string::npos, argspos );
   EXPECT_LT( oLine->find( "--bees" ), argspos );
}

TEST( HelpMetavar, shouldReuseTheLastMetavarInOption )
{
   // .nargs(5)
   auto oLine = getMetavarHelpLine( 5, 5, { "FLY", "WORK" } );

   ASSERT_TRUE( oLine.has_value() );
   auto argspos = oLine->find( "FLY WORK WORK WORK WORK" );
   ASSERT_NE( std::string::npos, argspos );
   EXPECT_LT( oLine->find( "--bees" ), argspos );
}

TEST( HelpMetavar, shouldReuseTheLastMetavarInOptionWithMinArgs )
{
   // .minargs(3)
   auto oLine = getMetavarHelpLine( 3, -1, { "FLY", "WORK" } );

   ASSERT_TRUE( oLine.has_value() );
   auto argspos = oLine->find( "FLY WORK WORK [WORK ...]" );
   ASSERT_NE( std::string::npos, argspos );
   EXPECT_LT( oLine->find( "--bees" ), argspos );
}

TEST( HelpMetavar, shouldDisplayExcessiveMetavarsAsOptional )
{
   // .minargs(2)
   auto oLine = getMetavarHelpLine( 2, -1, { "FLY", "WORK", "EAT", "DRINK", "SLEEP" } );

   ASSERT_TRUE( oLine.has_value() );
   auto argspos = oLine->find( "FLY WORK [EAT [DRINK [SLEEP ...]]]" );
   ASSERT_NE( std::string::npos, argspos );
   EXPECT_LT( oLine->find( "--bees" ), argspos );
}

TEST( HelpMetavar, shouldDisplayExcessiveMetavarsAsOptional_WithExactMax )
{
   // .maxargs(5)
   auto oLine = getMetavarHelpLine( -1, 5, { "FLY", "WORK", "EAT", "DRINK", "SLEEP" } );

   ASSERT_TRUE( oLine.has_value() );
   auto argspos = oLine->find( "[FLY [WORK [EAT [DRINK [SLEEP]]]]]" );
   ASSERT_NE( std::string::npos, argspos );
   EXPECT_LT( oLine->find( "--bees" ), argspos );
}

TEST( HelpMetavar, shouldDisplayExcessiveMetavarsAsOptional_WithLowerMax )
{
   // .maxargs(4)
   auto oLine = getMetavarHelpLine( -1, 4, { "FLY", "WORK", "EAT", "DRINK", "SLEEP" } );

   ASSERT_TRUE( oLine.has_value() );
   auto argspos = oLine->find( "[FLY [WORK [EAT [DRINK]]]]" );
   ASSERT_NE( std::string::npos, argspos );
   EXPECT_LT( oLine->find( "--bees" ), argspos );
}

TEST( HelpMetavar, shouldDisplayExcessiveMetavarsAsOptional_WithLowerMaxAtMinPlus1 )
{
   // .maxargs(1)
   auto oLine = getMetavarHelpLine( -1, 1, { "FLY", "WORK", "EAT", "DRINK", "SLEEP" } );

   ASSERT_TRUE( oLine.has_value() );
   auto argspos = oLine->find( "[FLY]" );
   ASSERT_NE( std::string::npos, argspos );
   EXPECT_LT( oLine->find( "--bees" ), argspos );
}

TEST( HelpMetavar, shouldDisplayExcessiveMetavarsAsOptional_WithHigherMax )
{
   // .maxargs(6)
   auto oLine = getMetavarHelpLine( -1, 6, { "FLY", "WORK", "EAT", "DRINK", "SLEEP" } );

   ASSERT_TRUE( oLine.has_value() );
   auto argspos = oLine->find( "[FLY [WORK [EAT [DRINK [SLEEP{0-2}]]]]]" );
   ASSERT_NE( std::string::npos, argspos );
   EXPECT_LT( oLine->find( "--bees" ), argspos );
}
