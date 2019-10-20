// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include "vectors.h"
#include "../src/argparser.h"

#include <algorithm>
#include <gtest/gtest.h>

using namespace argparse;
using namespace testing;

TEST( ArgumentParserTest, shouldParseShortOptions )
{
   std::optional<std::string> value;

   auto parser = argument_parser{};
   parser.add_argument( value, "-v" ).nargs( 1 );
   parser.parse_args( { "-v", "success" } );

   EXPECT_EQ( "success", value.value() );
}

TEST( ArgumentParserTest, shouldParseLongOptions )
{
   std::optional<std::string> value;

   auto parser = argument_parser{};
   parser.add_argument( value, "--value", "-v" ).nargs( 1 );
   parser.parse_args( { "--value", "success" } );

   EXPECT_EQ( "success", value.value() );
}

TEST( ArgumentParserTest, shouldNotSetOptionValuesWithoutArguments )
{
   std::optional<long> value;
   std::optional<std::string> unused;

   auto parser = argument_parser{};
   parser.add_argument( value, "-v", "--value" ).nargs( 1 );
   parser.add_argument( unused, "--unused" );
   parser.parse_args( { "--value", "2314" } );

   EXPECT_EQ( 2314, value.value() );
   EXPECT_FALSE( bool( unused ) );
}

TEST( ArgumentParserTest, shouldOnlyAddOptionValueIfRequired )
{
   std::optional<long> value;
   std::optional<std::string> flag;

   auto parser = argument_parser{};
   parser.add_argument( value, "-v", "--value" ).nargs( 1 );
   parser.add_argument( flag, "--flag" );

   parser.parse_args( { "--value", "2314", "--flag", "notused" } );

   EXPECT_EQ( 2314, value.value() );

   // The parameters that do not require an argumet will be given the value "1".
   // NOTE: This can be changed with the flagValue() parameter option.
   EXPECT_EQ( "1", flag.value() );
}

TEST( ArgumentParserTest, shouldSkipParsingOptionsAfterDashDash )
{
   std::optional<long> value;
   std::optional<std::string> flag;

   auto parser = argument_parser{};
   parser.add_argument( value, "-v", "--value" ).nargs( 1 );
   parser.add_argument( flag, "--skipped" );

   parser.parse_args( { "--value", "2314", "--", "--skipped" } );

   EXPECT_EQ( 2314, value.value() );
   EXPECT_FALSE( bool( flag ) );
}

TEST( ArgumentParserTest, shouldSupportShortOptionGroups )
{
   std::optional<long> flagA;
   std::optional<std::string> flagB;
   std::optional<std::string> flagC;
   std::optional<long> flagD;

   auto parser = argument_parser{};
   parser.add_argument( flagA, "-a" );
   parser.add_argument( flagB, "-b" );
   parser.add_argument( flagC, "-c" );
   parser.add_argument( flagD, "-d" );

   parser.parse_args( { "-abd" } );

   EXPECT_EQ( 1, flagA.value() );
   EXPECT_EQ( "1", flagB.value() );
   EXPECT_FALSE( bool( flagC ) );
   EXPECT_EQ( 1, flagD.value() );
}

TEST( ArgumentParserTest, shouldReadArgumentForLastOptionInGroup )
{
   std::optional<long> flagA;
   std::optional<std::string> flagB;
   std::optional<std::string> flagC;
   std::optional<long> flagD;

   auto parser = argument_parser{};
   parser.add_argument( flagA, "-a" );
   parser.add_argument( flagB, "-b" );
   parser.add_argument( flagC, "-c" );
   parser.add_argument( flagD, "-d" ).nargs( 1 );

   parser.parse_args( { "-abd", "4213" } );

   EXPECT_EQ( 1, flagA.value() );
   EXPECT_EQ( "1", flagB.value() );
   EXPECT_FALSE( bool( flagC ) );
   EXPECT_EQ( 4213, flagD.value() );
}

TEST( ArgumentParserTest, shouldReportErrorForMissingArgument )
{
   std::optional<long> flagA;
   std::optional<std::string> flagB;

   auto parser = argument_parser{};
   parser.add_argument( flagA, "-a" ).nargs( 1 );
   parser.add_argument( flagB, "-b" );

   auto res = parser.parse_args( { "-a", "-b", "freearg" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "-a", res.errors.front().option );
   ASSERT_EQ( 1, res.ignoredArguments.size() );
   EXPECT_EQ( "freearg", res.ignoredArguments.front() );
   EXPECT_EQ( argument_parser::MISSING_ARGUMENT, res.errors.front().errorCode );
}

TEST( ArgumentParserTest, shouldReportUnknownOptionError )
{
   std::optional<long> flagA;

   auto parser = argument_parser{};
   parser.add_argument( flagA, "-a" ).nargs( 1 );

   auto res = parser.parse_args( { "-a", "2135", "--unknown" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "--unknown", res.errors.front().option );
   EXPECT_EQ( argument_parser::UNKNOWN_OPTION, res.errors.front().errorCode );
}

TEST( ArgumentParserTest, shouldReportMissingRequiredOptionError )
{
   std::optional<long> flagA;
   std::optional<long> flagB;

   auto parser = argument_parser{};
   parser.add_argument( flagA, "-a" ).nargs( 1 );
   parser.add_argument( flagA, "-b" ).required();

   auto res = parser.parse_args( { "-a", "2135" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "-b", res.errors.front().option );
   EXPECT_EQ( argument_parser::MISSING_OPTION, res.errors.front().errorCode );
}

TEST( ArgumentParserTest, shouldAcceptOptionNamesInConstructor )
{
   std::string strvalue;

   auto parser = argument_parser{};
   parser.add_argument( strvalue, "-s", "--string" ).nargs( 1 );

   auto res = parser.parse_args( { "-s", "short" } );
   EXPECT_EQ( 0, res.errors.size() );
   EXPECT_EQ( "short", strvalue );

   res = parser.parse_args( { "--string", "long" } );
   EXPECT_EQ( 0, res.errors.size() );
   EXPECT_EQ( "long", strvalue );
}

TEST( ArgumentParserTest, shouldNotAcceptInvalidShortOptions )
{
   std::string strvalue;

   auto parser = argument_parser{};
   parser.add_argument( strvalue, "-s", "--string" ).nargs( 1 );
   parser.add_argument( strvalue, "--l" ).nargs( 1 );

   EXPECT_THROW( parser.add_argument( strvalue, "-long" ).nargs( 1 ), std::invalid_argument );

   auto res = parser.parse_args( { "-s", "short" } );
   EXPECT_EQ( 0, res.errors.size() );
   EXPECT_EQ( "short", strvalue );

   res = parser.parse_args( { "--string", "long" } );
   EXPECT_EQ( 0, res.errors.size() );
   EXPECT_EQ( "long", strvalue );

   res = parser.parse_args( { "--l", "onecharlong" } );
   EXPECT_EQ( 0, res.errors.size() );
   EXPECT_EQ( "onecharlong", strvalue );
}

TEST( ArgumentParserTest, shouldNotAcceptOptionsWithoutName )
{
   std::string strvalue;

   auto parser = argument_parser{};
   EXPECT_THROW( parser.add_argument( strvalue, "-" ), std::invalid_argument );
   EXPECT_THROW( parser.add_argument( strvalue, "--" ), std::invalid_argument );
   EXPECT_THROW( parser.add_argument( strvalue, "" ), std::invalid_argument );
}

TEST( ArgumentParserTest, shouldStorePositionalArgumentsInValues )
{
   std::vector<std::string> strings;

   auto parser = argument_parser{};
   parser.add_argument( strings, "text" ).minargs( 0 );

   auto res = parser.parse_args( { "one", "two", "three" } );

   EXPECT_TRUE( vector_eq( { "one", "two", "three" }, strings ) );
}

TEST( ArgumentParserTest, shouldGroupPositionalArguments )
{
   std::string strvalue;
   std::string firstArgument;
   std::vector<std::string> otherArguments;

   auto makeParser = [&]() {
      auto parser = argument_parser{};
      parser.add_argument( strvalue, "-s", "--string" ).nargs( 1 );
      parser.add_argument( strvalue, "--l" ).nargs( 1 );
      parser.add_argument( firstArgument, "text" ).nargs( 1 );
      parser.add_argument( otherArguments, "args" ).minargs( 0 );
      return parser;
   };

   auto parser = makeParser();
   auto res = parser.parse_args( { "-s", "string", "first", "second", "third" } );
   EXPECT_EQ( "first", firstArgument );
   EXPECT_TRUE( vector_eq( { "second", "third" }, otherArguments ) );

   parser = makeParser();
   firstArgument.clear();
   otherArguments.clear();
   res = parser.parse_args( { "first", "second", "-s", "string", "third" } );
   EXPECT_EQ( "first", firstArgument );
   EXPECT_TRUE( vector_eq( { "second", "third" }, otherArguments ) );
}

TEST( ArgumentParserTest, shouldSupportOptionArgumentCounts )
{
   std::string strvalue;
   std::vector<std::string> texts;
   std::vector<std::string> files;

   auto parser = argument_parser{};
   parser.add_argument( strvalue, "-s" ).nargs( 1 );
   parser.add_argument( texts, "-t" ).nargs( 2 );
   parser.add_argument( files, "-f" ).minargs( 0 );

   parser.parse_args( { "-t", "the", "text", "-f", "file1", "file2", "file3", "-s", "string" } );
   EXPECT_EQ( "string", strvalue );
   EXPECT_TRUE( vector_eq( { "the", "text" }, texts ) );
   EXPECT_TRUE( vector_eq( { "file1", "file2", "file3" }, files ) );
}

TEST( ArgumentParserTest, shouldFailWhenOptionArgumentCountsAreWrong )
{
   std::string strvalue;
   std::vector<std::string> texts;
   std::vector<std::string> files;

   auto parser = argument_parser{};
   parser.add_argument( strvalue, "-s" ).nargs( 1 );
   parser.add_argument( texts, "-t" ).nargs( 2 );
   parser.add_argument( files, "-f" ).nargs( 2 );

   auto res =
         parser.parse_args( { "-t", "the", "-f", "file1", "file2", "not-file3", "-s", "string" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_EQ( "string", strvalue );
   EXPECT_TRUE( vector_eq( { "the" }, texts ) );
   EXPECT_TRUE( vector_eq( { "file1", "file2" }, files ) );

   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "-t", res.errors.front().option );
   EXPECT_EQ( argument_parser::MISSING_ARGUMENT, res.errors.front().errorCode );

   EXPECT_TRUE( vector_eq( { "not-file3" }, res.ignoredArguments ) );
}

TEST( ArgumentParserTest, shouldSupportPositionalArgumentCounts )
{
   std::string strvalue;
   std::vector<std::string> texts;
   std::vector<std::string> files;

   auto parser = argument_parser{};
   parser.add_argument( strvalue, "-s" ).nargs( 1 );
   parser.add_argument( texts, "text" ).nargs( 2 );
   parser.add_argument( files, "file" ).nargs( 2 );
   auto res = parser.parse_args(
         { "the", "-s", "string1", "text", "file1", "file2", "not-file3", "-s", "string2" } );

   EXPECT_EQ( "string2", strvalue );
   EXPECT_TRUE( vector_eq( { "the", "text" }, texts ) );
   EXPECT_TRUE( vector_eq( { "file1", "file2" }, files ) );
   EXPECT_TRUE( vector_eq( { "not-file3" }, res.ignoredArguments ) );
}

TEST( ArgumentParserTest, shouldSupportExactNumberOfOptionArguments )
{
   std::vector<std::string> texts;

   auto testWithNargs = [&]( int nargs, const std::vector<std::string>& params ) {
      texts.clear();
      auto parser = argument_parser{};
      parser.add_argument( texts, "-t" ).nargs( nargs );
      return parser.parse_args( params );
   };
   auto params = std::vector<std::string>{ "-t", "read", "the", "text" };

   auto res = testWithNargs( 0, params );
   // When an option doesn't accept arguments, the default value is set/added
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_TRUE( vector_eq( { "1" }, texts ) );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithNargs( 1, params );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_TRUE( vector_eq( { "read" }, texts ) );
   EXPECT_TRUE( vector_eq( { "the", "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithNargs( 2, params );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_TRUE( vector_eq( { "read", "the" }, texts ) );
   EXPECT_TRUE( vector_eq( { "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithNargs( 3, params );
   EXPECT_TRUE( static_cast<bool>( res ) );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) );
   EXPECT_EQ( 0, res.ignoredArguments.size() );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithNargs( 4, params );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) );
   EXPECT_EQ( 0, res.ignoredArguments.size() );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "-t", res.errors[0].option );
   EXPECT_EQ( argument_parser::MISSING_ARGUMENT, res.errors[0].errorCode );
}

TEST( ArgumentParserTest, shouldSupportExactNumberOfPositionalArguments )
{
   std::vector<std::string> texts;

   auto testWithNargs = [&]( int nargs, const std::vector<std::string>& params ) {
      texts.clear();
      auto parser = argument_parser{};
      parser.add_argument( texts, "text" ).nargs( nargs );
      return parser.parse_args( params );
   };
   auto params = std::vector<std::string>{ "read", "the", "text" };

   auto res = testWithNargs( 0, params );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_EQ( 0, texts.size() );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithNargs( 1, params );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_TRUE( vector_eq( { "read" }, texts ) );
   EXPECT_TRUE( vector_eq( { "the", "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithNargs( 2, params );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_TRUE( vector_eq( { "read", "the" }, texts ) );
   EXPECT_TRUE( vector_eq( { "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithNargs( 3, params );
   EXPECT_TRUE( static_cast<bool>( res ) );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) );
   EXPECT_EQ( 0, res.ignoredArguments.size() );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithNargs( 4, params );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) );
   EXPECT_EQ( 0, res.ignoredArguments.size() );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "text", res.errors[0].option );
   EXPECT_EQ( argument_parser::MISSING_ARGUMENT, res.errors[0].errorCode );
}

TEST( ArgumentParserTest, shouldSupportMinNumberOfOptionArguments )
{
   std::vector<std::string> texts;

   auto testWithMinArgs = [&]( int nargs, const std::vector<std::string>& params ) {
      texts.clear();
      auto parser = argument_parser{};
      parser.add_argument( texts, "-t" ).minargs( nargs );
      return parser.parse_args( params );
   };
   auto params = std::vector<std::string>{ "-t", "read", "the", "text" };

   for ( int nargs = 0; nargs < 4; ++nargs ) {
      auto res = testWithMinArgs( nargs, params );
      EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) ) << "minargs:" << nargs;
      EXPECT_EQ( 0, res.ignoredArguments.size() ) << "minargs:" << nargs;
      EXPECT_EQ( 0, res.errors.size() ) << "minargs:" << nargs;
   }

   auto res = testWithMinArgs( 4, params );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) );
   EXPECT_EQ( 0, res.ignoredArguments.size() );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "-t", res.errors[0].option );
   EXPECT_EQ( argument_parser::MISSING_ARGUMENT, res.errors[0].errorCode );
}

TEST( ArgumentParserTest, shouldSupportMaxNumberOfOptionArguments )
{
   std::vector<std::string> texts;

   auto testWithMaxArgs = [&]( int nargs, const std::vector<std::string>& params ) {
      texts.clear();
      auto parser = argument_parser{};
      parser.add_argument( texts, "-t" ).maxargs( nargs );
      return parser.parse_args( params );
   };
   auto params = std::vector<std::string>{ "-t", "read", "the", "text" };

   auto res = testWithMaxArgs( 0, params );
   // When an option doesn't accept arguments, the default value is set/added
   EXPECT_TRUE( vector_eq( { "1" }, texts ) );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithMaxArgs( 1, params );
   EXPECT_TRUE( vector_eq( { "read" }, texts ) );
   EXPECT_TRUE( vector_eq( { "the", "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithMaxArgs( 2, params );
   EXPECT_TRUE( vector_eq( { "read", "the" }, texts ) );
   EXPECT_TRUE( vector_eq( { "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   for ( int nargs = 3; nargs < 5; ++nargs ) {
      auto res = testWithMaxArgs( nargs, params );
      EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) ) << "maxargs:" << nargs;
      EXPECT_EQ( 0, res.ignoredArguments.size() ) << "maxargs:" << nargs;
      EXPECT_EQ( 0, res.errors.size() ) << "maxargs:" << nargs;
   }
}

TEST( ArgumentParserTest, shouldSupportMinNumberOfPositionalArguments )
{
   std::vector<std::string> texts;

   auto testWithMinArgs = [&]( int nargs, const std::vector<std::string>& params ) {
      texts.clear();
      auto parser = argument_parser{};
      parser.add_argument( texts, "text" ).minargs( nargs );
      return parser.parse_args( params );
   };
   auto params = std::vector<std::string>{ "read", "the", "text" };

   for ( int nargs = 0; nargs < 4; ++nargs ) {
      auto res = testWithMinArgs( nargs, params );
      EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) ) << "maxargs:" << nargs;
      EXPECT_EQ( 0, res.errors.size() ) << "maxargs:" << nargs;
      EXPECT_EQ( 0, res.ignoredArguments.size() ) << "maxargs:" << nargs;
   }

   auto res = testWithMinArgs( 4, params );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) );
   EXPECT_EQ( 0, res.ignoredArguments.size() );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "text", res.errors[0].option );
   EXPECT_EQ( argument_parser::MISSING_ARGUMENT, res.errors[0].errorCode );
}

TEST( ArgumentParserTest, shouldSupportMaxNumberOfPositionalArguments )
{
   std::vector<std::string> texts;

   auto testWithMaxArgs = [&]( int nargs, const std::vector<std::string>& params ) {
      texts.clear();
      auto parser = argument_parser{};
      parser.add_argument( texts, "text" ).maxargs( nargs );
      return parser.parse_args( params );
   };
   auto params = std::vector<std::string>{ "read", "the", "text" };

   auto res = testWithMaxArgs( 0, params );
   EXPECT_EQ( 0, texts.size() );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithMaxArgs( 1, params );
   EXPECT_TRUE( vector_eq( { "read" }, texts ) );
   EXPECT_TRUE( vector_eq( { "the", "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithMaxArgs( 2, params );
   EXPECT_TRUE( vector_eq( { "read", "the" }, texts ) );
   EXPECT_TRUE( vector_eq( { "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   for ( int nargs = 3; nargs < 5; ++nargs ) {
      auto res = testWithMaxArgs( nargs, params );
      EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) ) << "maxargs:" << nargs;
      EXPECT_EQ( 0, res.ignoredArguments.size() ) << "maxargs:" << nargs;
      EXPECT_EQ( 0, res.errors.size() ) << "maxargs:" << nargs;
   }
}

TEST( ArgumentParserTest, shouldSetDefaultCountForPositionalArgumentsWithVectorValues )
{
   std::vector<std::string> texts;
   auto params = std::vector<std::string>{ "read", "the", "text" };

   auto parser = argument_parser{};
   // If the value variable is a vector, the default is minargs(0)
   parser.add_argument( texts, "text" );

   auto res = parser.parse_args( params );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) );
   EXPECT_EQ( 0, res.ignoredArguments.size() );
   EXPECT_EQ( 0, res.errors.size() );
}

TEST( ArgumentParserTest, shouldSetDefaultCountForPositionalArgumentsWithScalarValues )
{
   std::string strvalue;
   auto params = std::vector<std::string>{ "read", "the", "text" };

   auto parser = argument_parser{};
   // If the value variable is a scalar, the default is nargs(1)
   parser.add_argument( strvalue, "text" );

   auto res = parser.parse_args( params );
   EXPECT_EQ( "read", strvalue );
   EXPECT_TRUE( vector_eq( { "the", "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );
}

TEST( ArgumentParserTest, shouldSetFlagValueWhenZeroOrMoreArgumentsRequiredAndNoneGiven )
{
   std::vector<std::string> texts;
   auto parser = argument_parser{};
   parser.add_argument( texts, "-t" ).maxargs( 1 );

   auto res = parser.parse_args( { "-t" } );
   EXPECT_TRUE( vector_eq( { "1" }, texts ) );
}

TEST( ArgumentParserTest, shouldSetArgumentCountAtMostOnce )
{
   std::vector<std::string> texts;
   auto parser = argument_parser{};

   EXPECT_NO_THROW( parser.add_argument( texts, "-a" ).nargs( 1 ) );
   EXPECT_NO_THROW( parser.add_argument( texts, "-b" ).minargs( 1 ) );
   EXPECT_NO_THROW( parser.add_argument( texts, "-c" ).maxargs( 1 ) );

   EXPECT_THROW(
         parser.add_argument( texts, "-d" ).nargs( 1 ).minargs( 1 ), std::invalid_argument );
   EXPECT_THROW(
         parser.add_argument( texts, "-e" ).nargs( 1 ).maxargs( 1 ), std::invalid_argument );
   EXPECT_THROW(
         parser.add_argument( texts, "-f" ).minargs( 1 ).nargs( 1 ), std::invalid_argument );
   EXPECT_THROW(
         parser.add_argument( texts, "-g" ).minargs( 1 ).maxargs( 1 ), std::invalid_argument );
   EXPECT_THROW(
         parser.add_argument( texts, "-h" ).maxargs( 1 ).nargs( 1 ), std::invalid_argument );
   EXPECT_THROW(
         parser.add_argument( texts, "-i" ).maxargs( 1 ).minargs( 1 ), std::invalid_argument );
}

TEST( ArgumentParserTest, shouldSetOptionChoices )
{
   std::string strvalue;
   auto parser = argument_parser{};

   parser.add_argument( strvalue, "-s" ).nargs( 1 ).choices( { "alpha", "beta", "gamma" } );
   auto res = parser.parse_args( { "-s", "beta" } );
   EXPECT_EQ( "beta", strvalue );
   EXPECT_EQ( 0, res.errors.size() );
}

TEST( ArgumentParserTest, shouldFailIfArgumentIsNotInChoices )
{
   std::string strvalue;
   auto parser = argument_parser{};

   parser.add_argument( strvalue, "-s" ).nargs( 1 ).choices( { "alpha", "beta", "gamma" } );
   auto res = parser.parse_args( { "-s", "phi" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_TRUE( strvalue.empty() );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "-s", res.errors[0].option );
   EXPECT_EQ( argument_parser::INVALID_CHOICE, res.errors[0].errorCode );
}

TEST( ArgumentParserTest, shouldFailIfPositionalArgumentIsNotInChoices )
{
   std::string strvalue;
   auto parser = argument_parser{};

   parser.add_argument( strvalue, "string" ).nargs( 1 ).choices( { "alpha", "beta", "gamma" } );
   auto res = parser.parse_args( { "phi" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_TRUE( strvalue.empty() );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "string", res.errors[0].option );
   EXPECT_EQ( argument_parser::INVALID_CHOICE, res.errors[0].errorCode );
}

TEST( ArgumentParserTest, shouldDistinguishLongAndShortAndPositionalNames )
{
   std::string strShort;
   std::string strLong;
   std::string strArg;

   auto parser = argument_parser{};
   parser.add_argument( strShort, "-s" ).nargs( 1 );
   parser.add_argument( strLong, "--s" ).nargs( 1 );
   parser.add_argument( strArg, "s" ).nargs( 1 );

   auto res = parser.parse_args( { "-s", "short", "--s", "long", "string" } );
   EXPECT_EQ( "short", strShort );
   EXPECT_EQ( "long", strLong );
   EXPECT_EQ( "string", strArg );
   EXPECT_EQ( 0, res.errors.size() );
}

TEST( ArgumentParserTest, shouldNotAcceptOptionsWithWhitespace )
{
   std::string strvalue;
   auto parser = argument_parser{};

   EXPECT_THROW( parser.add_argument( strvalue, "a string" ), std::invalid_argument );
   EXPECT_THROW( parser.add_argument( strvalue, " string" ), std::invalid_argument );
   EXPECT_THROW( parser.add_argument( strvalue, "string " ), std::invalid_argument );
   EXPECT_THROW( parser.add_argument( strvalue, "-string " ), std::invalid_argument );
   EXPECT_THROW( parser.add_argument( strvalue, "--string " ), std::invalid_argument );
   EXPECT_THROW( parser.add_argument( strvalue, "-a string" ), std::invalid_argument );
   EXPECT_THROW( parser.add_argument( strvalue, "--a string" ), std::invalid_argument );
   EXPECT_THROW( parser.add_argument( strvalue, "- string" ), std::invalid_argument );
   EXPECT_THROW( parser.add_argument( strvalue, "-- string" ), std::invalid_argument );
}

TEST( ArgumentParserTest, shouldAcceptSharedOptionStructure )
{
   struct Options : public argparse::Options
   {
      std::string str;
      long count;

      void add_arguments( argument_parser& parser ) override
      {
         parser.add_argument( str, "-s" ).nargs( 1 );
         parser.add_argument( count, "-n" ).nargs( 1 );
      }
   };

   auto parser = argument_parser{};
   auto pOpt = std::make_shared<Options>();
   parser.add_arguments( pOpt );
   auto res = parser.parse_args( { "-s", "str", "-n", "3274" } );
   EXPECT_EQ( "str", pOpt->str );
   EXPECT_EQ( 3274, pOpt->count );
}

TEST( ArgumentParserTest, shouldAcceptMultipleSharedOptionStructures )
{
   struct Options : public argparse::Options
   {
      std::string str;
      long count;

      void add_arguments( argument_parser& parser ) override
      {
         parser.add_argument( str, "-s" ).nargs( 1 );
         parser.add_argument( count, "-n" ).nargs( 1 );
      }
   };

   struct MoreOptions : public argparse::Options
   {
      std::string str;
      long count;

      void add_arguments( argument_parser& parser ) override
      {
         parser.add_argument( str, "-S" ).nargs( 1 );
         parser.add_argument( count, "-N" ).nargs( 1 );
      }
   };

   auto parser = argument_parser{};
   auto pOpt = std::make_shared<Options>();
   auto pMoreOpt = std::make_shared<MoreOptions>();
   parser.add_arguments( pOpt );
   parser.add_arguments( pMoreOpt );

   auto res = parser.parse_args( { "-s", "str", "-n", "3274", "-S", "Str", "-N", "4723" } );
   EXPECT_EQ( "str", pOpt->str );
   EXPECT_EQ( 3274, pOpt->count );
   EXPECT_EQ( "Str", pMoreOpt->str );
   EXPECT_EQ( 4723, pMoreOpt->count );
}

TEST( ArgumentParserTest, shouldTakeLongOptionArgumentsWithEquals )
{
   double floatvalue = .0;
   long longvalue = 0;
   std::string strvalue;
   auto parser = argument_parser{};
   parser.add_argument( strvalue, "--string" ).nargs( 1 ).choices( { "alpha", "beta", "gamma" } );
   parser.add_argument( longvalue, "--long" ).nargs( 1 );
   parser.add_argument( floatvalue, "--float" ).nargs( 1 );

   auto res = parser.parse_args( { "--string=alpha", "--long=124", "--float=3.5" } );
   EXPECT_EQ( "alpha", strvalue );
   EXPECT_EQ( 124, longvalue );
   EXPECT_NEAR( 3.5, floatvalue, 1e-9 );
   EXPECT_EQ( 0, res.errors.size() );
}

TEST( ArgumentParserTest, shouldFailIfArgumentFollowsFlagWithEquals )
{
   std::string strvalue;
   auto parser = argument_parser{};
   parser.add_argument( strvalue, "--string" );

   auto res = parser.parse_args( { "--string=alpha" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_EQ( "1", strvalue );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "--string", res.errors.front().option );
   EXPECT_EQ( argument_parser::FLAG_PARAMETER, res.errors.front().errorCode );
}

// The program name must be set explicitly with parser.config().program().
TEST( ArgumentParserTest, shouldNotSetProgramNameFromParameter0 )
{
   std::string strvalue;
   long intvalue = 1;
   double floatvalue = 2.0;

   auto parser = argument_parser{};
   parser.add_argument( intvalue, "--int" ).nargs( 1 );
   parser.add_argument( floatvalue, "--float" ).nargs( 1 );
   parser.add_argument( strvalue, "params" ).nargs( 1 );

   auto res = parser.parse_args( { "testName", "--int", "2134", "--float", "32.4" } );

   EXPECT_TRUE( parser.getConfig().program.empty() );
   EXPECT_EQ( "testName", strvalue );
   ASSERT_EQ( 0, res.errors.size() );
}

TEST( ArgumentParserTest, shouldHaveHelpByDefault )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );

   // -- WHEN
   auto res = parser.parse_args( { "-h" } );

   // -- THEN
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::HELP_REQUESTED, res.errors[0].errorCode );

   // -- WHEN
   res = parser.parse_args( { "--help" } );

   // -- THEN
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::HELP_REQUESTED, res.errors[0].errorCode );
}

TEST( ArgumentParserTest, shouldNotAddDefaultHelpWhenDefined )
{
   std::optional<int> hide;
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );
   parser.add_argument( hide, "-h" );
   parser.add_default_help_option();

   // -- WHEN
   auto res = parser.parse_args( { "-h" } );

   // -- THEN
   EXPECT_TRUE( static_cast<bool>( res ) );
   EXPECT_EQ( 0, res.errors.size() );
   EXPECT_TRUE( hide.has_value() );

   // -- WHEN
   res = parser.parse_args( { "--help" } );

   // -- THEN
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::HELP_REQUESTED, res.errors[0].errorCode );
   EXPECT_FALSE( hide.has_value() );
}

TEST( ArgumentParserTest, shouldSetCustomHelpOptions )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );

   // -- WHEN
   parser.add_help_option( "-a", "--asistado" );

   // -- THEN
   auto res = parser.parse_args( { "-a" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::HELP_REQUESTED, res.errors[0].errorCode );

   res = parser.parse_args( { "--asistado" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::HELP_REQUESTED, res.errors[0].errorCode );

   res = parser.parse_args( { "-h" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::UNKNOWN_OPTION, res.errors[0].errorCode );

   res = parser.parse_args( { "--help" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::UNKNOWN_OPTION, res.errors[0].errorCode );
}

TEST( ArgumentParserTest, shouldSupportMultipleHelpOptions )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );

   // -- WHEN
   parser.add_default_help_option();
   parser.add_help_option( "-a", "--asistado" );
   parser.add_help_option( "--advice" );

   // -- THEN
   auto res = parser.parse_args( { "-a" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::HELP_REQUESTED, res.errors[0].errorCode );

   res = parser.parse_args( { "--asistado" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::HELP_REQUESTED, res.errors[0].errorCode );

   res = parser.parse_args( { "--advice" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::HELP_REQUESTED, res.errors[0].errorCode );

   res = parser.parse_args( { "-h" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::HELP_REQUESTED, res.errors[0].errorCode );

   res = parser.parse_args( { "--help" } );
   EXPECT_FALSE( static_cast<bool>( res ) );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::HELP_REQUESTED, res.errors[0].errorCode );
}

TEST( ArgumentParserTest, shouldThrowIfDefaultHelpOptionsCanNotBeSet )
{
   std::stringstream strout;
   int hide;
   auto parser = argument_parser{};
   parser.config().cout( strout );

   // -- WHEN
   parser.add_argument( hide, "-h", "--help" );

   // -- THEN
   EXPECT_THROW( parser.add_default_help_option(), std::invalid_argument );
}

TEST( ArgumentParserTest, shouldSetParserOutputToStream )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   EXPECT_EQ( nullptr, parser.getConfig().pOutStream );

   parser.config().cout( strout );

   // NOTE: EXPECT_NE fails to compile with MSVC 2017, 15.9.16
   EXPECT_TRUE( nullptr != parser.getConfig().pOutStream );
}

TEST( ArgumentParserTest, shouldWriteHelpAndExitWhenHelpOptionIsPresent )
{
   std::optional<int> maybeInt;

   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );
   parser.add_argument( maybeInt, "--maybe" ).nargs( 1 );
   parser.add_default_help_option().help( "Print this test help message and exit!" );

   auto res = parser.parse_args( { "--maybe", "123", "-h" } );
   EXPECT_FALSE( static_cast<bool>( res ) );

   auto text = strout.str();
   EXPECT_NE( std::string::npos, text.find( "Print this test help message and exit!" ) );

   EXPECT_FALSE( bool( maybeInt ) );
}

TEST( ArgumentParserTest, shouldResetValuesWhenCalledMultipleTimes )
{
   std::string first;
   std::string second;
   auto parser = argument_parser{};
   parser.add_argument( first, "--first" );
   parser.add_argument( second, "--second" );

   auto res = parser.parse_args( { "--first" } );
   EXPECT_NE( "", first );
   EXPECT_EQ( "", second );

   res = parser.parse_args( { "--second" } );
   EXPECT_EQ( "", first );
   EXPECT_NE( "", second );
}

TEST( ArgumentParserTest, shouldForbidDuplicateOptions )
{
   std::string first;
   std::string second;
   auto parser = argument_parser{};
   parser.add_argument( first, "--first" );
   EXPECT_THROW( parser.add_argument( second, "--first" ), argparse::DuplicateOption );
}

TEST( ArgumentParserTest, shouldForbidDuplicateHelpOptions )
{
   std::string first;
   auto parser = argument_parser{};
   parser.add_argument( first, "--aiuto" );
   EXPECT_THROW( parser.add_help_option( "--aiuto" ), argparse::DuplicateOption );
}

TEST( ArgumentParserTest, shouldForbidDuplicateHelpOptions2 )
{
   std::string first;
   auto parser = argument_parser{};
   parser.add_help_option( "--aiuto" );
   EXPECT_THROW( parser.add_argument( first, "--aiuto" ), argparse::DuplicateOption );
}

TEST( ArgumentParserTest, shouldSkipInitialArguments )
{
   std::optional<int> first;
   std::optional<int> second;
   auto parser = argument_parser{};
   parser.add_argument( first, "--first" );
   parser.add_argument( second, "--second" );

   parser.parse_args( { "--first", "--second" }, 0 );
   EXPECT_TRUE( first.has_value() );
   EXPECT_TRUE( second.has_value() );

   parser.parse_args( { "--first", "--second" }, 1 );
   EXPECT_FALSE( first.has_value() );
   EXPECT_TRUE( second.has_value() );

   parser.parse_args( { "--first", "--second" }, 2 );
   EXPECT_FALSE( first.has_value() );
   EXPECT_FALSE( second.has_value() );
}

TEST( ArgumentParserTest, shouldShowHelpWhenHasRequiredArgumentsAndNoneAreGiven )
{
   std::stringstream strout;
   auto parser = argument_parser{};
   parser.config().cout( strout );

   int num = 0;
   parser.add_argument( num, "--num" ).nargs( 1 ).required( true );
   EXPECT_TRUE( strout.str().empty() );

   auto res = parser.parse_args( {} );
   EXPECT_FALSE( static_cast<bool>( res ) );
   EXPECT_FALSE( strout.str().empty() );

   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( argument_parser::HELP_REQUESTED, res.errors[0].errorCode );
}
