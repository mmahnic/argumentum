#include "../src/argparser.h"

#include <gtest/gtest.h>
#include <algorithm>

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
   // NOTE: This can be changed with the flagValue() parameter option.
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
   EXPECT_EQ( CArgumentParser::MISSING_ARGUMENT, res.errors.front().errorCode );
}

TEST( CArgumentParserTest, shouldReportBadConversionError )
{
   std::optional<long> flagA;

   CArgumentParser parser;
   parser.addOption( flagA ).shortName( "a" ).hasArgument();

   auto res = parser.parseArguments( { "-a", "wrong" } );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "a", res.errors.front().option );
   EXPECT_EQ( CArgumentParser::CONVERSION_ERROR, res.errors.front().errorCode );
}

TEST( CArgumentParserTest, shouldReportUnknownOptionError )
{
   std::optional<long> flagA;

   CArgumentParser parser;
   parser.addOption( flagA ).shortName( "a" ).hasArgument();

   auto res = parser.parseArguments( { "-a", "2135", "--unknown" } );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "unknown", res.errors.front().option );
   EXPECT_EQ( CArgumentParser::UNKNOWN_OPTION, res.errors.front().errorCode );
}

TEST( CArgumentParserTest, shouldReportMissingRequiredOptionError )
{
   std::optional<long> flagA;
   std::optional<long> flagB;

   CArgumentParser parser;
   parser.addOption( flagA ).shortName( "a" ).hasArgument();
   parser.addOption( flagA ).shortName( "b" ).required();

   auto res = parser.parseArguments( { "-a", "2135" } );
   ASSERT_EQ( 1, res.errors.size() );
   EXPECT_EQ( "b", res.errors.front().option );
   EXPECT_EQ( CArgumentParser::MISSING_OPTION, res.errors.front().errorCode );
}

TEST( CArgumentParserTest, shouldSupportCustomOptionTypes )
{
   struct CustomType {
      std::string value;
      std::string reversed;
   };

   class CustomValue: public CArgumentParser::Value
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

   CArgumentParser parser;
   parser.addOption( CustomValue( custom ) ).shortName( "c" ).hasArgument();

   auto res = parser.parseArguments( { "-c", "value" } );
   EXPECT_EQ( "value", custom.value );
   EXPECT_EQ( "eulav", custom.reversed );
}

TEST( CArgumentParserTest, shouldSupportFlagValues )
{
   std::optional<std::string> flag;

   CArgumentParser parser;
   parser.addOption( flag ).shortName( "a" ).flagValue( "from-a" );
   parser.addOption( flag ).shortName( "b" ).flagValue( "from-b" );

   auto res = parser.parseArguments( { "-a", "-b" } );
   EXPECT_EQ( "from-b", flag.value() );

   flag = {};
   res = parser.parseArguments( { "-b", "-a" } );
   EXPECT_EQ( "from-a", flag.value() );
}
