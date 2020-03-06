[![Build Status](https://travis-ci.com/mmahnic/argumentum.svg?branch=master)](https://travis-ci.com/mmahnic/argumentum)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/mmahnic/argumentum.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/mmahnic/argumentum/context:cpp)

# Argumentum / Argparse

Argumentum is a C++ library for writing command-line program interfaces. The parameters that a
program supports are registered in an instance of `arument_parser`, the main library class.
`argument_parser` processes the input arguments, checks that they are valid and converts them to C++
variables. It also generates help and usage messages when requested.

The parsed values are stored in normal C++ variables. Support for numeric types, `std::string`,
`std::vector` and `std::optional` is built into the parser. Any type that can be converted from
`string` with a constructor or an assignment operator can also be used. Other types of values are
supported with parsing actions. The library requires a C++17 compiler.

The library is loosely based on the Python argparse module. It covers most functionality of Python argparse. 

## Building

The library can be built as a static library or used as header-only.
See [Building and consuming the library](doc/building.md).

## A basic example

In this example the program accepts integers and finds the largest one.  If the
option `--sum` is passed to the program, the numbers are summed, instead.
(Compare with [Python argparse](https://docs.python.org/3/library/argparse.html#example).)

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
   auto params = parser.params();
   parser.config().program( argv[0] ).description( "Accumulator" );
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
```

## Target values

The parser parses input strings and stores the parsed results in target values
which are ordinary C++ variables.  The target values must outlive the parser
because the parser stores target value references. On the other hand, the
parser can be destroyed after parsing is done.

The supported types of target values are:

- C++ numeric types, `bool`, `std::string`,
- any type that has a constructor that accepts `std::string`,
- any type that has an `operator=` that accepts `std::string`,
- any type `T` for which a converter `argumentum::from_string<T>::convert` exists,
- `std::vector` of simple target values.

If information about whether a value was set or not is needed, `std::optional` can be used:

```c++
   std::optional<std::string> str;
   params.add_parameter( str, "-s" ).maxargs( 1 );

   // ... parser.parse_args ...

   if ( !str )
      std::cout << "The option -s was not used.\n";
   else
      std::cout << "The option -s was set with value '" << *str << "' .\n";
```

Additional types can be supported through parsing actions like in the examples below.


## Variants of the basic example

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
   auto params = parser.params();
   parser.config().program( argv[0] ).description( "Accumulator" );
   params.add_parameter( numbers, "N" ).minargs( 1 ).metavar( "INT" ).help( "Integers" );
   params.add_parameter( operation, "--sum", "-s" )
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

In simple programs target variables can be declared in the function where parameters are defined and
arguments parsed.  In larger programs it is more convenient to store target variables in one or more
structures.  This is the same example rewritten so that it stores options in a structure:

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
   void add_parameters( ParameterConfig& params ) override
   {
      auto max = []( int a, int b ) { return std::max( a, b ); };
      auto sum = []( int a, int b ) { return a + b; };

      params.add_parameter( numbers, "N" ).minargs( 1 ).metavar( "INT" ).help( "Integers" );
      params.add_parameter( operation, "--sum", "-s" )
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
   auto params = parser.params();
   parser.config().program( argv[0] ).description( "Accumulator" );

   auto pOptions = std::make_shared<AccumulatorOptions>();
   params.add_parameters( pOptions );

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
   void add_parameters( ParameterConfig& params ) override
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
   void add_parameters( ParameterConfig& params ) override
   {
      params.add_parameter( numbers, "N" ).minargs( 1 ).metavar( "INT" ).help( "Integers" );
   };
};


int main( int argc, char** argv )
{
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().program( argv[0] ).description( "Accumulator" );
   params.add_command<CmdAccumulatorOptions>( "fold" ).help( "Accumulate integer values." );
   params.add_command<CmdEchoOptions>( "echo" ).help( "Echo integers from the command line." );

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

Command options are instantiated and added to the parser only when the appropriate command is
selected with an argument.  The chain of instantiated subcommands is stored in
`ParseResults::commands`.  Typically we execute only the last instantiated (the "deepest")
subcommand.

If a program has global options that we want to access from a command, we have to instantiate
command options in advance and set a link in command options to global options.  In this case it is
most convenient to store global options in an Options structure.  Even though we add an instance of
command options to the parser, the actual options will be registered with the parser only when the
command is activated by the arguments.

In the following example AccumulatorOptions need access to global options.  Only the added and
modified methods are shown:

```C++
class GlobalOptions : public argumentum::Options
{
public:
   int logLevel = 0;
   void add_parameters( ParameterConfig& params ) override
   {
      params.add_parameter( logLevel, "--loglevel" ).nargs( 1 );
   }
};

class AccumulatorOptions : public argumentum::CommandOptions
{
   std::shared_ptr<GloblaOptions> mpGlobal;
public:
   AccumulatorOptions( std::string_view name, std::shared_ptr<GloblaOptions> pGlobal )
      : CommandOptions( name )
      , mpGlobal( pGlobal )
  {}

  void execute( const ParseResults& res )
  {
     if ( mpGlobal && mpGlobal->logLevel > 0 )
       cout << "Accumulating " << numbers.size() << " numbers\n";

     auto acc = accumulate(
        numbers.begin(), numbers.end(), operation.second, operation.first );
     cout << acc << "\n";
  }
};

int main( int argc, char** argv )
{
   auto parser = argument_parser{};
   auto params = parser.params();
   parser.config().program( argv[0] ).description( "Accumulator" );

   auto pGlobal = std::make_shared<GlobalOptions>();
   auto pAccumulator = std::make_shared<CmdAccumulatorOptions>( "fold", pGlobal );

   params.add_parameters( pGlobal );
   params.add_command( pAccumulator ).help( "Accumulate integer values." );
   params.add_command<CmdEchoOptions>( "echo" ).help( "Echo integers from the command line." );

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
