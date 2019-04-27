// In this example the parsed parameters are stored in local variables.
//
// ./example1 -s str -i 1 --float=3.5 -g param

#include "../src/argparser.h"

#include <iostream>
#include <optional>
#include <string>
#include <vector>

using namespace argparse;
using namespace std;

int main( int argc, char** argv )
{
   optional<string> stringValue;
   optional<long> intValue;
   optional<double> floatValue;
   long flag = 0;
   vector<string> params;

   auto parser = argument_parser{};
   parser.config().program( argv[0] );
   parser.add_argument( stringValue, "-s", "--string" )
         .help( "Set the value of a string option." )
         .nargs( 1 );
   parser.add_argument( intValue, "-i", "--int" )
         .help( "Set the value of an integer option." )
         .nargs( 1 );
   parser.add_argument( floatValue, "-f", "--float" )
         .help( "Set the value of a float option." )
         .nargs( 1 );
   parser.add_argument( flag, "-g", "--flag" ).help( "Set the flag." );
   parser.add_argument( params, "params" ).help( "Free parameters." );
   parser.add_help();
   parser.add_help( "--extra-help" ).help( "Another way to print help and exit." );

   vector<string> args;
   for ( int i = 1; i < argc; ++i )
      args.emplace_back( argv[i] );

   auto res = parser.parse_args( args );

   cout << "stringValue: " << stringValue.value_or( "not set" ) << "\n";
   cout << "intValue:    " << ( intValue ? to_string( *intValue ) : "not set" ) << "\n";
   cout << "floatValue:  " << ( floatValue ? to_string( *floatValue ) : "not set" ) << "\n";
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
