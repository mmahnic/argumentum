// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "helpformatter.h"

#include "argdescriber.h"
#include "optionsorter.h"
#include "parser.h"
#include "writer.h"

#include <sstream>

namespace argumentum {

ARGUMENTUM_INLINE std::string HelpFormatter::formatArgument( const ArgumentHelpResult& arg ) const
{
   if ( arg.isCommand )
      return arg.help_name;
   else if ( arg.is_positional() )
      return arg.help_name;

   std::string res;
   if ( !arg.short_name.empty() && !arg.long_name.empty() )
      res = arg.short_name + ", " + arg.long_name;
   else if ( !arg.short_name.empty() )
      res = arg.short_name;
   else if ( !arg.long_name.empty() )
      res = "    " + arg.long_name;

   return !arg.arguments.empty() ? res + " " + arg.arguments : res;
}

ARGUMENTUM_INLINE size_t HelpFormatter::deriveMaxArgumentWidth(
      const std::vector<ArgumentHelpResult>& args ) const
{
   if ( args.empty() )
      return 0U;

   auto imax =
         std::max_element( std::begin( args ), std::end( args ), [this]( auto&& a, auto&& b ) {
            return formatArgument( a ).size() < formatArgument( b ).size();
         } );

   return formatArgument( *imax ).size();
}

ARGUMENTUM_INLINE void HelpFormatter::formatUsage(
      const ParserDefinition& parserDef, std::vector<ArgumentHelpResult>& args, Writer& writer )
{
   const auto& config = parserDef.getConfig();
   if ( !config.program().empty() )
      writer.write( config.program() );

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

ARGUMENTUM_INLINE void HelpFormatter::format( const ParserDefinition& parserDef, std::ostream& out )
{
   const auto& config = parserDef.getConfig();
   ArgumentDescriber describer;
   auto args = describer.describe_arguments( parserDef );

   Writer writer( out, mTextWidth );
   writer.write( "usage: " );
   if ( !config.usage().empty() )
      writer.write( config.usage() );
   else
      formatUsage( parserDef, args, writer );
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

   if ( !config.description().empty() ) {
      writer.write( config.description() );
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

   if ( !config.epilog().empty() ) {
      writer.write( config.epilog() );
      writer.startParagraph();
   }
}

}   // namespace argumentum
