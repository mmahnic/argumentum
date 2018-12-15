#include <gtest/gtest.h>
#include "../src/argparser.h"

TEST( CArgumentParserTest, shouldParseShortOptions )
{
   std::optional<std::string> value;

   CArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).hasArgument();
   parser.parseArguments( { "-v", "success" } );

   EXPECT_EQ( "success", value.value() );
}

TEST( CArgumentParserTest, shouldParseLongOptions )
{
   std::optional<std::string> value;

   CArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).longName( "value" ).hasArgument();
   parser.parseArguments( { "--value", "success" } );

   EXPECT_EQ( "success", value.value() );
}

TEST( CArgumentParserTest, shouldParseIntegerValues )
{
   std::optional<long> value;

   CArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).longName( "value" ).hasArgument();
   parser.parseArguments( { "--value", "2314" } );

   EXPECT_EQ( 2314, value.value() );
}

TEST( CArgumentParserTest, shouldNotSetOptionValuesWithoutArguments )
{
   std::optional<long> value;
   std::optional<std::string> unused;

   CArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).longName( "value" ).hasArgument();
   parser.addOption( unused ).longName( "unused" );
   parser.parseArguments( { "--value", "2314" } );

   EXPECT_EQ( 2314, value.value() );
   EXPECT_FALSE( bool(unused) );
}

TEST( CArgumentParserTest, shouldOnlyAddOptionValueIfRequired )
{
   std::optional<long> value;
   std::optional<std::string> flag;

   CArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).longName( "value" ).hasArgument();
   parser.addOption( flag ).longName( "flag" );

   parser.parseArguments( { "--value", "2314", "--flag", "notused" } );

   EXPECT_EQ( 2314, value.value() );

   // The parameters that do not require an argumet will be given the value "1".
   // TODO: This can be changed with the flagValue() parameter option.
   EXPECT_EQ( "1", flag.value() );
}

TEST( CArgumentParserTest, shouldSkipParsingOptionsAfterDashDash )
{
   std::optional<long> value;
   std::optional<std::string> flag;

   CArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).longName( "value" ).hasArgument();
   parser.addOption( flag ).longName( "skipped" );

   parser.parseArguments( { "--value", "2314", "--", "--skipped" } );

   EXPECT_EQ( 2314, value.value() );
   EXPECT_FALSE( bool(flag) );
}

TEST( CArgumentParserTest, shouldSupportShortOptionGroups )
{
   std::optional<long> flagA;
   std::optional<std::string> flagB;
   std::optional<std::string> flagC;
   std::optional<long> flagD;

   CArgumentParser parser;
   parser.addOption( flagA ).shortName( "a" );
   parser.addOption( flagB ).shortName( "b" );
   parser.addOption( flagC ).shortName( "c" );
   parser.addOption( flagD ).shortName( "d" );

   parser.parseArguments( { "-abd" } );

   EXPECT_EQ( 1, flagA.value() );
   EXPECT_EQ( "1", flagB.value() );
   EXPECT_FALSE( bool(flagC) );
   EXPECT_EQ( 1, flagD.value() );
}

TEST( CArgumentParserTest, shouldReadArgumentForLastOptionInGroup )
{
   std::optional<long> flagA;
   std::optional<std::string> flagB;
   std::optional<std::string> flagC;
   std::optional<long> flagD;

   CArgumentParser parser;
   parser.addOption( flagA ).shortName( "a" );
   parser.addOption( flagB ).shortName( "b" );
   parser.addOption( flagC ).shortName( "c" );
   parser.addOption( flagD ).shortName( "d" ).hasArgument();

   parser.parseArguments( { "-abd", "4213" } );

   EXPECT_EQ( 1, flagA.value() );
   EXPECT_EQ( "1", flagB.value() );
   EXPECT_FALSE( bool(flagC) );
   EXPECT_EQ( 4213, flagD.value() );
}

TEST( CArgumentParserTest, shouldReportErrorForMissingArgument )
{
   std::optional<long> flagA;
   std::optional<std::string> flagB;

   CArgumentParser parser;
   parser.addOption( flagA ).shortName( "a" ).hasArgument();
   parser.addOption( flagB ).shortName( "b" );

   auto res = parser.parseArguments( { "-a", "-b", "freearg" } );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "a", res.errors.front().option );
   ASSERT_EQ( 1, res.freeArguments.size() );
   EXPECT_EQ( "freearg", res.freeArguments.front() );
}
