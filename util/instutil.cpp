// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include <cppargparse/argparse.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

using namespace std;
using namespace argparse;

class MainHeaderCmd : public argparse::CommandOptions
{
   std::string inputFilename;
   std::string outputFilename;

public:
   using CommandOptions::CommandOptions;
   void execute( const argparse::ParseResult& res ) override
   {
      auto fin = ifstream( inputFilename );
      auto fout = ofstream( outputFilename );

      auto rxSrcPath = std::regex( "\\.\\./\\.\\./src/" );
      auto instPath = "inc/";

      std::string line;
      while ( std::getline( fin, line ) )
         fout << regex_replace( line, rxSrcPath, instPath ) << "\n";
   }

protected:
   void add_arguments( argparse::argument_parser& parser ) override
   {
      parser.add_argument( inputFilename, "input" )
            .nargs( 1 )
            .help( "The path of the source file." );

      parser.add_argument( outputFilename, "output" )
            .nargs( 1 )
            .help( "The path of the destination file." );
   }
};

int main( int argc, char** argv )
{
   auto parser = argument_parser{};
   parser.config().program( argv[0] ).description( "cpp-argparse installation utility" );
   parser.add_command<MainHeaderCmd>( "header" ).help( "Transform the main header." );

   auto res = parser.parse_args( argc, argv, 1 );
   if ( !res )
      return 1;

   for ( auto& pcmd : res.commands )
      if ( pcmd )
         pcmd->execute( res );

   return 0;
}
