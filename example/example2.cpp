// In this example the parsed parameters are stored in two Options structures.
//
// ./example2 -s str -i 1 2 3 --float=3.5 -g param

#include "../src/argparser.h"

#include <iostream>
#include <optional>
#include <string>
#include <vector>

using namespace argparse;
using namespace std;

struct OptionsA : public argparse::Options
{
   optional<string> stringValue;
   vector<long> intValues;
   void add_arguments( argument_parser& parser ) override
   {
      parser.add_argument( stringValue, "-s", "--string" ).nargs( 1 );
      parser.add_argument( intValues, "-i", "--int" ).minargs( 0 );
   }
};

struct OptionsB : public argparse::Options
{
   optional<double> floatValue;
   long flag = 0;
   vector<string> params;

   void add_arguments( argument_parser& parser ) override
   {
      parser.add_argument( floatValue, "-f", "--float" ).nargs( 1 );
      parser.add_argument( flag, "-g", "--flag" );
      parser.add_argument( params, "params" );
   }
};

int main( int argc, char** argv )
{
   auto parser = argument_parser{};
   parser.config().program( argv[0] );
   auto pOptionsA = std::make_shared<OptionsA>();
   parser.add_arguments( pOptionsA );
   auto pOptionsB = std::make_shared<OptionsB>();
   parser.add_arguments( pOptionsB );

   vector<string> args;
   for ( int i = 1; i < argc; ++i )
      args.emplace_back( argv[i] );

   auto res = parser.parse_args( args );

   cout << "A stringValue: " << pOptionsA->stringValue.value_or( "not set" ) << "\n";
   cout << "A intValues:   " << ( pOptionsA->intValues.empty() ? "not set" : "" );
   for ( auto&& v : pOptionsA->intValues )
      cout << v << " ";
   cout << "\n";
   cout << "B floatValue:  "
        << ( pOptionsB->floatValue ? to_string( *pOptionsB->floatValue ) : "not set" ) << "\n";
   cout << "B flag:        " << pOptionsB->flag << "\n";

   cout << "B Positional parameters: ";
   for ( auto& param : pOptionsB->params )
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
