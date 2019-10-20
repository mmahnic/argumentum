#include <climits>
#include <cppargparse/argparse.h>
#include <numeric>
#include <vector>

using namespace std;
using namespace argparse;

int main( int argc, char** argv )
{
   vector<int> numbers;
   bool isSum = false;

   auto parser = argument_parser{};
   parser.config().program( argv[0] ).on_exit_return().description( "Accumulator" );
   parser.add_argument( numbers, "N" ).minargs( 1 ).metavar( "INT" ).help( "Integers" );
   parser.add_argument( isSum, "--sum", "-s" )
         .nargs( 0 )
         .help( "Sum the integers (default: find the max)" );

   auto res = parser.parse_args( argc, argv, 1 );
   if ( !res ) {
      // res.format_errors( res, cout );
      cout << "errors\n";
      return 1;
   }

   auto mmax = []( auto&& a, auto&& b ) { return max( a, b ); };
   auto acc = isSum ? accumulate( numbers.begin(), numbers.end(), 0 )
                    : accumulate( numbers.begin(), numbers.end(), INT_MIN, mmax );
   cout << acc << "\n";
   return 0;
}
