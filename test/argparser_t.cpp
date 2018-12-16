// Copyright (c) 2018 Marko Mahnič
// License: MIT. See LICENSE in the root of the project.

#include "../src/argparser.h"

#include <gtest/gtest.h>
#include <algorithm>

using namespace argparse;

TEST( ArgumentParserTest, shouldParseShortOptions )
{
   std::optional<std::string> value;

   ArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).hasArgument();
   parser.parseArguments( { "-v", "success" } );

   EXPECT_EQ( "success", value.value() );
}

TEST( ArgumentParserTest, shouldParseLongOptions )
{
   std::optional<std::string> value;

   ArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).longName( "value" ).hasArgument();
   parser.parseArguments( { "--value", "success" } );

   EXPECT_EQ( "success", value.value() );
}

TEST( ArgumentParserTest, shouldParseIntegerValues )
{
   std::optional<long> value;

   ArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).longName( "value" ).hasArgument();
   parser.parseArguments( { "--value", "2314" } );

   EXPECT_EQ( 2314, value.value() );
}

TEST( ArgumentParserTest, shouldNotSetOptionValuesWithoutArguments )
{
   std::optional<long> value;
   std::optional<std::string> unused;

   ArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).longName( "value" ).hasArgument();
   parser.addOption( unused ).longName( "unused" );
   parser.parseArguments( { "--value", "2314" } );

   EXPECT_EQ( 2314, value.value() );
   EXPECT_FALSE( bool(unused) );
}

TEST( ArgumentParserTest, shouldOnlyAddOptionValueIfRequired )
{
   std::optional<long> value;
   std::optional<std::string> flag;

   ArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).longName( "value" ).hasArgument();
   parser.addOption( flag ).longName( "flag" );

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

   ArgumentParser parser;
   parser.addOption( value ).shortName( "v" ).longName( "value" ).hasArgument();
   parser.addOption( flag ).longName( "skipped" );

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

   ArgumentParser parser;
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

TEST( ArgumentParserTest, shouldReadArgumentForLastOptionInGroup )
{
   std::optional<long> flagA;
   std::optional<std::string> flagB;
   std::optional<std::string> flagC;
   std::optional<long> flagD;

   ArgumentParser parser;
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

TEST( ArgumentParserTest, shouldReportErrorForMissingArgument )
{
   std::optional<long> flagA;
   std::optional<std::string> flagB;

   ArgumentParser parser;
   parser.addOption( flagA ).shortName( "a" ).hasArgument();
   parser.addOption( flagB ).shortName( "b" );

   auto res = parser.parseArguments( { "-a", "-b", "freearg" } );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "a", res.errors.front().option );
   ASSERT_EQ( 1, res.freeArguments.size() );
   EXPECT_EQ( "freearg", res.freeArguments.front() );
   EXPECT_EQ( ArgumentParser::MISSING_ARGUMENT, res.errors.front().errorCode );
}

TEST( ArgumentParserTest, shouldReportBadConversionError )
{
   std::optional<long> flagA;

   ArgumentParser parser;
   parser.addOption( flagA ).shortName( "a" ).hasArgument();

   auto res = parser.parseArguments( { "-a", "wrong" } );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "a", res.errors.front().option );
   EXPECT_EQ( ArgumentParser::CONVERSION_ERROR, res.errors.front().errorCode );
}

TEST( ArgumentParserTest, shouldReportUnknownOptionError )
{
   std::optional<long> flagA;

   ArgumentParser parser;
   parser.addOption( flagA ).shortName( "a" ).hasArgument();

   auto res = parser.parseArguments( { "-a", "2135", "--unknown" } );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "unknown", res.errors.front().option );
   EXPECT_EQ( ArgumentParser::UNKNOWN_OPTION, res.errors.front().errorCode );
}

TEST( ArgumentParserTest, shouldReportMissingRequiredOptionError )
{
   std::optional<long> flagA;
   std::optional<long> flagB;

   ArgumentParser parser;
   parser.addOption( flagA ).shortName( "a" ).hasArgument();
   parser.addOption( flagA ).shortName( "b" ).required();

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

   ArgumentParser parser;
   parser.addOption( CustomValue( custom ) ).shortName( "c" ).hasArgument();

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

   ArgumentParser parser;
   parser.addOption( CustomValue( custom ) ).shortName( "c" ).hasArgument();

   auto res = parser.parseArguments( { "-c", "value" } );
   EXPECT_EQ( "value", custom.value.value() );
   EXPECT_EQ( "eulav", custom.reversed );
}

TEST( ArgumentParserTest, shouldSupportFlagValues )
{
   std::optional<std::string> flag;

   ArgumentParser parser;
   parser.addOption( flag ).shortName( "a" ).flagValue( "from-a" );
   parser.addOption( flag ).shortName( "b" ).flagValue( "from-b" );

   auto res = parser.parseArguments( { "-a", "-b" } );
   EXPECT_EQ( "from-b", flag.value() );

   flag = {};
   res = parser.parseArguments( { "-b", "-a" } );
   EXPECT_EQ( "from-a", flag.value() );
}

TEST( ArgumentParserTest, shouldSupportFloatingPointValues )
{
   std::optional<double> value;

   ArgumentParser parser;
   parser.addOption( value ).shortName( "a" ).hasArgument();

   auto res = parser.parseArguments( { "-a", "23.5" } );
   EXPECT_NEAR( 23.5, value.value(), 1e-9 );
}

TEST( ArgumentParserTest, shouldSupportRawValueTypes )
{
   std::string strvalue;
   long intvalue = 1;
   double floatvalue = 2.0;

   ArgumentParser parser;
   parser.addOption( strvalue ).longName( "str" ).hasArgument();
   parser.addOption( intvalue ).longName( "int" ).hasArgument();
   parser.addOption( floatvalue ).longName( "float" ).hasArgument();

   auto res = parser.parseArguments( { "--str", "string", "--int", "2134", "--float", "32.4" } );
   EXPECT_EQ( "string", strvalue );
   EXPECT_EQ( 2134, intvalue );
   EXPECT_NEAR( 32.4, floatvalue, 1e-9 );
}
