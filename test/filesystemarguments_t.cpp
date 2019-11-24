// Copyright (c) 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include <cppargparse/argparse.h>

#include <algorithm>
#include <gtest/gtest.h>
#include <map>
#include <sstream>

using namespace argparse;

class TestFilesystem : public Filesystem
{
   std::map<std::string, std::vector<std::string>> mFiles;

public:
   std::unique_ptr<ArgumentStream> open( const std::string& filename ) override
   {
      auto iv = mFiles.find( filename );
      if ( iv != mFiles.end() ) {
         using iter_t = std::vector<std::string>::iterator;
         return std::make_unique<IteratorArgumentStream<iter_t>>(
               std::begin( iv->second ), std::end( iv->second ) );
      }

      return nullptr;
   }

   void addFile( const std::string& name, const std::vector<std::string>& content )
   {
      mFiles[name] = content;
   }

   void addFile( const std::string& name, std::vector<std::string>&& content )
   {
      mFiles[name] = std::move( content );
   }
};

TEST( FilesystemArguments, shouldReadArgumentsFromFilesystem )
{
   auto pfs = std::make_shared<TestFilesystem>();
   pfs->addFile( "a.opt", { "--alpha", "--beta" } );
   pfs->addFile( "b.opt", { "--three", "--four" } );

   auto parser = argument_parser{};
   parser.config().filesystem( pfs );

   std::array<bool, 6> v{ false, false, false, false, false, false };
   parser.add_argument( v[0], "--alpha" ).nargs( 0 );
   parser.add_argument( v[1], "--beta" ).nargs( 0 );
   parser.add_argument( v[2], "--three" ).nargs( 0 );
   parser.add_argument( v[3], "--four" ).nargs( 0 );
   parser.add_argument( v[4], "--alice" ).nargs( 0 );
   parser.add_argument( v[5], "--bob" ).nargs( 0 );

   auto res = parser.parse_args( { "--alice", "@a.opt", "@b.opt", "--bob" } );

   EXPECT_TRUE( !!res );

   for ( auto flag : v )
      EXPECT_TRUE( flag );
}

TEST( FilesystemArguments, shouldReadArgumentsFromFilesystemRecursively )
{
   auto pfs = std::make_shared<TestFilesystem>();
   pfs->addFile( "a.opt", { "--alpha", "--beta", "@b.opt" } );
   pfs->addFile( "b.opt", { "--three", "--four" } );

   auto parser = argument_parser{};
   parser.config().filesystem( pfs );

   std::array<bool, 6> v{ false, false, false, false, false, false };
   parser.add_argument( v[0], "--alpha" ).nargs( 0 );
   parser.add_argument( v[1], "--beta" ).nargs( 0 );
   parser.add_argument( v[2], "--three" ).nargs( 0 );
   parser.add_argument( v[3], "--four" ).nargs( 0 );
   parser.add_argument( v[4], "--alice" ).nargs( 0 );
   parser.add_argument( v[5], "--bob" ).nargs( 0 );

   auto res = parser.parse_args( { "--alice", "@a.opt", "--bob" } );

   EXPECT_TRUE( !!res );

   for ( auto flag : v )
      EXPECT_TRUE( flag );
}

TEST( FilesystemArguments, shouldFailWhenStreamsRecursedTooDeep )
{
   auto pfs = std::make_shared<TestFilesystem>();
   pfs->addFile( "a.opt", { "--alpha", "--beta", "@a.opt" } );

   auto parser = argument_parser{};
   parser.config().filesystem( pfs );

   std::array<bool, 6> v{ false, false, false, false, false, false };
   parser.add_argument( v[0], "--alpha" ).nargs( 0 );
   parser.add_argument( v[1], "--beta" ).nargs( 0 );
   parser.add_argument( v[4], "--alice" ).nargs( 0 );
   parser.add_argument( v[5], "--bob" ).nargs( 0 );

   auto res = parser.parse_args( { "--alice", "@a.opt", "--bob" } );

   EXPECT_FALSE( !!res );
   EXPECT_EQ( INCLUDE_TOO_DEEP, res.errors.front().errorCode );
}
