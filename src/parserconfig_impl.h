// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "helpformatter.h"
#include "parserconfig.h"

#include <ostream>
#include <string>
#include <string_view>

namespace argumentum {

ARGUMENTUM_INLINE const ParserConfig::Data& ParserConfig::data() const
{
   return mData;
}

ARGUMENTUM_INLINE ParserConfig& ParserConfig::program( std::string_view program )
{
   mData.mProgram = program;
   return *this;
}

ARGUMENTUM_INLINE ParserConfig& ParserConfig::usage( std::string_view usage )
{
   mData.mUsage = usage;
   return *this;
}

ARGUMENTUM_INLINE ParserConfig& ParserConfig::description( std::string_view description )
{
   mData.mDescription = description;
   return *this;
}

ARGUMENTUM_INLINE ParserConfig& ParserConfig::epilog( std::string_view epilog )
{
   mData.mEpilog = epilog;
   return *this;
}

ARGUMENTUM_INLINE ParserConfig& ParserConfig::cout( std::ostream& stream )
{
   mData.mpOutStream = &stream;
   return *this;
}

ARGUMENTUM_INLINE ParserConfig& ParserConfig::filesystem( std::shared_ptr<Filesystem> pFilesystem )
{
   mData.mpFilesystem = std::move( pFilesystem );
   return *this;
}

ARGUMENTUM_INLINE ParserConfig& ParserConfig::help_formatter(
      std::shared_ptr<IFormatHelp> pFormatter )
{
   mData.mpHelpFormatter = std::move( pFormatter );
   return *this;
}

ARGUMENTUM_INLINE const std::string& ParserConfig::Data::program() const
{
   return mProgram;
}

ARGUMENTUM_INLINE const std::string& ParserConfig::Data::usage() const
{
   return mUsage;
}

ARGUMENTUM_INLINE const std::string& ParserConfig::Data::description() const
{
   return mDescription;
}

ARGUMENTUM_INLINE const std::string& ParserConfig::Data::epilog() const
{
   return mEpilog;
}

ARGUMENTUM_INLINE unsigned ParserConfig::Data::max_include_depth() const
{
   return mMaxIncludeDepth;
}

ARGUMENTUM_INLINE std::ostream* ParserConfig::Data::output_stream() const
{
   return mpOutStream ? mpOutStream : &std::cout;
}

ARGUMENTUM_INLINE std::shared_ptr<IFormatHelp> ParserConfig::Data::help_formatter(
      const std::string& helpOption ) const
{
   return mpHelpFormatter ? mpHelpFormatter : std::make_shared<HelpFormatter>();
}

ARGUMENTUM_INLINE std::shared_ptr<Filesystem> ParserConfig::Data::filesystem() const
{
   return mpFilesystem ? mpFilesystem : std::make_shared<DefaultFilesystem>();
}

}   // namespace argumentum
