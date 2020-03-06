// In this example the parsed parameters are stored in two Options structures.
//
// ./example2 -s str -i 1 2 3 --float=3.5 -g param

#include <argumentum/argparse.h>

#include <iostream>
#include <optional>
#include <string>
#include <vector>

using namespace argumentum;
using namespace std;

struct OptionsA : public argumentum::Options
{
   optional<string> stringValue;
   vector<long> intValues;
   void add_arguments( argument_parser& parser ) override
   {
      auto params = parser.params();
      params.add_parameter( stringValue, "-s", "--string" ).nargs( 1 );
      params.add_parameter( intValues, "-i", "--int" ).minargs( 0 );
   }
};

struct OptionsB : public argumentum::Options
{
   optional<double> floatValue;
   long flag = 0;
   vector<string> strings;

   void add_arguments( argument_parser& parser ) override
   {
      auto params = parser.params();
      params.add_parameter( floatValue, "-f", "--float" ).nargs( 1 );
      params.add_parameter( flag, "-g", "--flag" );
      params.add_parameter( strings, "params" );
   }
};

int main( int argc, char** argv )
{
   auto parser = argument_parser{};
   parser.config().program( argv[0] );
   auto params = parser.params();
   auto pOptionsA = std::make_shared<OptionsA>();
   params.add_parameters( pOptionsA );
   auto pOptionsB = std::make_shared<OptionsB>();
   params.add_parameters( pOptionsB );

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
   for ( auto& param : pOptionsB->strings )
      cout << "'" << param << "' ";
   cout << "\n";

   if ( !res ) {
      if ( res.errors.size() ) {
         cout << "Errors: ";
         for ( auto& err : res.errors )
            cout << "'" << err.option << "':" << err.errorCode << " ";
         cout << "\n";
      }
   }

   return 0;
}
