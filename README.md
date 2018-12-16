<a href="https://scan.coverity.com/projects/mmahnic-argparse">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/17475/badge.svg"/>
</a>

# Argparse

A C++ library for parsing command line arguments.

This is the result of a weekend TDD practice.

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

   ArgumentParser parser;
   parser.addOption( stringValue ).shortName( "s" ).longName( "string" ).hasArgument();
   parser.addOption( intValue ).shortName( "i" ).longName( "int" ).hasArgument();
   parser.addOption( floatValue ).shortName( "f" ).longName( "float" ).hasArgument();
   parser.addOption( intValue2 ).shortName( "v" ).longName( "verbose" );

   vector<string> args;
   for ( int i = 1; i < argc; ++i )
      args.emplace_back( argv[i] );

   auto res = parser.parseArguments( args );

   cout << "stringValue: "
      << ( bool(stringValue) ? stringValue.value() : "not set" ) << "\n";
   cout << "intValue:    "
      << ( bool(intValue) ? to_string(intValue.value()) : "not set" ) << "\n";
   cout << "floatValue:  "
      << ( bool(floatValue) ? to_string(floatValue.value()) : "not set" ) << "\n";
   cout << "intValue2:   " << intValue2 << "\n";

   cout << "Free arguments: ";
   for ( auto& arg : res.freeArguments )
      cout << "'" << arg << "' ";
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
