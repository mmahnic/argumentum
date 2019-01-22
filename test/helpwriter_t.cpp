// Copyright (c) 2018, 2019 Marko Mahnič
// License: MIT. See LICENSE in the root of the project.

#include "../src/argparser.h"
#include "../src/helpformatter.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <sstream>

using namespace argparse;

namespace {
std::vector<std::string_view> splitLines( std::string_view text )
{
    std::vector<std::string_view> output;
    size_t start = 0;
    auto delims = "\n\r";

    auto isWinEol = [&text]( auto pos ) {
       return text[pos] == '\r' && text[pos+1] == '\n';
    };

    while ( start < text.size() ) {
       const auto stop = text.find_first_of( delims, start );

       if ( start != stop )
          output.emplace_back( text.substr( start, stop-start ) );

       if ( stop == std::string_view::npos )
          break;

       start = stop + ( isWinEol( stop ) ? 2 : 1 );
    }

    return output;
}
}

TEST( WriterTest, shouldSplitTextIntoWordsAtWhitespace )
{
   std::string text =
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
      "sed do eiusmod tempor incididunt ut labore et dolore magna "
      "aliqua.";

   std::stringstream strout;
   Writer writer( strout );
   auto words = writer.splitIntoWords( text );

   EXPECT_EQ( 19, words.size() );
}
