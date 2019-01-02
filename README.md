[![Build Status](https://travis-ci.com/mmahnic/argparse.svg?branch=master)](https://travis-ci.com/mmahnic/argparse)

# Argparse

A C++ library for parsing command line parameters.

This is the result of a weekend TDD practice. The library is still under construction.

Example:

```c++
#include "argparser.h"

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
   long intValue2 = 0;
   vector<string> params;

   // Note: the argument parser will take references to the variables so it must
   // outlive those variables. We signal this by using the create_unsafe() 
   // factory method.
   auto parser = ArgumentParser::create_unsafe();
   parser.config().prog( argv[0] );
   parser.add_argument( stringValue, "-s", "--string" ).nargs( 1 );
   parser.add_argument( intValue, "-i", "--int" ).nargs( 1 );
   parser.add_argument( floatValue, "-f", "--float" ).nargs( 1 );
   parser.add_argument( intValue2, "-v", "--verbose" );
   parser.add_argument( params, "params" );

   vector<string> args;
   for ( int i = 1; i < argc; ++i )
      args.emplace_back( argv[i] );

   auto res = parser.parse_args( args );

   cout << "stringValue: "
      << ( bool(stringValue) ? stringValue.value() : "not set" ) << "\n";
   cout << "intValue:    "
      << ( bool(intValue) ? to_string(intValue.value()) : "not set" ) << "\n";
   cout << "floatValue:  "
      << ( bool(floatValue) ? to_string(floatValue.value()) : "not set" ) << "\n";
   cout << "intValue2:   " << intValue2 << "\n";

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
```
