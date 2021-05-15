// Copyright (c) 2018-2021 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "testutil.h"

#include <argumentum/argparse.h>

#include <algorithm>
#include <gtest/gtest.h>
#include <sstream>

using namespace argumentum;
using namespace testutil;

TEST( HelpMetavar, shouldChangeOptionMetavarName )
{
   std::string str;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( str, "--bees" ).minargs( 1 ).metavar( "WORK" );

   auto formatter = HelpFormatter();
   formatter.setTextWidth( 60 );
   formatter.setMaxDescriptionIndent( 20 );
   auto help = getTestHelp( parser, formatter );
   auto lines = splitLines( help, KEEPEMPTY );

   for ( auto line : lines ) {
      auto optpos = line.find( "--bees" );
      if ( optpos == std::string::npos )
         continue;

      auto argspos = line.find( "WORK [WORK ...]" );
      ASSERT_NE( std::string::npos, argspos );
      EXPECT_LT( optpos, argspos );
   }
}

TEST( HelpMetavar, shouldSupportMultipleMetavarsInOption )
{
   std::string str;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( str, "--bees" ).nargs( 2 ).metavar( { "FLY", "WORK" } );

   auto formatter = HelpFormatter();
   formatter.setTextWidth( 60 );
   formatter.setMaxDescriptionIndent( 20 );
   auto help = getTestHelp( parser, formatter );
   auto lines = splitLines( help, KEEPEMPTY );

   for ( auto line : lines ) {
      auto optpos = line.find( "--bees" );
      if ( optpos == std::string::npos )
         continue;

      auto argspos = line.find( "FLY WORK" );
      ASSERT_NE( std::string::npos, argspos );
      EXPECT_LT( optpos, argspos );
   }
}

TEST( HelpMetavar, shouldReuseTheLastMetavarInOption )
{
   std::string str;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( str, "--bees" ).nargs( 5 ).metavar( { "FLY", "WORK" } );

   auto formatter = HelpFormatter();
   formatter.setTextWidth( 60 );
   formatter.setMaxDescriptionIndent( 20 );
   auto help = getTestHelp( parser, formatter );
   auto lines = splitLines( help, KEEPEMPTY );

   for ( auto line : lines ) {
      auto optpos = line.find( "--bees" );
      if ( optpos == std::string::npos )
         continue;

      auto argspos = line.find( "FLY WORK WORK WORK WORK" );
      ASSERT_NE( std::string::npos, argspos );
      EXPECT_LT( optpos, argspos );
   }
}

TEST( HelpMetavar, shouldReuseTheLastMetavarInOptionWithMinArgs )
{
   std::string str;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( str, "--bees" ).minargs( 3 ).metavar( { "FLY", "WORK" } );

   auto formatter = HelpFormatter();
   formatter.setTextWidth( 60 );
   formatter.setMaxDescriptionIndent( 20 );
   auto help = getTestHelp( parser, formatter );
   auto lines = splitLines( help, KEEPEMPTY );

   for ( auto line : lines ) {
      auto optpos = line.find( "--bees" );
      if ( optpos == std::string::npos )
         continue;

      auto argspos = line.find( "FLY WORK WORK [WORK ...]" );
      ASSERT_NE( std::string::npos, argspos );
      EXPECT_LT( optpos, argspos );
   }
}

TEST( HelpMetavar, shouldDisplayExcessiveMetavarsAsOptional )
{
   std::string str;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( str, "--bees" )
         .minargs( 2 )
         .metavar( { "FLY", "WORK", "EAT", "DRINK", "SLEEP" } );

   auto formatter = HelpFormatter();
   formatter.setTextWidth( 60 );
   formatter.setMaxDescriptionIndent( 20 );
   auto help = getTestHelp( parser, formatter );
   auto lines = splitLines( help, KEEPEMPTY );

   for ( auto line : lines ) {
      auto optpos = line.find( "--bees" );
      if ( optpos == std::string::npos )
         continue;

      auto argspos = line.find( "FLY WORK [EAT [DRINK [SLEEP ...]]]" );
      ASSERT_NE( std::string::npos, argspos );
      EXPECT_LT( optpos, argspos );
   }
}

TEST( HelpMetavar, shouldDisplayExcessiveMetavarsAsOptional_WithExactMax )
{
   std::string str;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( str, "--bees" )
         .minargs( 2 )
         .maxargs( 5 )
         .metavar( { "FLY", "WORK", "EAT", "DRINK", "SLEEP" } );

   auto formatter = HelpFormatter();
   formatter.setTextWidth( 60 );
   formatter.setMaxDescriptionIndent( 20 );
   auto help = getTestHelp( parser, formatter );
   auto lines = splitLines( help, KEEPEMPTY );

   for ( auto line : lines ) {
      auto optpos = line.find( "--bees" );
      if ( optpos == std::string::npos )
         continue;

      auto argspos = line.find( "FLY WORK [EAT [DRINK [SLEEP]]]" );
      ASSERT_NE( std::string::npos, argspos );
      EXPECT_LT( optpos, argspos );
   }
}

TEST( HelpMetavar, shouldDisplayExcessiveMetavarsAsOptional_WithLowerMax )
{
   std::string str;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( str, "--bees" )
         .minargs( 2 )
         .maxargs( 4 )
         .metavar( { "FLY", "WORK", "EAT", "DRINK", "SLEEP" } );

   auto formatter = HelpFormatter();
   formatter.setTextWidth( 60 );
   formatter.setMaxDescriptionIndent( 20 );
   auto help = getTestHelp( parser, formatter );
   auto lines = splitLines( help, KEEPEMPTY );

   for ( auto line : lines ) {
      auto optpos = line.find( "--bees" );
      if ( optpos == std::string::npos )
         continue;

      auto argspos = line.find( "FLY WORK [EAT [DRINK]]" );
      ASSERT_NE( std::string::npos, argspos );
      EXPECT_LT( optpos, argspos );
   }
}

TEST( HelpMetavar, shouldDisplayExcessiveMetavarsAsOptional_WithHigherMax )
{
   std::string str;
   auto parser = argument_parser{};
   auto params = parser.params();
   params.add_parameter( str, "--bees" )
         .minargs( 2 )
         .maxargs( 6 )
         .metavar( { "FLY", "WORK", "EAT", "DRINK", "SLEEP" } );

   auto formatter = HelpFormatter();
   formatter.setTextWidth( 60 );
   formatter.setMaxDescriptionIndent( 20 );
   auto help = getTestHelp( parser, formatter );
   auto lines = splitLines( help, KEEPEMPTY );

   for ( auto line : lines ) {
      auto optpos = line.find( "--bees" );
      if ( optpos == std::string::npos )
         continue;

      auto argspos = line.find( "FLY WORK [EAT [DRINK [SLEEP{0-2}]]]" );
      ASSERT_NE( std::string::npos, argspos );
      EXPECT_LT( optpos, argspos );
   }
}
