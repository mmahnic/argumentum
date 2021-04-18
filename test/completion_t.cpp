// Copyright (c) 2021 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include <argumentum/argparse.h>

#include <gtest/gtest.h>

using namespace argumentum;

TEST( Completion, shouldReturnOptionList )
{
   std::string str;
   long depth;

   auto parser = argument_parser{};
#if 1
   std::stringstream strout;
   parser.config().cout( strout );
#else
   // TODO: normal output and completion output are separate settings
   std::vector<std::string> completions;
   parser.config().complete_into( completions );
#endif

   auto params = parser.params();
   params.add_parameter( str, "-s" ).nargs( 1 ).help( "some string" );
   params.add_parameter( depth, "-d", "--depth" ).nargs( 1 ).help( "some depth" );
   params.add_help_option( "-h" );

   // When ---complete is present at the end, completion starts.
   // Parsing is done without setting target values.
   // Errors are ignored.
   // Completions are written to stdout.  --- Option to write to a file.
   // The position of the cursor must be specified by the caller.  If it is not
   // specified, completion is done at the end.
   // Completion can complete a new word or extend the preceeding word.
   //   --complete-new=1      complete a new word after first word
   //   --complete-extend=1   complete by extending the first word
   auto res = parser.parse_args( { "-", "---complete-extend=1" } );

   // Completions make the result invalid so that the program can not continue.
   EXPECT_FALSE( res );
#if 1
   auto pos = strout.str().find( "-s\n-d\n--depth\n-h" );
   EXPECT_NE( std::string::npos, pos );
#else
   ASSERT_EQ( 2, completions.size() );
   EXPECT_EQ( "-s", completions[0] );
   EXPECT_EQ( "-d", completions[0] );
   EXPECT_EQ( "--depth", completions[1] );
#endif
}

TEST( Completion, shouldCompleteOptionWithPrefix )
{
   auto parser = argument_parser{};
   std::stringstream strout;
   parser.config().cout( strout );

   auto params = parser.params();
   int dummy;
   params.add_parameter( dummy, "--sin" ).nargs( 1 );
   params.add_parameter( dummy, "--sum" ).nargs( 1 );
   params.add_parameter( dummy, "--depth" ).nargs( 1 );
   params.add_parameter( dummy, "--done" ).nargs( 1 );
   params.add_help_option( "-h" );

   auto res = parser.parse_args( { "--depth", "--s", "--done", "---complete-extend=2" } );

   EXPECT_FALSE( res );
   auto pos = strout.str().find( "--sin\n--sum\n" );
   EXPECT_NE( std::string::npos, pos );
   EXPECT_EQ( std::string::npos, strout.str().find( "--depth" ) );
   EXPECT_EQ( std::string::npos, strout.str().find( "--done" ) );
}

TEST( CompletionParams, shouldSplitCompletionAndNormalArguments )
{
   CompletionParams completion;

   std::vector<std::string> args{ "some", "-a", "--normal", "argument", "---complete-extend=5",
      "between", "---complete-new=3", "-and", "after" };

   completion.splitArguments( args.begin(), args.end() );

   ASSERT_EQ( 2, completion.completeArgs.size() );
   EXPECT_EQ( "--complete-extend=5", completion.completeArgs[0] );
   EXPECT_EQ( "--complete-new=3", completion.completeArgs[1] );
   ASSERT_EQ( 7, completion.programArgs.size() );
}

TEST( CompletionParams, shouldParseCompletionArguments )
{
   CompletionParams completion;

   std::vector<std::string> args{ "some", "-a", "--normal", "argument", "---complete-extend=5",
      "between", "-and", "after" };

   completion.splitArguments( args.begin(), args.end() );
   completion.parseCompletionArguments();

   ASSERT_EQ( 1, completion.completeArgs.size() );
   EXPECT_EQ( "--complete-extend=5", completion.completeArgs[0] );
   ASSERT_EQ( 7, completion.programArgs.size() );

   EXPECT_EQ( 4, completion.argumentIndex );
   EXPECT_GT( 0, completion.byteOffset );
   EXPECT_FALSE( completion.isNewArgument );
}

TEST( CompletionParams, shouldGetCompletionPrefix )
{
   std::vector<std::string> args{ "some", "-a", "--normal", "argument" };
   struct Example
   {
      std::vector<std::string> args;
      std::string completeOption;
      std::string result;
   };

   const std::vector<Example> examples{ // (clf)
      { args, "---complete-extend=1", "some" },   // (clf)
      { args, "---complete-extend=3", "--normal" },   // (clf)
      { args, "---complete-extend", "argument" },   // (clf)
      { args, "---complete-extend=9", "" },   // (clf)
      { args, "---complete-new=1", "" }
   };

   for ( const auto& ex : examples ) {
      auto args = ex.args;
      args.push_back( ex.completeOption );

      CompletionParams completion;
      completion.splitArguments( args.begin(), args.end() );
      completion.parseCompletionArguments();

      EXPECT_EQ( ex.result, completion.getPrefix() );
   }
}

#if 0
TEST( Completion, shouldRedirectCompletions )
{
   std::vector<std::string> completions;
   auto parser = argument_parser{};
   parser.config().complete_into( completions );
}
#endif

#if 0
TEST( Completion, shouldCompleteShortOptions )
{
   std::string str;
   long depth;
   std::vector<std::string> completions;

   auto parser = argument_parser{};
   parser.config().complete_into( completions );

   auto params = parser.params();
   params.add_parameter( str, "-s" ).nargs( 1 ).help( "some string" );
   params.add_parameter( depth, "-d", "--depth" ).nargs( 1 ).help( "some depth" );

   // This serves more like help (what short options are available) since the
   // uset has to press the desired letter, anyway.
   auto res = parser.parse_args( { "-s", "---complete-extend=1" } );

   EXPECT_FALSE( res );
   ASSERT_EQ( 1, completions.size() );
   EXPECT_EQ( "-sd", completions[0] );
}
#endif
