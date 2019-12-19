#include <argumentum/argparse.h>
#include <climits>
#include <numeric>
#include <vector>

using namespace std;
using namespace argparse;

class AccumulatorOptions : public argparse::Options
{
public:
   vector<int> numbers;
   std::pair<std::function<int( int, int )>, int> operation;

protected:
   void add_arguments( argument_parser& parser ) override
   {
      auto max = []( int a, int b ) { return std::max( a, b ); };
      auto sum = []( int a, int b ) { return a + b; };

      parser.add_argument( numbers, "N" ).minargs( 1 ).metavar( "INT" ).help( "Integers" );
      parser.add_argument( operation, "--sum", "-s" )
            .nargs( 0 )
            .absent( std::make_pair( max, INT_MIN ) )
            .action( [&]( auto& target, const std::string& value ) {
               target = std::make_pair( sum, 0 );
            } )
            .help( "Sum the integers (default: find the max)" );
   }
};

void execute( AccumulatorOptions& opt )
{
   auto acc = accumulate(
         opt.numbers.begin(), opt.numbers.end(), opt.operation.second, opt.operation.first );
   cout << acc << "\n";
}

int main( int argc, char** argv )
{
   auto parser = argument_parser{};
   parser.config().program( argv[0] ).description( "Accumulator" );

   auto pOptions = std::make_shared<AccumulatorOptions>();
   parser.add_arguments( pOptions );

   if ( !parser.parse_args( argc, argv, 1 ) )
      return 1;

   execute( *pOptions );
   return 0;
}
