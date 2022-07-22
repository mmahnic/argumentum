// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#ifndef ARGUMENTUM_BUILD_MODULE
#include <ostream>
#include <regex>
#include <string>
#include <string_view>
#include <vector>
#endif

namespace argumentum {

class Writer
{
   std::ostream& stream;
   size_t position = 0;
   size_t lastWritePosition = 0;
   size_t width = 80;
   bool startOfParagraph = true;
   std::string indent;

public:
   Writer( std::ostream& outStream, size_t widthBytes = 80 );

   void setIndent( size_t indentBytes );
   void write( std::string_view text );
   void startLine();
   void skipToColumnOrNewLine( size_t column );
   void startParagraph();
   static std::vector<std::string_view> splitIntoWords( std::string_view text );

   // Paragraphs are delimited by two or more consecutive newlines intermixed
   // with other whitespace. The paragraph delimiters are returned as empty blocks.
   static std::vector<std::string_view> splitIntoParagraphs( std::string_view text );

private:
   void write_paragraph( std::string_view text );
};

}   // namespace argumentum
