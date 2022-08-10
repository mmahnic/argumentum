import std.core;
import Argumentum;

using namespace std;
using namespace argumentum;

// NOTE: Both command classes use the same definitions, but each uses a separate
// instance of these common options.
class SharedOptions : public argumentum::Options
{
public:
   vector<int> numbers;

public:
   void add_arguments( argument_parser& parser ) override
   {
      auto params = parser.params();
      params.add_parameter( numbers, "N" ).minargs( 1 ).metavar( "INT" ).help( "Integers" );
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
      auto params = parser.params();
      common = std::make_shared<SharedOptions>();
      params.add_parameters( common );

      auto max = []( int a, int b ) {
         return std::max( a, b );
      };
      auto sum = []( int a, int b ) {
         return a + b;
      };

      params.add_parameter( operation, "--sum", "-s" )
            .nargs( 0 )
            .absent( std::make_pair( max, std::numeric_limits<int>::min() ) )
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
      auto params = parser.params();
      common = std::make_shared<SharedOptions>();
      params.add_parameters( common );
   };

   void execute( const ParseResult& res ) override
   {
      for ( auto n : common->numbers )
         cout << n << " ";
      cout << "\n";
   }
};

int main( int argc, char** argv )
{
   auto parser = argument_parser{};
   parser.config().program( argv[0] ).description( "Accumulator" );
   auto params = parser.params();
   params.add_command<CmdAccumulatorOptions>( "fold" ).help( "Accumulate integer values." );
   params.add_command<CmdEchoOptions>( "echo" ).help( "Echo integers from the command line." );

   auto res = parser.parse_args( argc, argv, 1 );
   if ( !res )
      return 1;

   for ( auto& pcmd : res.commands )
      if ( pcmd )
         pcmd->execute( res );

   return 0;
}
