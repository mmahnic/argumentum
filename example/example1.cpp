// In this example the parsed parameters are stored in local variables.
//
// ./example1 -s str -i 1 --float=3.5 -g param

#include "../src/argparser.h"

#include <vector>
#include <string>
#include <optional>
#include <iostream>

using namespace argparse;
using namespace std;

int main( int argc, char** argv )
{
   optional<string> stringValue;
   optional<long> intValue;
   optional<double> floatValue;
   long flag = 0;
   vector<string> params;

   auto parser = ArgumentParser::create();
   parser.config().prog( argv[0] );
   parser.add_argument( stringValue, "-s", "--string" ).nargs( 1 );
   parser.add_argument( intValue, "-i", "--int" ).nargs( 1 );
   parser.add_argument( floatValue, "-f", "--float" ).nargs( 1 );
   parser.add_argument( flag, "-g", "--flag" );
   parser.add_argument( params, "params" );

   vector<string> args;
   for ( int i = 1; i < argc; ++i )
      args.emplace_back( argv[i] );

   auto res = parser.parse_args( args );

   cout << "stringValue: " << stringValue.value_or( "not set" ) << "\n";
   cout << "intValue:    " << ( intValue ? to_string(*intValue) : "not set" ) << "\n";
   cout << "floatValue:  " << ( floatValue ? to_string(*floatValue) : "not set" ) << "\n";
   cout << "flag:        " << flag << "\n";

   cout << "Positional parameters: ";
   for ( auto& param : params )
      cout << "'" << param << "' ";
   cout << "\n";

   if ( res.errors.size() ) {
      cout << "Errors: ";
      for ( auto& err : res.errors )
         cout << "'" << err.option << "':" << err.errorCode << " ";
      cout << "\n";
   }

   return 0;
}
