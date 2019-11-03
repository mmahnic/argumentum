// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace argparse {

class ParserConfig
{
public:
   struct Data
   {
      std::string program;
      std::string usage;
      std::string description;
      std::string epilog;
      std::ostream* pOutStream = nullptr;
   };

private:
   Data mData;

public:
   const Data& data() const
   {
      return mData;
   }

   ParserConfig& program( std::string_view program )
   {
      mData.program = program;
      return *this;
   }

   ParserConfig& usage( std::string_view usage )
   {
      mData.usage = usage;
      return *this;
   }

   ParserConfig& description( std::string_view description )
   {
      mData.description = description;
      return *this;
   }

   ParserConfig& epilog( std::string_view epilog )
   {
      mData.epilog = epilog;
      return *this;
   }

   // NOTE: The @p stream must outlive the parser.
   ParserConfig& cout( std::ostream& stream )
   {
      mData.pOutStream = &stream;
      return *this;
   }
};

}   // namespace argparse
