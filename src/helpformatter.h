// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "iformathelp.h"

#ifndef ARGUMENTUM_BUILD_MODULE
#include "nomodule.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#endif

namespace argumentum {

class Writer;

ARGUMENTUM_EXPORT
class HelpFormatter : public IFormatHelp
{
   // The number of spaces before argument names.
   size_t mArgumentIndent = 2;

   // The width of the formatted text in bytes.
   size_t mTextWidth = 80;

   // The maximum width of an argument at which the description of the argument
   // can start in the same line.
   size_t mMaxDescriptionIndent = 30;

public:
   void format( const ParserDefinition& parserDef, std::ostream& out ) override;

   void setTextWidth( size_t widthBytes )
   {
      mTextWidth = widthBytes;
   }

   void setMaxDescriptionIndent( size_t widthBytes )
   {
      mMaxDescriptionIndent = widthBytes;
   }

private:
   void formatUsage(
         const ParserDefinition& parser, std::vector<ArgumentHelpResult>& args, Writer& writer );
   std::string formatArgument( const ArgumentHelpResult& arg ) const;
   size_t deriveMaxArgumentWidth( const std::vector<ArgumentHelpResult>& args ) const;
};

}   // namespace argumentum
