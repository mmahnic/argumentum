#include <climits>
#include <cppargparse/argparse.h>
#include <numeric>
#include <vector>

using namespace std;
using namespace argparse;

int main( int argc, char** argv )
{
   vector<int> numbers;
   auto omax = []( int a, int b ) { return std::max( a, b ); };
   auto osum = []( int a, int b ) { return a + b; };
   std::function<int( int, int )> operation = omax;
   int init = INT_MIN;

   auto parser = argument_parser{};
   parser.config().program( argv[0] ).description( "Accumulator" );
   parser.add_argument( numbers, "N" ).minargs( 1 ).metavar( "INT" ).help( "Integers" );
   parser.add_argument( operation, "--sum", "-s" )
         .nargs( 0 )
         .action( [&]( auto& target, const std::string& value ) {
            target = osum;
            init = 0;
         } )
         .help( "Sum the integers (default: find the max)" );

   if ( !parser.parse_args( argc, argv, 1 ) )
      return 1;

   std::cout << "parsed\n";
   auto acc = accumulate( numbers.begin(), numbers.end(), init, operation );
   cout << acc << "\n";
   return 0;
}
