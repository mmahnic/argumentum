// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#include <argumentum/argparse.h>

#include <argumentum/../../src/writer.h>

#include <algorithm>
#include <gtest/gtest.h>
#include <sstream>

using namespace argumentum;

namespace {
enum class EKeepEmpty : bool { no = false, yes = true };

std::vector<std::string_view> splitLines(
      std::string_view text, EKeepEmpty keepEmpty = EKeepEmpty::no )
{
   std::vector<std::string_view> output;
   size_t start = 0;
   auto delims = "\n\r";

   auto isWinEol = [&text]( auto pos ) {
      return text[pos] == '\r' && text[pos + 1] == '\n';
   };

   while ( start < text.size() ) {
      const auto stop = text.find_first_of( delims, start );

      if ( keepEmpty == EKeepEmpty::yes || start != stop )
         output.emplace_back( text.substr( start, stop - start ) );

      if ( stop == std::string_view::npos )
         break;

      start = stop + ( isWinEol( stop ) ? 2 : 1 );
   }

   return output;
}
}   // namespace

namespace {
std::string loremIpsum123_19w =
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed "
      "do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
}

TEST( WriterTest, shouldSplitTextIntoWordsAtWhitespace )
{
   std::string text = loremIpsum123_19w;
   std::stringstream strout;
   Writer writer( strout );
   auto words = writer.splitIntoWords( text );

   EXPECT_EQ( 19, words.size() );
}

TEST( WriterTest, shouldReformatText )
{
   std::string text = loremIpsum123_19w;
   std::stringstream strout;
   Writer writer( strout, 27 );

   writer.write( text );
   auto written = strout.str();
   auto lines = splitLines( written );

   EXPECT_LT( 4, lines.size() );

   for ( auto line : lines )
      EXPECT_GE( 27, line.size() );
}

TEST( WriterIndentTest, shouldIndentFormattedText )
{
   std::string text = loremIpsum123_19w;
   std::stringstream strout;
   Writer writer( strout, 27 );
   writer.setIndent( 3 );

   writer.write( text );
   auto written = strout.str();
   auto lines = splitLines( written );

   EXPECT_LT( 4, lines.size() );

   for ( auto line : lines )
      EXPECT_GE( 27, line.size() );

   for ( auto line : lines ) {
      EXPECT_EQ( "   ", line.substr( 0, 3 ) );
      EXPECT_NE( "    ", line.substr( 0, 4 ) );
   }
}

TEST( WriterStartLineTest, shouldStartANewLine )
{
   std::stringstream strout;
   Writer writer( strout, 80 );
   writer.write( "aaaa" );
   writer.write( "bbbb" );
   writer.startLine();
   writer.write( "cccc" );

   auto written = strout.str();
   auto lines = splitLines( written );

   ASSERT_EQ( 2, lines.size() );
   EXPECT_EQ( "aaaa bbbb", lines[0] );
   EXPECT_EQ( "cccc", lines[1] );
}

TEST( WriterStartLineTest, shouldNotStartANewLineAtBol )
{
   std::stringstream strout;
   Writer writer( strout, 80 );
   writer.startLine();
   writer.startLine();
   writer.write( "aaaa" );
   writer.write( "bbbb" );
   writer.startLine();
   writer.startLine();
   writer.startLine();
   writer.write( "cccc" );

   auto written = strout.str();
   auto lines = splitLines( written );

   ASSERT_EQ( 2, lines.size() );
   EXPECT_EQ( "aaaa bbbb", lines[0] );
   EXPECT_EQ( "cccc", lines[1] );
}

TEST( WriterSkipToColumnTest, shouldSkipToRequestedColumn )
{
   std::stringstream strout;
   Writer writer( strout, 80 );
   writer.write( "aaaa" );
   writer.skipToColumnOrNewLine( 31 );
   writer.write( "bbbb" );
   writer.startLine();
   writer.write( "cccc" );

   auto written = strout.str();
   auto lines = splitLines( written );

   ASSERT_EQ( 2, lines.size() );
   EXPECT_EQ( 0, lines[0].find( "aaaa" ) );
   EXPECT_EQ( 31, lines[0].find( "bbbb" ) );
   EXPECT_EQ( 0, lines[1].find( "cccc" ) );
}

TEST( WriterSkipToColumnTest, shouldSkipToNewLineIfPastRequestedColumn )
{
   std::stringstream strout;
   Writer writer( strout, 80 );
   writer.write( "aaaa" );
   writer.skipToColumnOrNewLine( 3 );
   writer.write( "bbbb" );
   writer.startLine();
   writer.write( "cccc" );

   auto written = strout.str();
   auto lines = splitLines( written );

   ASSERT_EQ( 3, lines.size() );
   EXPECT_EQ( "aaaa", lines[0] );
   EXPECT_EQ( "bbbb", lines[1] );
   EXPECT_EQ( "cccc", lines[2] );
}

TEST( WriterSkipToColumnTest, shouldContinueWritingIfAtRequestedColumn )
{
   std::stringstream strout;
   Writer writer( strout, 80 );
   writer.write( "aaaa" );
   writer.skipToColumnOrNewLine( 4 );
   writer.write( "bbbb" );
   writer.startLine();
   writer.write( "cccc" );

   auto written = strout.str();
   auto lines = splitLines( written );

   ASSERT_EQ( 2, lines.size() );
   EXPECT_EQ( "aaaa bbbb", lines[0] );
   EXPECT_EQ( "cccc", lines[1] );
}

TEST( WriterParagraphTest, shouldStartParagraph )
{
   std::stringstream strout;
   Writer writer( strout, 80 );
   writer.write( "aaaa" );
   writer.startParagraph();
   writer.write( "bbbb" );

   auto written = strout.str();
   auto lines = splitLines( written, EKeepEmpty::yes );

   ASSERT_EQ( 3, lines.size() );
   EXPECT_EQ( "aaaa", lines[0] );
   EXPECT_EQ( "", lines[1] );
   EXPECT_EQ( "bbbb", lines[2] );
}

TEST( WriterParagraphTest, shouldStartParagraphWhenAtBol )
{
   std::stringstream strout;
   Writer writer( strout, 80 );
   writer.write( "aaaa" );
   writer.startLine();
   writer.startParagraph();
   writer.write( "bbbb" );

   auto written = strout.str();
   auto lines = splitLines( written, EKeepEmpty::yes );

   ASSERT_EQ( 3, lines.size() );
   EXPECT_EQ( "aaaa", lines[0] );
   EXPECT_EQ( "", lines[1] );
   EXPECT_EQ( "bbbb", lines[2] );
}

TEST( WriterParagraphTest, shouldNotStartConsecutiveParagraphs )
{
   std::stringstream strout;
   Writer writer( strout, 80 );
   writer.write( "aaaa" );
   writer.startParagraph();
   writer.startParagraph();
   writer.startParagraph();
   writer.startParagraph();
   writer.write( "bbbb" );

   auto written = strout.str();
   auto lines = splitLines( written, EKeepEmpty::yes );

   ASSERT_EQ( 3, lines.size() );
   EXPECT_EQ( "aaaa", lines[0] );
   EXPECT_EQ( "", lines[1] );
   EXPECT_EQ( "bbbb", lines[2] );
}

TEST( WriterParagraphTest, shouldStartParagraphWithoutNextWrite )
{
   std::stringstream strout;
   Writer writer( strout, 80 );
   writer.write( "aaaa" );
   writer.startParagraph();
   writer.write( "bbbb" );
   writer.startParagraph();   // no write() after this

   auto written = strout.str();
   auto lines = splitLines( written, EKeepEmpty::yes );

   ASSERT_EQ( 4, lines.size() );
   EXPECT_EQ( "aaaa", lines[0] );
   EXPECT_EQ( "", lines[1] );
   EXPECT_EQ( "bbbb", lines[2] );
   EXPECT_EQ( "", lines[3] );
}

TEST( WriterInputParagraphTest, shouldBreakTextIntoParagraphs )
{
   auto text = "Two.\n\nParagraphs.";
   auto paras = Writer::splitIntoParagraphs( text );

   // Two "full" paragraphs, one empty paragraph.
   ASSERT_EQ( 3, paras.size() );
   EXPECT_EQ( "Two.", paras[0] );
   EXPECT_EQ( "", paras[1] );
   EXPECT_EQ( "Paragraphs.", paras[2] );
}

TEST( WriterInputParagraphTest, shouldBreakTextIntoParagraphsWithMixedParagraphBreaks )
{
   auto text = "Two. \n \n   \n \t  \n Paragraphs.";
   auto paras = Writer::splitIntoParagraphs( text );

   // Two "full" paragraphs, one empty paragraph.
   ASSERT_EQ( 3, paras.size() );
   EXPECT_EQ( "Two.", paras[0] );
   EXPECT_EQ( "", paras[1] );
   EXPECT_EQ( "Paragraphs.", paras[2] );
}

TEST( WriterInputParagraphTest, shouldRecognizeLeadingParagraphBreaks )
{
   auto text = "  \n \n Two.\n\nParagraphs.";
   auto paras = Writer::splitIntoParagraphs( text );

   // Two "full" paragraphs, two empty paragraphs.
   ASSERT_EQ( 4, paras.size() );
   EXPECT_EQ( "", paras[0] );
   EXPECT_EQ( "Two.", paras[1] );
   EXPECT_EQ( "", paras[2] );
   EXPECT_EQ( "Paragraphs.", paras[3] );
}

TEST( WriterInputParagraphTest, shouldRecognizeTrailingParagraphBreaks )
{
   auto text = "Two.\n\nParagraphs.\n\n";
   auto paras = Writer::splitIntoParagraphs( text );

   // Two "full" paragraphs, two empty paragraphs.
   ASSERT_EQ( 4, paras.size() );
   EXPECT_EQ( "Two.", paras[0] );
   EXPECT_EQ( "", paras[1] );
   EXPECT_EQ( "Paragraphs.", paras[2] );
   EXPECT_EQ( "", paras[3] );
}
