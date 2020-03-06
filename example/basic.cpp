#include <argumentum/argparse.h>
#include <climits>
#include <numeric>
#include <vector>

using namespace std;
using namespace argumentum;

int main( int argc, char** argv )
{
   vector<int> numbers;
   bool isSum = false;

   auto parser = argument_parser{};
   parser.config().program( argv[0] ).description( "Accumulator" );
   auto params = parser.params();
   params.add_parameter( numbers, "N" ).minargs( 1 ).metavar( "INT" ).help( "Integers" );
   params.add_parameter( isSum, "--sum", "-s" )
         .nargs( 0 )
         .help( "Sum the integers (default: find the max)" );

   if ( !parser.parse_args( argc, argv, 1 ) )
      return 1;

   auto mmax = []( auto&& a, auto&& b ) { return max( a, b ); };
   auto acc = isSum ? accumulate( numbers.begin(), numbers.end(), 0 )
                    : accumulate( numbers.begin(), numbers.end(), INT_MIN, mmax );
   cout << acc << "\n";
   return 0;
}
