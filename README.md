[![Build Status](https://travis-ci.com/mmahnic/cpp-argparse.svg?branch=master)](https://travis-ci.com/mmahnic/cpp-argparse)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/mmahnic/cpp-argparse.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/mmahnic/cpp-argparse/context:cpp)

# Argumentum / Argparse

cpp-argparse is a C++ library for writing command-line program interfaces. The arguments that a
program supports are registered in an instance of `arument_parser`, the main library class.
`argument_parser` processes the input arguments, checks that they are valid and converts them to C++
variables. It also generates help and usage messages when requested.

The parsed values are stored in normal C++ variables. Support for numeric types, `std::string`,
`std::vector` and `std::optional` is built into the parser. Any type that can be converted from
`string` with a constructor or an assignment operator can also be used. Other types of values are
supported with parsing actions. The library requires a C++17 compiler.

The library is loosely based on the Python argparse module. It covers most functionality of Python argparse. 

## Building

The library can be prebuilt as a static library or used as header-only.
See [Building and consuming the library](doc/building.md).

## Examples

A basic example (compare with [Python argparse](https://docs.python.org/3/library/argparse.html#example)):

```c++
#include <climits>
#include <argumentum/argparse.h>
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
   parser.add_argument( numbers, "N" ).minargs( 1 ).metavar( "INT" ).help( "Integers" );
   parser.add_argument( isSum, "--sum", "-s" )
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
```

The same example implemented with an action and a default value:

```c++
#include <climits>
#include <argumentum/argparse.h>
#include <numeric>
#include <vector>

using namespace std;
using namespace argumentum;

int main( int argc, char** argv )
{
   vector<int> numbers;
   auto max = []( int a, int b ) { return std::max( a, b ); };
   auto sum = []( int a, int b ) { return a + b; };
   std::pair<std::function<int( int, int )>, int> operation;

   auto parser = argument_parser{};
   parser.config().program( argv[0] ).description( "Accumulator" );
   parser.add_argument( numbers, "N" ).minargs( 1 ).metavar( "INT" ).help( "Integers" );
   parser.add_argument( operation, "--sum", "-s" )
         .nargs( 0 )
         .absent( std::make_pair( max, INT_MIN ) )
         .action( [&]( auto& target, const std::string& value ) {
            target = std::make_pair( sum, 0 );
         } )
         .help( "Sum the integers (default: find the max)" );

   if ( !parser.parse_args( argc, argv, 1 ) )
      return 1;

   auto acc = accumulate( numbers.begin(), numbers.end(), operation.second, operation.first );
   cout << acc << "\n";
   return 0;
}
```

In simple programs target variables can be declared in the function where the arguments are defined
and parsed. In larger programs it is more convenient to store variables in one or more structures.
This is the same example rewritten so that it stores options in a structure:

```c++
#include <climits>
#include <argumentum/argparse.h>
#include <numeric>
#include <vector>

using namespace std;
using namespace argumentum;

class AccumulatorOptions : public argumentum::Options
{
public:
   vector<int> numbers;
   std::pair<std::function<int( int, int )>, int> operation;

protected:
   void add_arguments( argument_parser& parser ) override
   {
      auto max = []( int a, int b ) { return std::max( a, b ); };
      auto sum = []( int a, int b ) { return a + b; };

      parser.add_argument( numbers, "N" ).minargs( 1 ).metavar( "INT" ).help( "Integers" );
      parser.add_argument( operation, "--sum", "-s" )
            .nargs( 0 )
            .absent( std::make_pair( max, INT_MIN ) )
            .action( [&]( auto& target, const std::string& value ) {
               target = std::make_pair( sum, 0 );
            } )
            .help( "Sum the integers (default: find the max)" );
   }
};

void execute( AccumulatorOptions& opt )
{
   auto acc = accumulate(
         opt.numbers.begin(), opt.numbers.end(), opt.operation.second, opt.operation.first );
   cout << acc << "\n";
}

int main( int argc, char** argv )
{
   auto parser = argument_parser{};
   parser.config().program( argv[0] ).description( "Accumulator" );

   auto pOptions = std::make_shared<AccumulatorOptions>();
   parser.add_arguments( pOptions );

   if ( !parser.parse_args( argc, argv, 1 ) )
      return 1;

   execute( *pOptions );
   return 0;
}
```

When a program becomes even more complex it can be subdivided into commands that often act as
independent programs.  We can rewrite the above example with commands.  The main change is that the
class `AccumulatorOptions` is now derived from `CommandOptions` which has the method `execute` that
we use to execute the selected command.  

Note: the interface for defining and executing commands is not stable and will change in the
future.  

```C++
#include <climits>
#include <argumentum/argparse.h>
#include <numeric>
#include <vector>

using namespace std;
using namespace argumentum;

class AccumulatorOptions : public argumentum::CommandOptions
{
public:
   vector<int> numbers;
   std::pair<std::function<int( int, int )>, int> operation;

   void execute( const ParseResults& res )
   {
      auto acc = accumulate( 
         numbers.begin(), numbers.end(), operation.second, operation.first );
      cout << acc << "\n";
   }

protected:
   void add_arguments( argument_parser& parser ) override
   {
     // ... same as above
   }
};

class CmdEchoOptions : public argumentum::CommandOptions
{
public:
   vector<int> numbers;

public:
   void execute( const ParseResult& res ) override
   {
      for ( auto n : numbers )
         cout << n << " ";
      cout << "\n";
   }

protected:
   void add_arguments( argument_parser& parser ) override
   {
      parser.add_argument( numbers, "N" ).minargs( 1 ).metavar( "INT" ).help( "Integers" );
   };
};


int main( int argc, char** argv )
{
   auto parser = argument_parser{};
   parser.config().program( argv[0] ).description( "Accumulator" );
   parser.add_command<CmdAccumulatorOptions>( "fold" ).help( "Accumulate integer values." );
   parser.add_command<CmdEchoOptions>( "echo" ).help( "Echo integers from the command line." );

   auto res = parser.parse_args( argc, argv, 1 );
   if ( !res )
      return 1;

   auto pcmd = res.commands.back();
   if ( !pcmd )
      return 1;

   pcmd->execute( res );
   return 0;
}
```

Command options are instantiated only when the appropriate command is selected with an argument.
The chain of instantiated subcommands is stored in `ParseResults::commands`.  Typically we execute
only the last instantiated (the "deepest") subcommand.

