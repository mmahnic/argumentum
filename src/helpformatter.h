// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "helpformatter_i.h"

#include "argparser.h"

#include <regex>

namespace argparse {

class Writer
{
   std::ostream& stream;
   size_t position = 0;
   size_t lastWritePosition = 0;
   size_t width = 80;
   bool startOfParagraph = true;
   std::string indent;

public:
   Writer( std::ostream& outStream, size_t widthBytes = 80 )
      : stream( outStream )
      , width( widthBytes )
   {}

   void setIndent( size_t indentBytes )
   {
      if ( indentBytes > width )
         indentBytes = width;
      indent = indentBytes == 0 ? "" : std::string( indentBytes, ' ' );
   }

   void write( std::string_view text )
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

   void startLine()
   {
      if ( position > 0 )
         stream << "\n";
      position = 0;
      lastWritePosition = 0;
      startOfParagraph = false;
   }

   void skipToColumnOrNewLine( size_t column )
   {
      if ( column >= width || column < position )
         startLine();
      else if ( column > position ) {
         stream << std::string( column - position, ' ' );
         position = column;
      }
      startOfParagraph = false;
   }

   void startParagraph()
   {
      if ( !startOfParagraph ) {
         startLine();
         stream << "\n";
         startOfParagraph = true;
      }
   }

   static std::vector<std::string_view> splitIntoWords( std::string_view text )
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

   // Paragraphs are delimited by two or more consecutive newlines intermixed
   // with other whitespace. The paragraph delimiters are returned as empty blocks.
   static std::vector<std::string_view> splitIntoParagraphs( std::string_view text )
   {
      auto rxParagraph = std::regex( "[ \t]*\n[ \t]*\n\\s*" );
      std::vector<std::string_view> res;

      auto it = std::cregex_iterator( text.data(), text.data() + text.size(), rxParagraph );
      auto iend = std::cregex_iterator();
      auto lastPosition = 0;
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

private:
   void write_paragraph( std::string_view text )
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
};

class OptionSorter
{
public:
   struct GroupLimit
   {
      using iterator_t = std::vector<ArgumentHelpResult>::iterator;
      // Start of group in the reordered ArgumentHelpResult vector.
      iterator_t ibegin;
      // End of positional parameters.
      iterator_t iendpos;
      // End of requiered parameters.
      iterator_t iendreq;
      // End of optional parameters, end of group.
      iterator_t iend;
      GroupLimit( iterator_t begin, iterator_t end )
         : ibegin( begin )
         , iendpos( begin )
         , iendreq( begin )
         , iend( end )
      {}
   };

   // 1. required free group
   // 2. required groups by name
   // 3. optional free group
   // 4. optional groups by name
   // 5. commands
   std::vector<GroupLimit> reorderGroups( std::vector<ArgumentHelpResult>& args )
   {
      auto lowerGroup = []( auto&& a, auto&& b ) {
         if ( a.isCommand == b.isCommand ) {
            if ( a.group.isRequired == b.group.isRequired )
               return a.group.name < b.group.name;

            // Required before optional ( false < true => !isRequired(true) < isRequired(true) )
            return !a.group.isRequired < !b.group.isRequired;
         }

         return a.isCommand < b.isCommand;
      };

      std::stable_sort( std::begin( args ), std::end( args ), lowerGroup );

      std::vector<GroupLimit> limits;
      auto iprev = std::begin( args );
      for ( auto icur = iprev; icur != std::end( args ); ++icur ) {
         if ( lowerGroup( *iprev, *icur ) ) {
            limits.emplace_back( iprev, icur );
            iprev = icur;
         }
      }
      if ( iprev != std::end( args ) )
         limits.emplace_back( iprev, std::end( args ) );

      return limits;
   }

   void reorderOptions( GroupLimit& limit )
   {
      limit.iendpos = std::stable_partition(
            limit.ibegin, limit.iend, []( auto&& opt ) { return opt.is_positional(); } );
      limit.iendreq = std::stable_partition(
            limit.iendpos, limit.iend, []( auto&& opt ) { return opt.is_required(); } );
   }
};

inline void HelpFormatter::formatUsage(
      const argument_parser& parser, std::vector<ArgumentHelpResult>& args, Writer& writer )
{
   const auto& config = parser.getConfig();
   if ( !config.program.empty() )
      writer.write( config.program );

   for ( auto& arg : args ) {
      if ( arg.isCommand ) {
         writer.write( "<command> ..." );
         break;
      }

      std::string_view name;

      if ( !arg.is_positional() ) {
         if ( !arg.long_name.empty() )
            name = arg.long_name;
         else if ( !arg.short_name.empty() )
            name = arg.short_name;
      }

      if ( arg.isRequired ) {
         if ( !name.empty() )
            writer.write( name );
         if ( !arg.arguments.empty() )
            writer.write( arg.arguments );
      }
      else {
         if ( !name.empty() || !arg.arguments.empty() ) {
            auto addBracket = !name.empty() || arg.arguments.substr( 0, 1 ) != "[";
            std::ostringstream oss;
            if ( addBracket )
               oss << "[";
            if ( !name.empty() ) {
               oss << name;
               if ( !arg.arguments.empty() )
                  oss << " " << arg.arguments;
            }
            else if ( !arg.arguments.empty() )
               oss << arg.arguments;
            if ( addBracket )
               oss << "]";
            writer.write( oss.str() );
         }
      }
   }
}

inline void HelpFormatter::format( const argument_parser& parser, std::ostream& out )
{
   const auto& config = parser.getConfig();
   auto args = parser.describe_arguments();

   Writer writer( out, mTextWidth );
   writer.write( "usage: " );
   if ( !config.usage.empty() )
      writer.write( config.usage );
   else
      formatUsage( parser, args, writer );
   writer.startParagraph();

   auto desctiptionIndent = deriveMaxArgumentWidth( args ) + mArgumentIndent + 1;
   if ( desctiptionIndent > mMaxDescriptionIndent )
      desctiptionIndent = mMaxDescriptionIndent;

   auto writeArguments = [&]( auto&& writer, auto&& start, auto&& end ) {
      writer.startLine();
      for ( auto it = start; it != end; ++it ) {
         writer.setIndent( mArgumentIndent );
         writer.write( formatArgument( *it ) );
         writer.skipToColumnOrNewLine( desctiptionIndent );
         writer.setIndent( desctiptionIndent );
         writer.write( it->help );
         writer.startLine();
      }
      writer.startParagraph();
      writer.setIndent( 0 );
   };

   OptionSorter sorter;
   auto groups = sorter.reorderGroups( args );
   for ( auto& group : groups )
      sorter.reorderOptions( group );

   if ( !config.description.empty() ) {
      writer.write( config.description );
      writer.startParagraph();
   }

   for ( auto& group : groups ) {
      auto hasPositional = group.ibegin != group.iendpos;
      auto hasRequired = group.iendpos != group.iendreq;
      auto hasOptional = group.iendreq != group.iend;
      auto& firstArg = *group.ibegin;
      auto isDefaultGroup = firstArg.group.name.empty();
      auto isCommand = firstArg.isCommand;

      if ( !isDefaultGroup ) {
         writer.write( firstArg.group.title + ":" );
         writer.startLine();
         writer.setIndent( mArgumentIndent );
         writer.write( firstArg.group.description );
         writer.startParagraph();
         writer.setIndent( 0 );
      }

      if ( hasPositional && !isCommand ) {
         if ( isDefaultGroup )
            writer.write( "positional arguments:" );
         writeArguments( writer, group.ibegin, group.iendpos );
      }

      if ( hasRequired ) {
         if ( isDefaultGroup )
            writer.write( "required arguments:" );
         writeArguments( writer, group.iendpos, group.iendreq );
      }

      if ( hasOptional ) {
         if ( isDefaultGroup )
            writer.write( "optional arguments:" );
         writeArguments( writer, group.iendreq, group.iend );
      }

      if ( hasPositional && isCommand ) {
         // Commands are not options (their names do not start with '-') so they
         // are all in the positional part of the group.
         if ( isDefaultGroup )
            writer.write( "commands:" );
         writeArguments( writer, group.ibegin, group.iendpos );
      }
   }

   if ( !config.epilog.empty() ) {
      writer.write( config.epilog );
      writer.startParagraph();
   }
}

}   // namespace argparse
