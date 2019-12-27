#include "../testutil.h"

#include <argumentum/argparse.h>
#include <climits>
#include <gtest/gtest.h>
#include <numeric>
#include <vector>

using namespace std;
using namespace argumentum;
using namespace testutil;

// NOTE: Both command classes use the same definitions, but each uses a separate
// instance of these common options.
class SharedOptions : public argumentum::Options
{
public:
   vector<int> numbers;

public:
   void add_arguments( argument_parser& parser ) override
   {
      parser.add_argument( numbers, "N" ).minargs( 1 ).metavar( "INT" ).help( "Integers" );
   }
};

class CmdAccumulatorOptions : public argumentum::CommandOptions
{
public:
   std::shared_ptr<SharedOptions> common;
   std::pair<std::function<int( int, int )>, int> operation;

public:
   using CommandOptions::CommandOptions;

   void execute( const ParseResult& res ) override
   {
      const auto& numbers = common->numbers;
      auto acc = accumulate( numbers.begin(), numbers.end(), operation.second, operation.first );
      cout << acc << "\n";
   }

protected:
   void add_arguments( argument_parser& parser ) override
   {
      common = std::make_shared<SharedOptions>();
      parser.add_arguments( common );

      auto max = []( int a, int b ) { return std::max( a, b ); };
      auto sum = []( int a, int b ) { return a + b; };

      parser.add_argument( operation, "--sum", "-s" )
            .nargs( 0 )
            .absent( std::make_pair( max, INT_MIN ) )
            .action( [&]( auto& target, const std::string& value ) {
               target = std::make_pair( sum, 0 );
            } )
            .help( "Sum the integers (default: find the max)" );
   }
};

class CmdEchoOptions : public argumentum::CommandOptions
{
public:
   std::shared_ptr<SharedOptions> common;

public:
   using CommandOptions::CommandOptions;

   void add_arguments( argument_parser& parser ) override
   {
      common = std::make_shared<SharedOptions>();
      parser.add_arguments( common );
   };

   void execute( const ParseResult& res ) override
   {
      for ( auto n : common->numbers )
         cout << n << " ";
      cout << "\n";
   }
};

TEST( StaticLibrary, shouldAddNumbers )
{
   auto parser = argument_parser{};
   parser.config().program( "staticlib" ).description( "Accumulator" );
   parser.add_command<CmdAccumulatorOptions>( "fold" ).help( "Accumulate integer values." );
   parser.add_command<CmdEchoOptions>( "echo" ).help( "Echo integers from the command line." );

   auto res = parser.parse_args( { "fold", "1", "2", "3", "--sum" } );
   ASSERT_TRUE( !!res );
   ASSERT_EQ( 1, res.commands.size() );

   EXPECT_NE( nullptr, dynamic_cast<CmdAccumulatorOptions*>( res.commands[0].get() ) );
}

TEST( StaticLibrary, shouldEchoNumbers )
{
   auto parser = argument_parser{};
   parser.config().program( "staticlib" ).description( "Accumulator" );
   parser.add_command<CmdAccumulatorOptions>( "fold" ).help( "Accumulate integer values." );
   parser.add_command<CmdEchoOptions>( "echo" ).help( "Echo integers from the command line." );

   auto res = parser.parse_args( { "echo", "1", "2", "3" } );
   ASSERT_TRUE( !!res );
   ASSERT_EQ( 1, res.commands.size() );

   EXPECT_NE( nullptr, dynamic_cast<CmdEchoOptions*>( res.commands[0].get() ) );
}
