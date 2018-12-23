// Copyright (c) 2018 Marko Mahnič
// License: MIT. See LICENSE in the root of the project.

#include "../src/argparser.h"

#include <gtest/gtest.h>
#include <algorithm>

using namespace argparse;

namespace {

template<typename T>
bool vector_eq( const std::vector<T>& values, const std::vector<T>& var )
{
   auto dump = [&var]() {
      std::cout << "Result: ";
      for ( auto& v : var )
         std::cout << "'" << v << "'  ";
      std::cout << "\n";
   };

   if ( values.size() != var.size() ) {
      std::cout << "Size: " << values.size() << "!=" << var.size() << "\n";
      dump();
      return false;
   }

   for ( int i = 0; i < var.size(); ++i )
      if ( values[i] != var[i] ) {
         std::cout << "Value: '" << values[i] << "'!='" << var[i] << "'\n";
         dump();
         return false;
      }

   return true;
}

}   // namespace

TEST( ArgumentParserTest, shouldParseShortOptions )
{
   std::optional<std::string> value;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( value, "-v" ).nargs( 1 );
   parser.parseArguments( { "-v", "success" } );

   EXPECT_EQ( "success", value.value() );
}

TEST( ArgumentParserTest, shouldParseLongOptions )
{
   std::optional<std::string> value;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( value, "--value", "-v" ).nargs( 1 );
   parser.parseArguments( { "--value", "success" } );

   EXPECT_EQ( "success", value.value() );
}

TEST( ArgumentParserTest, shouldParseIntegerValues )
{
   std::optional<long> value;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( value, "-v", "--value" ).nargs( 1 );
   parser.parseArguments( { "--value", "2314" } );

   EXPECT_EQ( 2314, value.value() );
}

TEST( ArgumentParserTest, shouldNotSetOptionValuesWithoutArguments )
{
   std::optional<long> value;
   std::optional<std::string> unused;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( value, "-v", "--value" ).nargs( 1 );
   parser.addOption( unused, "--unused" );
   parser.parseArguments( { "--value", "2314" } );

   EXPECT_EQ( 2314, value.value() );
   EXPECT_FALSE( bool(unused) );
}

TEST( ArgumentParserTest, shouldOnlyAddOptionValueIfRequired )
{
   std::optional<long> value;
   std::optional<std::string> flag;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( value, "-v", "--value" ).nargs( 1 );
   parser.addOption( flag, "--flag" );

   parser.parseArguments( { "--value", "2314", "--flag", "notused" } );

   EXPECT_EQ( 2314, value.value() );

   // The parameters that do not require an argumet will be given the value "1".
   // NOTE: This can be changed with the flagValue() parameter option.
   EXPECT_EQ( "1", flag.value() );
}

TEST( ArgumentParserTest, shouldSkipParsingOptionsAfterDashDash )
{
   std::optional<long> value;
   std::optional<std::string> flag;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( value, "-v", "--value" ).nargs( 1 );
   parser.addOption( flag, "--skipped" );

   parser.parseArguments( { "--value", "2314", "--", "--skipped" } );

   EXPECT_EQ( 2314, value.value() );
   EXPECT_FALSE( bool(flag) );
}

TEST( ArgumentParserTest, shouldSupportShortOptionGroups )
{
   std::optional<long> flagA;
   std::optional<std::string> flagB;
   std::optional<std::string> flagC;
   std::optional<long> flagD;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( flagA, "-a" );
   parser.addOption( flagB, "-b" );
   parser.addOption( flagC, "-c" );
   parser.addOption( flagD, "-d" );

   parser.parseArguments( { "-abd" } );

   EXPECT_EQ( 1, flagA.value() );
   EXPECT_EQ( "1", flagB.value() );
   EXPECT_FALSE( bool(flagC) );
   EXPECT_EQ( 1, flagD.value() );
}

TEST( ArgumentParserTest, shouldReadArgumentForLastOptionInGroup )
{
   std::optional<long> flagA;
   std::optional<std::string> flagB;
   std::optional<std::string> flagC;
   std::optional<long> flagD;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( flagA, "-a" );
   parser.addOption( flagB, "-b" );
   parser.addOption( flagC, "-c" );
   parser.addOption( flagD, "-d" ).nargs( 1 );

   parser.parseArguments( { "-abd", "4213" } );

   EXPECT_EQ( 1, flagA.value() );
   EXPECT_EQ( "1", flagB.value() );
   EXPECT_FALSE( bool(flagC) );
   EXPECT_EQ( 4213, flagD.value() );
}

TEST( ArgumentParserTest, shouldReportErrorForMissingArgument )
{
   std::optional<long> flagA;
   std::optional<std::string> flagB;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( flagA, "-a" ).nargs( 1 );
   parser.addOption( flagB, "-b" );

   auto res = parser.parseArguments( { "-a", "-b", "freearg" } );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "a", res.errors.front().option );
   ASSERT_EQ( 1, res.ignoredArguments.size() );
   EXPECT_EQ( "freearg", res.ignoredArguments.front() );
   EXPECT_EQ( ArgumentParser::MISSING_ARGUMENT, res.errors.front().errorCode );
}

TEST( ArgumentParserTest, shouldReportBadConversionError )
{
   std::optional<long> flagA;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( flagA, "-a" ).nargs( 1 );

   auto res = parser.parseArguments( { "-a", "wrong" } );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "a", res.errors.front().option );
   EXPECT_EQ( ArgumentParser::CONVERSION_ERROR, res.errors.front().errorCode );
}

TEST( ArgumentParserTest, shouldReportUnknownOptionError )
{
   std::optional<long> flagA;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( flagA, "-a" ).nargs( 1 );

   auto res = parser.parseArguments( { "-a", "2135", "--unknown" } );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "unknown", res.errors.front().option );
   EXPECT_EQ( ArgumentParser::UNKNOWN_OPTION, res.errors.front().errorCode );
}

TEST( ArgumentParserTest, shouldReportMissingRequiredOptionError )
{
   std::optional<long> flagA;
   std::optional<long> flagB;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( flagA, "-a" ).nargs( 1 );
   parser.addOption( flagA, "-b" ).required();

   auto res = parser.parseArguments( { "-a", "2135" } );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "b", res.errors.front().option );
   EXPECT_EQ( ArgumentParser::MISSING_OPTION, res.errors.front().errorCode );
}

TEST( ArgumentParserTest, shouldSupportCustomOptionTypes )
{
   struct CustomType {
      std::string value;
      std::string reversed;
   };

   class CustomValue: public ArgumentParser::Value
   {
      CustomType& mValue;
   public:
      CustomValue( CustomType& value )
         : mValue( value )
      {}

   protected:
      void doSetValue( const std::string& value ) override
      {
         mValue.value = value;
         mValue.reversed = value;
         std::reverse( mValue.reversed.begin(), mValue.reversed.end() );
      }
   };

   CustomType custom;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( CustomValue( custom ), "-c" ).nargs( 1 );

   auto res = parser.parseArguments( { "-c", "value" } );
   EXPECT_EQ( "value", custom.value );
   EXPECT_EQ( "eulav", custom.reversed );
}

TEST( ArgumentParserTest, shouldSupportCustomOptionTypes_WithConvertedValue )
{
   struct CustomType {
      std::optional<std::string> value;
      std::string reversed;
   };

   class CustomValue: public ArgumentParser::ConvertedValue<CustomType>
   {
   public:
      CustomValue( CustomType& value )
         : ConvertedValue( value,
               []( const std::string& value ) {
                  CustomType custom;
                  custom.value = value;
                  custom.reversed = value;
                  std::reverse( custom.reversed.begin(), custom.reversed.end() );
                  return custom;
               } )
      {}
   };

   CustomType custom;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( CustomValue( custom ), "-c" ).nargs( 1 );

   auto res = parser.parseArguments( { "-c", "value" } );
   EXPECT_EQ( "value", custom.value.value() );
   EXPECT_EQ( "eulav", custom.reversed );
}

TEST( ArgumentParserTest, shouldSupportFlagValues )
{
   std::optional<std::string> flag;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( flag, "-a" ).flagValue( "from-a" );
   parser.addOption( flag, "-b" ).flagValue( "from-b" );

   auto res = parser.parseArguments( { "-a", "-b" } );
   EXPECT_EQ( "from-b", flag.value() );

   flag = {};
   res = parser.parseArguments( { "-b", "-a" } );
   EXPECT_EQ( "from-a", flag.value() );
}

TEST( ArgumentParserTest, shouldSupportFloatingPointValues )
{
   std::optional<double> value;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( value, "-a" ).nargs( 1 );

   auto res = parser.parseArguments( { "-a", "23.5" } );
   EXPECT_NEAR( 23.5, value.value(), 1e-9 );
}

TEST( ArgumentParserTest, shouldSupportRawValueTypes )
{
   std::string strvalue;
   long intvalue = 1;
   double floatvalue = 2.0;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( strvalue, "--str" ).nargs( 1 );
   parser.addOption( intvalue, "--int" ).nargs( 1 );
   parser.addOption( floatvalue, "--float" ).nargs( 1 );

   auto res = parser.parseArguments( { "--str", "string", "--int", "2134", "--float", "32.4" } );
   EXPECT_EQ( "string", strvalue );
   EXPECT_EQ( 2134, intvalue );
   EXPECT_NEAR( 32.4, floatvalue, 1e-9 );
}

TEST( ArgumentParserTest, shouldAcceptOptionNamesInConstructor )
{
   std::string strvalue;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( strvalue, "-s", "--string" ).nargs( 1 );

   auto res = parser.parseArguments( { "-s", "short" } );
   EXPECT_EQ( 0, res.errors.size() );
   EXPECT_EQ( "short", strvalue );

   res = parser.parseArguments( { "--string", "long" } );
   EXPECT_EQ( 0, res.errors.size() );
   EXPECT_EQ( "long", strvalue );
}

TEST( ArgumentParserTest, shouldNotAcceptInvalidShortOptions )
{
   std::string strvalue;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( strvalue, "-s", "--string" ).nargs( 1 );
   parser.addOption( strvalue, "--l" ).nargs( 1 );

   EXPECT_THROW( parser.addOption( strvalue, "-long" ).nargs( 1 ), std::invalid_argument );

   auto res = parser.parseArguments( { "-s", "short" } );
   EXPECT_EQ( 0, res.errors.size() );
   EXPECT_EQ( "short", strvalue );

   res = parser.parseArguments( { "--string", "long" } );
   EXPECT_EQ( 0, res.errors.size() );
   EXPECT_EQ( "long", strvalue );

   res = parser.parseArguments( { "--l", "onecharlong" } );
   EXPECT_EQ( 0, res.errors.size() );
   EXPECT_EQ( "onecharlong", strvalue );
}

TEST( ArgumentParserTest, shouldNotAcceptOptionsWithoutName )
{
   std::string strvalue;

   auto parser = ArgumentParser::unsafe();
   EXPECT_THROW( parser.addOption( strvalue, "-" ), std::invalid_argument );
   EXPECT_THROW( parser.addOption( strvalue, "--" ), std::invalid_argument );
   EXPECT_THROW( parser.addOption( strvalue, "" ), std::invalid_argument );
}

TEST( ArgumentParserTest, shouldSupportVectorOptions )
{
   std::vector<std::string> strings;
   std::vector<long> longs;
   std::vector<double> floats;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( strings, "-s" ).nargs( 1 );
   parser.addOption( longs, "-l" ).nargs( 1 );
   parser.addOption( floats, "-f" ).nargs( 1 );

   auto res = parser.parseArguments( { "-s", "string", "-f", "12.43", "-l", "576", "-l", "981" } );
   EXPECT_TRUE( vector_eq( { "string" }, strings ) );
   EXPECT_TRUE( vector_eq( { 12.43 }, floats ) );
   EXPECT_TRUE( vector_eq( { 576, 981 }, longs ) );
}

TEST( ArgumentParserTest, shouldStorePositionalArgumentsInValues )
{
   std::vector<std::string> strings;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( strings, "text" ).minargs( 0 );

   auto res = parser.parseArguments( { "one", "two", "three" } );

   EXPECT_TRUE( vector_eq( { "one", "two", "three" }, strings ) );
}

TEST( ArgumentParserTest, shouldGroupPositionalArguments )
{
   std::string strvalue;
   std::string firstArgument;
   std::vector<std::string> otherArguments;

   auto makeParser = [&]() {
      auto parser = ArgumentParser::unsafe();
      parser.addOption( strvalue, "-s", "--string" ).nargs( 1 );
      parser.addOption( strvalue, "--l" ).nargs( 1 );
      parser.addOption( firstArgument, "text" ).nargs( 1 );
      parser.addOption( otherArguments, "args" ).minargs( 0 );
      return parser;
   };

   auto parser = makeParser();
   auto res = parser.parseArguments( { "-s", "string", "first", "second", "third" } );
   EXPECT_EQ( "first", firstArgument );
   EXPECT_TRUE( vector_eq( { "second", "third" }, otherArguments ) );

   parser = makeParser();
   firstArgument.clear();
   otherArguments.clear();
   res = parser.parseArguments( { "first", "second", "-s", "string", "third" } );
   EXPECT_EQ( "first", firstArgument );
   EXPECT_TRUE( vector_eq( { "second", "third" }, otherArguments ) );
}

TEST( ArgumentParserTest, shouldSupportOptionArgumentCounts )
{
   std::string strvalue;
   std::vector<std::string> texts;
   std::vector<std::string> files;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( strvalue, "-s" ).nargs( 1 );
   parser.addOption( texts, "-t" ).nargs( 2 );
   parser.addOption( files, "-f" ).minargs( 0 );

   parser.parseArguments( { "-t", "the", "text", "-f", "file1", "file2", "file3", "-s", "string" } );
   EXPECT_EQ( "string", strvalue );
   EXPECT_TRUE( vector_eq( { "the", "text" }, texts ) );
   EXPECT_TRUE( vector_eq( { "file1", "file2", "file3" }, files ) );
}

TEST( ArgumentParserTest, shouldFailWhenOptionArgumentCountsAreWrong )
{
   std::string strvalue;
   std::vector<std::string> texts;
   std::vector<std::string> files;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( strvalue, "-s" ).nargs( 1 );
   parser.addOption( texts, "-t" ).nargs( 2 );
   parser.addOption( files, "-f" ).nargs( 2 );

   auto res = parser.parseArguments( { "-t", "the", "-f", "file1", "file2",
      "not-file3", "-s", "string" } );
   EXPECT_EQ( "string", strvalue );
   EXPECT_TRUE( vector_eq( { "the" }, texts ) );
   EXPECT_TRUE( vector_eq( { "file1", "file2" }, files ) );

   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "t", res.errors.front().option );
   EXPECT_EQ( ArgumentParser::MISSING_ARGUMENT, res.errors.front().errorCode );

   EXPECT_TRUE( vector_eq( { "not-file3" }, res.ignoredArguments ) );
}

TEST( ArgumentParserTest, shouldSupportPositionalArgumentCounts )
{
   std::string strvalue;
   std::vector<std::string> texts;
   std::vector<std::string> files;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( strvalue, "-s" ).nargs( 1 );
   parser.addOption( texts, "text" ).nargs( 2 );
   parser.addOption( files, "file" ).nargs( 2 );
   auto res = parser.parseArguments( { "the", "-s", "string1", "text", "file1", "file2",
      "not-file3", "-s", "string2" } );

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
      auto parser = ArgumentParser::unsafe();
      parser.addOption( texts, "-t" ).nargs( nargs );
      return parser.parseArguments( params );
   };
   auto params = std::vector<std::string>{ "-t", "read", "the", "text" };

   auto res = testWithNargs( 0, params );
   // When an option doesn't accept arguments, the default value is set/added
   EXPECT_TRUE( vector_eq( { "1" }, texts ) );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithNargs( 1, params );
   EXPECT_TRUE( vector_eq( { "read" }, texts ) );
   EXPECT_TRUE( vector_eq( { "the", "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithNargs( 2, params );
   EXPECT_TRUE( vector_eq( { "read", "the" }, texts ) );
   EXPECT_TRUE( vector_eq( { "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithNargs( 3, params );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) );
   EXPECT_EQ( 0, res.ignoredArguments.size() );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithNargs( 4, params );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) );
   EXPECT_EQ( 0, res.ignoredArguments.size() );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "t", res.errors[0].option );
   EXPECT_EQ( ArgumentParser::MISSING_ARGUMENT, res.errors[0].errorCode );
}

TEST( ArgumentParserTest, shouldSupportExactNumberOfPositionalArguments )
{
   std::vector<std::string> texts;

   auto testWithNargs = [&]( int nargs, const std::vector<std::string>& params ) {
      texts.clear();
      auto parser = ArgumentParser::unsafe();
      parser.addOption( texts, "text" ).nargs( nargs );
      return parser.parseArguments( params );
   };
   auto params = std::vector<std::string>{ "read", "the", "text" };

   auto res = testWithNargs( 0, params );
   EXPECT_EQ( 0, texts.size() );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithNargs( 1, params );
   EXPECT_TRUE( vector_eq( { "read" }, texts ) );
   EXPECT_TRUE( vector_eq( { "the", "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithNargs( 2, params );
   EXPECT_TRUE( vector_eq( { "read", "the" }, texts ) );
   EXPECT_TRUE( vector_eq( { "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithNargs( 3, params );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) );
   EXPECT_EQ( 0, res.ignoredArguments.size() );
   EXPECT_EQ( 0, res.errors.size() );

   res = testWithNargs( 4, params );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) );
   EXPECT_EQ( 0, res.ignoredArguments.size() );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "text", res.errors[0].option );
   EXPECT_EQ( ArgumentParser::MISSING_ARGUMENT, res.errors[0].errorCode );
}

TEST( ArgumentParserTest, shouldSupportMinNumberOfOptionArguments )
{
   std::vector<std::string> texts;

   auto testWithMinArgs = [&]( int nargs, const std::vector<std::string>& params ) {
      texts.clear();
      auto parser = ArgumentParser::unsafe();
      parser.addOption( texts, "-t" ).minargs( nargs );
      return parser.parseArguments( params );
   };
   auto params = std::vector<std::string>{ "-t", "read", "the", "text" };

   for ( int nargs = 0; nargs < 4; ++nargs ) {
      auto res = testWithMinArgs( nargs, params );
      EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) ) << "minargs:" << nargs;
      EXPECT_EQ( 0, res.ignoredArguments.size() ) << "minargs:" << nargs;
      EXPECT_EQ( 0, res.errors.size() ) << "minargs:" << nargs;
   }

   auto res = testWithMinArgs( 4, params );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) );
   EXPECT_EQ( 0, res.ignoredArguments.size() );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "t", res.errors[0].option );
   EXPECT_EQ( ArgumentParser::MISSING_ARGUMENT, res.errors[0].errorCode );
}

TEST( ArgumentParserTest, shouldSupportMaxNumberOfOptionArguments )
{
   std::vector<std::string> texts;

   auto testWithMaxArgs = [&]( int nargs, const std::vector<std::string>& params ) {
      texts.clear();
      auto parser = ArgumentParser::unsafe();
      parser.addOption( texts, "-t" ).maxargs( nargs );
      return parser.parseArguments( params );
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
      auto parser = ArgumentParser::unsafe();
      parser.addOption( texts, "text" ).minargs( nargs );
      return parser.parseArguments( params );
   };
   auto params = std::vector<std::string>{ "read", "the", "text" };

   for ( int nargs = 0; nargs < 4; ++nargs ) {
      auto res = testWithMinArgs( nargs, params );
      EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) ) << "maxargs:" << nargs;
      EXPECT_EQ( 0, res.errors.size() ) << "maxargs:" << nargs;
      EXPECT_EQ( 0, res.ignoredArguments.size() ) << "maxargs:" << nargs;
   }

   auto res = testWithMinArgs( 4, params );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) );
   EXPECT_EQ( 0, res.ignoredArguments.size() );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "text", res.errors[0].option );
   EXPECT_EQ( ArgumentParser::MISSING_ARGUMENT, res.errors[0].errorCode );
}

TEST( ArgumentParserTest, shouldSupportMaxNumberOfPositionalArguments )
{
   std::vector<std::string> texts;

   auto testWithMaxArgs = [&]( int nargs, const std::vector<std::string>& params ) {
      texts.clear();
      auto parser = ArgumentParser::unsafe();
      parser.addOption( texts, "text" ).maxargs( nargs );
      return parser.parseArguments( params );
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

   auto parser = ArgumentParser::unsafe();
   // If the value variable is a vector, the default is minargs(0)
   parser.addOption( texts, "text" );

   auto res = parser.parseArguments( params );
   EXPECT_TRUE( vector_eq( { "read", "the", "text" }, texts ) );
   EXPECT_EQ( 0, res.ignoredArguments.size() );
   EXPECT_EQ( 0, res.errors.size() );
}

TEST( ArgumentParserTest, shouldSetDefaultCountForPositionalArgumentsWithScalarValues )
{
   std::string strvalue;
   auto params = std::vector<std::string>{ "read", "the", "text" };

   auto parser = ArgumentParser::unsafe();
   // If the value variable is a scalar, the default is nargs(1)
   parser.addOption( strvalue, "text" );

   auto res = parser.parseArguments( params );
   EXPECT_EQ( "read", strvalue );
   EXPECT_TRUE( vector_eq( { "the", "text" }, res.ignoredArguments ) );
   EXPECT_EQ( 0, res.errors.size() );
}

TEST( ArgumentParserTest, shouldSetFlagValueWhenZeroOrMoreArgumentsRequiredAndNoneGiven )
{
   std::vector<std::string> texts;
   auto parser = ArgumentParser::unsafe();
   parser.addOption( texts, "-t" ).maxargs( 1 );

   auto res = parser.parseArguments( { "-t" } );
   EXPECT_TRUE( vector_eq( { "1" }, texts ) );
}

TEST( ArgumentParserTest, shouldSetArgumentCountAtMostOnce )
{
   std::vector<std::string> texts;
   auto parser = ArgumentParser::unsafe();

   EXPECT_NO_THROW( parser.addOption( texts, "-a" ).nargs( 1 ) );
   EXPECT_NO_THROW( parser.addOption( texts, "-b" ).minargs( 1 ) );
   EXPECT_NO_THROW( parser.addOption( texts, "-c" ).maxargs( 1 ) );

   EXPECT_THROW( parser.addOption( texts, "-d" ).nargs( 1 ).minargs( 1 ), std::invalid_argument );
   EXPECT_THROW( parser.addOption( texts, "-e" ).nargs( 1 ).maxargs( 1 ), std::invalid_argument );
   EXPECT_THROW( parser.addOption( texts, "-f" ).minargs( 1 ).nargs( 1 ), std::invalid_argument );
   EXPECT_THROW( parser.addOption( texts, "-g" ).minargs( 1 ).maxargs( 1 ), std::invalid_argument );
   EXPECT_THROW( parser.addOption( texts, "-h" ).maxargs( 1 ).nargs( 1 ), std::invalid_argument );
   EXPECT_THROW( parser.addOption( texts, "-i" ).maxargs( 1 ).minargs( 1 ), std::invalid_argument );
}

TEST( ArgumentParserTest, shouldSetOptionChoices )
{
   std::string strvalue;
   auto parser = ArgumentParser::unsafe();

   parser.addOption( strvalue, "-s" ).nargs( 1 ).choices( { "alpha", "beta", "gamma" } );
   auto res = parser.parseArguments( { "-s", "beta" } );
   EXPECT_EQ( "beta", strvalue );
   EXPECT_EQ( 0, res.errors.size() );
}

TEST( ArgumentParserTest, shouldFailIfArgumentIsNotInChoices )
{
   std::string strvalue;
   auto parser = ArgumentParser::unsafe();

   parser.addOption( strvalue, "-s" ).nargs( 1 ).choices( { "alpha", "beta", "gamma" } );
   auto res = parser.parseArguments( { "-s", "phi" } );
   EXPECT_TRUE( strvalue.empty() );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "s", res.errors[0].option );
   EXPECT_EQ( ArgumentParser::INVALID_CHOICE, res.errors[0].errorCode );
}

TEST( ArgumentParserTest, shouldFailIfPositionalArgumentIsNotInChoices )
{
   std::string strvalue;
   auto parser = ArgumentParser::unsafe();

   parser.addOption( strvalue, "string" ).nargs( 1 ).choices( { "alpha", "beta", "gamma" } );
   auto res = parser.parseArguments( { "phi" } );
   EXPECT_TRUE( strvalue.empty() );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "string", res.errors[0].option );
   EXPECT_EQ( ArgumentParser::INVALID_CHOICE, res.errors[0].errorCode );
}
