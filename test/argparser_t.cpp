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

};

TEST( ArgumentParserTest, shouldParseShortOptions )
{
   std::optional<std::string> value;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( value, "-v" ).hasArgument();
   parser.parseArguments( { "-v", "success" } );

   EXPECT_EQ( "success", value.value() );
}

TEST( ArgumentParserTest, shouldParseLongOptions )
{
   std::optional<std::string> value;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( value, "--value", "-v" ).hasArgument();
   parser.parseArguments( { "--value", "success" } );

   EXPECT_EQ( "success", value.value() );
}

TEST( ArgumentParserTest, shouldParseIntegerValues )
{
   std::optional<long> value;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( value, "-v", "--value" ).hasArgument();
   parser.parseArguments( { "--value", "2314" } );

   EXPECT_EQ( 2314, value.value() );
}

TEST( ArgumentParserTest, shouldNotSetOptionValuesWithoutArguments )
{
   std::optional<long> value;
   std::optional<std::string> unused;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( value, "-v", "--value" ).hasArgument();
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
   parser.addOption( value, "-v", "--value" ).hasArgument();
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
   parser.addOption( value, "-v", "--value" ).hasArgument();
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
   parser.addOption( flagD, "-d" ).hasArgument();

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
   parser.addOption( flagA, "-a" ).hasArgument();
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
   parser.addOption( flagA, "-a" ).hasArgument();

   auto res = parser.parseArguments( { "-a", "wrong" } );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "a", res.errors.front().option );
   EXPECT_EQ( ArgumentParser::CONVERSION_ERROR, res.errors.front().errorCode );
}

TEST( ArgumentParserTest, shouldReportUnknownOptionError )
{
   std::optional<long> flagA;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( flagA, "-a" ).hasArgument();

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
   parser.addOption( flagA, "-a" ).hasArgument();
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
   parser.addOption( CustomValue( custom ), "-c" ).hasArgument();

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
   parser.addOption( CustomValue( custom ), "-c" ).hasArgument();

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
   parser.addOption( value, "-a" ).hasArgument();

   auto res = parser.parseArguments( { "-a", "23.5" } );
   EXPECT_NEAR( 23.5, value.value(), 1e-9 );
}

TEST( ArgumentParserTest, shouldSupportRawValueTypes )
{
   std::string strvalue;
   long intvalue = 1;
   double floatvalue = 2.0;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( strvalue, "--str" ).hasArgument();
   parser.addOption( intvalue, "--int" ).hasArgument();
   parser.addOption( floatvalue, "--float" ).hasArgument();

   auto res = parser.parseArguments( { "--str", "string", "--int", "2134", "--float", "32.4" } );
   EXPECT_EQ( "string", strvalue );
   EXPECT_EQ( 2134, intvalue );
   EXPECT_NEAR( 32.4, floatvalue, 1e-9 );
}

TEST( ArgumentParserTest, shouldAcceptOptionNamesInConstructor )
{
   std::string strvalue;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( strvalue, "-s", "--string" ).hasArgument();

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
   parser.addOption( strvalue, "-s", "--string" ).hasArgument();
   parser.addOption( strvalue, "--l" ).hasArgument();

   EXPECT_THROW( parser.addOption( strvalue, "-long" ).hasArgument(), std::invalid_argument );

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
   parser.addOption( strings, "-s" ).hasArgument();
   parser.addOption( longs, "-l" ).hasArgument();
   parser.addOption( floats, "-f" ).hasArgument();

   auto res = parser.parseArguments( { "-s", "string", "-f", "12.43", "-l", "576", "-l", "981" } );
   ASSERT_EQ( 1, strings.size() );
   EXPECT_EQ( "string", strings.front() );
   ASSERT_EQ( 1, floats.size() );
   EXPECT_NEAR( 12.43, floats.front(), 1e-9 );
   ASSERT_EQ( 2, longs.size() );
   EXPECT_EQ( 576, longs[0] );
   EXPECT_EQ( 981, longs[1] );
}

TEST( ArgumentParserTest, shouldStorePositionalArgumentsInValues )
{
   std::vector<std::string> strings;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( strings, "text" ).nargs( -1 );

   auto res = parser.parseArguments( { "one", "two", "three" } );

   ASSERT_EQ( 3, strings.size() );
   EXPECT_EQ( "one", strings[0] );
   EXPECT_EQ( "two", strings[1] );
   EXPECT_EQ( "three", strings[2] );
}

TEST( ArgumentParserTest, shouldGroupPositionalArguments )
{
   std::string strvalue;
   std::string firstArgument;
   std::vector<std::string> otherArguments;

   auto makeParser = [&]() {
      auto parser = ArgumentParser::unsafe();
      parser.addOption( strvalue, "-s", "--string" ).hasArgument();
      parser.addOption( strvalue, "--l" ).hasArgument();
      parser.addOption( firstArgument, "text" ).nargs( 1 );
      parser.addOption( otherArguments, "args" ).nargs( -1 );
      return parser;
   };

   auto parser = makeParser();
   auto res = parser.parseArguments( { "-s", "string", "first", "second", "third" } );
   EXPECT_EQ( "first", firstArgument );
   ASSERT_EQ( 2, otherArguments.size() );
   EXPECT_EQ( "second", otherArguments[0] );
   EXPECT_EQ( "third", otherArguments[1] );

   parser = makeParser();
   firstArgument.clear();
   otherArguments.clear();
   res = parser.parseArguments( { "first", "second", "-s", "string", "third" } );
   EXPECT_EQ( "first", firstArgument );
   ASSERT_EQ( 2, otherArguments.size() );
   EXPECT_EQ( "second", otherArguments[0] );
   EXPECT_EQ( "third", otherArguments[1] );
}

TEST( ArgumentParserTest, shouldSupportOptionArgumentCounts )
{
   std::string strvalue;
   std::vector<std::string> texts;
   std::vector<std::string> files;

   auto parser = ArgumentParser::unsafe();
   parser.addOption( strvalue, "-s" ).nargs( 1 );
   parser.addOption( texts, "-t" ).nargs( 2 );
   parser.addOption( files, "-f" ).nargs( -1 );

   parser.parseArguments( { "-t", "the", "text", "-f", "file1", "file2", "file3", "-s", "string" } );
   EXPECT_EQ( "string", strvalue );
   ASSERT_EQ( 2, texts.size() );
   EXPECT_EQ( "the", texts[0] );
   EXPECT_EQ( "text", texts[1] );
   ASSERT_EQ( 3, files.size() );
   EXPECT_EQ( "file1", files[0] );
   EXPECT_EQ( "file2", files[1] );
   EXPECT_EQ( "file3", files[2] );
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
   ASSERT_EQ( 1, texts.size() );
   EXPECT_EQ( "the", texts[0] );
   ASSERT_EQ( 2, files.size() );
   EXPECT_EQ( "file1", files[0] );
   EXPECT_EQ( "file2", files[1] );

   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "t", res.errors.front().option );
   EXPECT_EQ( ArgumentParser::MISSING_ARGUMENT, res.errors.front().errorCode );

   ASSERT_EQ( 1, res.ignoredArguments.size() );
   EXPECT_EQ( "not-file3", res.ignoredArguments[0] );
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
