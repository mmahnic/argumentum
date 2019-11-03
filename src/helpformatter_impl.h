// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "helpformatter.h"

#include "argparser.h"
#include "optionsorter.h"
#include "writer.h"

namespace argparse {

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
