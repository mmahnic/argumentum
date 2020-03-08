// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

namespace argumentum {

ARGUMENTUM_INLINE Writer::Writer( std::ostream& outStream, size_t widthBytes )
   : stream( outStream )
   , width( widthBytes )
{}

ARGUMENTUM_INLINE void Writer::setIndent( size_t indentBytes )
{
   if ( indentBytes > width )
      indentBytes = width;
   indent = indentBytes == 0 ? "" : std::string( indentBytes, ' ' );
}

ARGUMENTUM_INLINE void Writer::write( std::string_view text )
{
   auto blocks = splitIntoParagraphs( text );
   for ( auto block : blocks ) {
      if ( block.empty() )
         startParagraph();
      else {
         write_paragraph( block );
         startOfParagraph = false;
      }
   }
}

ARGUMENTUM_INLINE void Writer::startLine()
{
   if ( position > 0 )
      stream << "\n";
   position = 0;
   lastWritePosition = 0;
   startOfParagraph = false;
}

ARGUMENTUM_INLINE void Writer::skipToColumnOrNewLine( size_t column )
{
   if ( column >= width || column < position )
      startLine();
   else if ( column > position ) {
      stream << std::string( column - position, ' ' );
      position = column;
   }
   startOfParagraph = false;
}

ARGUMENTUM_INLINE void Writer::startParagraph()
{
   if ( !startOfParagraph ) {
      startLine();
      stream << "\n";
      startOfParagraph = true;
   }
}

ARGUMENTUM_INLINE std::vector<std::string_view> Writer::splitIntoWords( std::string_view text )
{
   std::vector<std::string_view> words;

   size_t pos = 0;
   while ( pos < text.size() ) {
      while ( pos < text.size() && iswspace( text[pos] ) )
         ++pos;

      size_t end = pos;
      while ( end < text.size() && !iswspace( text[end] ) )
         ++end;

      if ( end > pos )
         words.push_back( text.substr( pos, end - pos ) );

      pos = end;
   }

   return words;
}

ARGUMENTUM_INLINE std::vector<std::string_view> Writer::splitIntoParagraphs( std::string_view text )
{
   auto rxParagraph = std::regex( "[ \t]*\n[ \t]*\n\\s*" );
   std::vector<std::string_view> res;

   auto it = std::cregex_iterator( text.data(), text.data() + text.size(), rxParagraph );
   auto iend = std::cregex_iterator();
   auto lastPosition = 0U;
   for ( ; it != iend; ++it ) {
      auto match = std::cmatch( *it );
      if ( match.position() == 0 )
         res.emplace_back();
      else {
         res.push_back( text.substr( lastPosition, match.position() - lastPosition ) );
         res.emplace_back();
      }
      lastPosition = match.position() + match.length();
   }

   if ( lastPosition < text.size() )
      res.push_back( text.substr( lastPosition ) );
   return res;
}

ARGUMENTUM_INLINE void Writer::write_paragraph( std::string_view text )
{
   auto words = splitIntoWords( text );
   for ( auto word : words ) {
      auto newpos = position + ( position == 0 ? indent.size() : 1 ) + word.size();
      if ( newpos > width )
         startLine();
      else if ( position > 0 && position == lastWritePosition ) {
         stream << " ";
         ++position;
      }

      if ( position == 0 && indent.size() > 0 ) {
         stream << indent;
         position = indent.size();
      }

      stream.write( word.data(), word.size() );
      position += word.size();
      lastWritePosition = position;
   }
}

}   // namespace argumentum
