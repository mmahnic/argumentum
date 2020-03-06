// Copyright (c) 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "testutil.h"

#include <argumentum/argparse.h>

#include <algorithm>
#include <gtest/gtest.h>

using namespace argumentum;
using namespace testutil;

namespace {
struct CmdOneOptions : public argumentum::CommandOptions
{
   std::optional<std::string> str;
   std::optional<long> count;

   using CommandOptions::CommandOptions;

   void add_parameters( ParameterConfig& params ) override
   {
      params.add_parameter( str, "-s" ).nargs( 1 );
      params.add_parameter( count, "-n" ).nargs( 1 );
   }
};

struct CmdTwoOptions : public argumentum::CommandOptions
{
   std::optional<std::string> str;
   std::optional<long> count;

   using CommandOptions::CommandOptions;

   void add_parameters( ParameterConfig& params ) override
   {
      params.add_parameter( str, "--string" ).nargs( 1 );
      params.add_parameter( count, "--count" ).nargs( 1 );
   }
};
}   // namespace

TEST( ArgumentParserCommand, shouldHandleCommandsWithSubparsers )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );
   auto params = parser.params();

   params.add_command<CmdOneOptions>( "one" );
   params.add_command<CmdTwoOptions>( "two" );

   // -- WHEN
   auto res = parser.parse_args( { "one", "-s", "works" } );

   // -- THEN
   auto pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   auto pCmdTwo = findCommand<CmdTwoOptions>( res, "two" );
   EXPECT_TRUE( res.errors.empty() );
   EXPECT_EQ( nullptr, pCmdTwo );
   ASSERT_NE( nullptr, pCmdOne );
   EXPECT_TRUE( pCmdOne->str.has_value() );
   EXPECT_EQ( "works", pCmdOne->str.value_or( "" ) );
   EXPECT_FALSE( pCmdOne->count.has_value() );

   // -- WHEN
   res = parser.parse_args( { "two", "--string", "works" } );

   // -- THEN
   pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   pCmdTwo = findCommand<CmdTwoOptions>( res, "two" );
   EXPECT_TRUE( res.errors.empty() );
   EXPECT_EQ( nullptr, pCmdOne );
   ASSERT_NE( nullptr, pCmdTwo );
   EXPECT_TRUE( pCmdTwo->str.has_value() );
   EXPECT_EQ( "works", pCmdTwo->str.value_or( "" ) );
   EXPECT_FALSE( pCmdTwo->count.has_value() );

   // -- WHEN
   res = parser.parse_args( { "-s", "works" } );

   // -- THEN
   pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   pCmdTwo = findCommand<CmdTwoOptions>( res, "two" );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_FALSE( res.errors.empty() );
   EXPECT_EQ( UNKNOWN_OPTION, res.errors.front().errorCode );
   EXPECT_EQ( nullptr, pCmdOne );
   EXPECT_EQ( nullptr, pCmdTwo );
}

// Form: program --global-options command --command-options
TEST( ArgumentParserCommand, shouldHandleGlobalOptionsWhenCommandsPresent )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );
   auto params = parser.params();

   std::optional<std::string> global;
   params.add_parameter( global, "-s" ).nargs( 1 );
   params.add_command<CmdOneOptions>( "one" );

   auto res = parser.parse_args( { "-s", "global-works", "one", "-s", "command-works" } );
   EXPECT_TRUE( res.errors.empty() );

   EXPECT_TRUE( global.has_value() );
   EXPECT_EQ( "global-works", global.value_or( "" ) );

   auto pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   ASSERT_NE( nullptr, pCmdOne );
   EXPECT_EQ( "command-works", pCmdOne->str.value_or( "" ) );
}

// A rewrite of the previous test with GlobalOptions structure.
TEST( ArgumentParserCommand, shouldHandleGlobalOptionsWhenCommandsPresent2 )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );
   auto params = parser.params();

   struct GlobalOptions : public argumentum::Options
   {
      std::optional<std::string> global;
      void add_parameters( ParameterConfig& params ) override
      {
         params.add_parameter( global, "-s" ).nargs( 1 );
      }
   };

   auto pGlobal = std::make_shared<GlobalOptions>();
   ASSERT_NE( nullptr, pGlobal );
   params.add_parameters( pGlobal );
   params.add_command<CmdOneOptions>( "one" );

   auto res = parser.parse_args( { "-s", "global-works", "one", "-s", "command-works" } );
   EXPECT_TRUE( res.errors.empty() );

   EXPECT_TRUE( pGlobal->global.has_value() );
   EXPECT_EQ( "global-works", pGlobal->global.value_or( "" ) );

   auto pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   ASSERT_NE( nullptr, pCmdOne );
   EXPECT_EQ( "command-works", pCmdOne->str.value_or( "" ) );
}

TEST( ArgumentParserCommand, shouldRequireParentsRequiredOptionsWhenCommandPresent )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );
   auto params = parser.params();

   struct GlobalOptions : public argumentum::Options
   {
      std::optional<std::string> global;
      void add_parameters( ParameterConfig& params ) override
      {
         params.add_parameter( global, "-s" ).nargs( 1 ).required( true );
      }
   };

   auto pGlobal = std::make_shared<GlobalOptions>();
   ASSERT_NE( nullptr, pGlobal );
   params.add_parameters( pGlobal );
   params.add_command<CmdOneOptions>( "one" );

   auto res = parser.parse_args( { "one", "-s", "command-works" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_FALSE( res.errors.empty() );
   EXPECT_EQ( MISSING_OPTION, res.errors.front().errorCode );

   auto pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   ASSERT_NE( nullptr, pCmdOne );
   EXPECT_EQ( "command-works", pCmdOne->str.value_or( "" ) );
}

// The parser tries to process commands before positional arguments. If an
// allowed argument value is equal to the name of a command, the ambiguity is
// resolved in favour of the command.
TEST( ArgumentParserCommand, shouldRequireParentsRequiredPositionalWhenCommandPresent )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );
   auto params = parser.params();

   struct GlobalOptions : public argumentum::Options
   {
      std::optional<std::string> global;
      void add_parameters( ParameterConfig& params ) override
      {
         params.add_parameter( global, "str" ).nargs( 1 ).required( true );
      }
   };

   auto pGlobal = std::make_shared<GlobalOptions>();
   ASSERT_NE( nullptr, pGlobal );
   params.add_parameters( pGlobal );
   params.add_command<CmdOneOptions>( "one" );

   auto res = parser.parse_args( { "one", "-s", "command-works" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_FALSE( res.errors.empty() );
   EXPECT_EQ( MISSING_ARGUMENT, res.errors.front().errorCode );

   auto pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   ASSERT_NE( nullptr, pCmdOne );
   EXPECT_EQ( "command-works", pCmdOne->str.value_or( "" ) );
}

TEST( ArgumentParserCommand, shouldStoreInstantiatedCommandsInParseResults )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );
   auto params = parser.params();

   params.add_command<CmdOneOptions>( "one" );
   params.add_command<CmdTwoOptions>( "two" );

   // -- WHEN
   auto res = parser.parse_args( { "one", "-s", "works" } );

   // -- THEN
   EXPECT_TRUE( res.errors.empty() );
   auto pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   auto pCmdTwo = findCommand<CmdTwoOptions>( res, "two" );
   EXPECT_EQ( nullptr, pCmdTwo );
   ASSERT_NE( nullptr, pCmdOne );
   EXPECT_TRUE( pCmdOne->str.has_value() );
   EXPECT_EQ( "works", pCmdOne->str.value_or( "" ) );
   EXPECT_FALSE( pCmdOne->count.has_value() );

   // -- WHEN
   res = parser.parse_args( { "two", "--string", "works" } );

   // -- THEN
   EXPECT_TRUE( res.errors.empty() );
   pCmdOne = findCommand<CmdOneOptions>( res, "one" );
   pCmdTwo = findCommand<CmdTwoOptions>( res, "two" );
   EXPECT_EQ( nullptr, pCmdOne );
   ASSERT_NE( nullptr, pCmdTwo );
   EXPECT_TRUE( pCmdTwo->str.has_value() );
   EXPECT_EQ( "works", pCmdTwo->str.value_or( "" ) );
   EXPECT_FALSE( pCmdTwo->count.has_value() );
}

TEST( ArgumentParserCommand, shouldReportErrorsOnlyInTopLevelParser )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );
   auto params = parser.params();

   params.add_command<CmdOneOptions>( "one" );

   // -- WHEN
   auto res = parser.parse_args( { "one", "--bad-option" } );
   EXPECT_FALSE( static_cast<bool>( res ) );

   // -- THEN
   auto help = strout.str();
   auto lines = splitLines( help );
   int count = 0;
   for ( auto line : lines ) {
      if ( strHasText( line, "--bad-option" ) )
         ++count;
   }
   EXPECT_EQ( 1, count );
}

TEST( ArgumentParserCommand, shouldAcceptInstantiatedOptions )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );
   auto params = parser.params();

   auto pCmdOne = std::make_shared<CmdOneOptions>( "one" );
   params.add_command( pCmdOne );

   // -- WHEN
   auto res = parser.parse_args( { "one", "-s", "works" } );

   // -- THEN
   EXPECT_TRUE( pCmdOne->str.has_value() );
   EXPECT_EQ( "works", pCmdOne->str.value_or( "" ) );
   EXPECT_FALSE( pCmdOne->count.has_value() );
}

// When using commands we may need to access global options from the command.
// We can do this by using instantiated command options that have a reference to
// the global options.  Global options must be stored in an Options structure.
TEST( ArgumentParserCommand, shouldAccessParentOptionsFromCommand )
{
   class Global : public argumentum::Options
   {
   public:
      int global = 0;
      void add_parameters( ParameterConfig& params ) override
      {
         params.add_parameter( global, "--global" ).nargs( 1 );
      }
   };

   class CmdOneWithGlobal : public CmdOneOptions
   {
   public:
      std::shared_ptr<Global> mpGlobal;

   public:
      CmdOneWithGlobal( std::string_view name, std::shared_ptr<Global> pGlobal )
         : CmdOneOptions( name )
         , mpGlobal( pGlobal )
      {}
   };

   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );
   auto params = parser.params();

   auto pGlobal = std::make_shared<Global>();
   auto pCmdOne = std::make_shared<CmdOneWithGlobal>( "one", pGlobal );
   params.add_parameters( pGlobal );
   params.add_command( pCmdOne );

   // -- WHEN
   auto res = parser.parse_args( { "--global", "5", "one", "-s", "works" } );

   // -- THEN
   EXPECT_TRUE( pCmdOne->str.has_value() );
   EXPECT_EQ( "works", pCmdOne->str.value_or( "" ) );
   EXPECT_FALSE( pCmdOne->count.has_value() );
   ASSERT_NE( nullptr, pCmdOne->mpGlobal );
   EXPECT_EQ( 5, pCmdOne->mpGlobal->global );
}
