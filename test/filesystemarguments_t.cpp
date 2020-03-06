// Copyright (c) 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include <argumentum/argparse.h>

#include <algorithm>
#include <gtest/gtest.h>
#include <map>
#include <sstream>

#if __has_include( <filesystem> )
#define HAVE_FILESYSTEM 1
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include( <experimental/filesystem> )
#define HAVE_FILESYSTEM 1
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#define HAVE_FILESYSTEM 0
#endif

using namespace argumentum;

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
   auto params = parser.params();
   parser.config().filesystem( pfs );

   std::array<bool, 6> v{ false, false, false, false, false, false };
   params.add_parameter( v[0], "--alpha" ).nargs( 0 );
   params.add_parameter( v[1], "--beta" ).nargs( 0 );
   params.add_parameter( v[2], "--three" ).nargs( 0 );
   params.add_parameter( v[3], "--four" ).nargs( 0 );
   params.add_parameter( v[4], "--alice" ).nargs( 0 );
   params.add_parameter( v[5], "--bob" ).nargs( 0 );

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
   auto params = parser.params();
   parser.config().filesystem( pfs );

   std::array<bool, 6> v{ false, false, false, false, false, false };
   params.add_parameter( v[0], "--alpha" ).nargs( 0 );
   params.add_parameter( v[1], "--beta" ).nargs( 0 );
   params.add_parameter( v[2], "--three" ).nargs( 0 );
   params.add_parameter( v[3], "--four" ).nargs( 0 );
   params.add_parameter( v[4], "--alice" ).nargs( 0 );
   params.add_parameter( v[5], "--bob" ).nargs( 0 );

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
   auto params = parser.params();
   parser.config().filesystem( pfs );

   std::array<bool, 6> v{ false, false, false, false, false, false };
   params.add_parameter( v[0], "--alpha" ).nargs( 0 );
   params.add_parameter( v[1], "--beta" ).nargs( 0 );
   params.add_parameter( v[4], "--alice" ).nargs( 0 );
   params.add_parameter( v[5], "--bob" ).nargs( 0 );

   auto res = parser.parse_args( { "--alice", "@a.opt", "--bob" } );

   EXPECT_FALSE( !!res );
   EXPECT_EQ( INCLUDE_TOO_DEEP, res.errors.front().errorCode );
}

// The default Filesystem reads from the real filesystem.
TEST( FilesystemArguments, shouldUseDefaultFilesystemWhenNoneIsDefined )
{
#if HAVE_FILESYSTEM
   auto tmpdir = fs::temp_directory_path() / "xdata";
   if ( !fs::exists( tmpdir ) )
      fs::create_directory( tmpdir );
   auto tmpfile = tmpdir / "a.opt";
   auto f = std::ofstream( tmpfile );
   f << "--alpha\nfrom-file";
   f.close();

   auto parser = argument_parser{};
   auto params = parser.params();
   std::string alpha;
   params.add_parameter( alpha, "--alpha" ).nargs( 1 );

   auto res = parser.parse_args( { "@" + tmpfile.generic_string() } );

   EXPECT_TRUE( !!res );
   EXPECT_EQ( "from-file", alpha );
#else
   std::cout << "No filesystem. Test skipped.\n";
#endif
}
