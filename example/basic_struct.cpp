#include <argumentum/argparse.h>
#include <climits>
#include <numeric>
#include <vector>

using namespace std;
using namespace argumentum;

class AccumulatorOptions : public argumentum::Options
{
public:
   vector<int> numbers;
   std::pair<std::function<int( int, int )>, int> operation;

protected:
   void add_arguments( argument_parser& parser ) override
   {
      auto params = parser.params();
      auto max = []( int a, int b ) {
         return std::max( a, b );
      };
      auto sum = []( int a, int b ) {
         return a + b;
      };

      params.add_parameter( numbers, "N" ).minargs( 1 ).metavar( "INT" ).help( "Integers" );
      params.add_parameter( operation, "--sum", "-s" )
            .nargs( 0 )
            .absent( std::make_pair( max, INT_MIN ) )
            .action( [&]( auto& target, const std::string& /*value*/ ) {
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
   auto params = parser.params();
   params.add_parameters( pOptions );

   if ( !parser.parse_args( argc, argv, 1 ) )
      return 1;

   execute( *pOptions );
   return 0;
}
