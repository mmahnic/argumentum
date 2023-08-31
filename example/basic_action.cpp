#include <argumentum/argparse.h>
#include <climits>
#include <iostream>
#include <numeric>
#include <vector>

using namespace std;
using namespace argumentum;

int main( int argc, char** argv )
{
   vector<int> numbers;
   auto max = []( int a, int b ) {
      return std::max( a, b );
   };
   auto sum = []( int a, int b ) {
      return a + b;
   };
   std::pair<std::function<int( int, int )>, int> operation;

   auto parser = argument_parser{};
   parser.config().program( argv[0] ).description( "Accumulator" );
   auto params = parser.params();
   params.add_parameter( numbers, "N" ).minargs( 1 ).metavar( "INT" ).help( "Integers" );
   params.add_parameter( operation, "--sum", "-s" )
         .nargs( 0 )
         .absent( std::make_pair( max, INT_MIN ) )
         .action( [&]( auto& target, const std::string& /*value*/ ) {
            target = std::make_pair( sum, 0 );
         } )
         .help( "Sum the integers (default: find the max)" );

   if ( !parser.parse_args( argc, argv, 1 ) )
      return 1;

   auto acc = accumulate( numbers.begin(), numbers.end(), operation.second, operation.first );
   cout << acc << "\n";
   return 0;
}
