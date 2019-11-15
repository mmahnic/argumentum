// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <fstream>
#include <memory>
#include <string>

class Filesystem
{
public:
   virtual std::unique_ptr<std::istream> open( const std::string& filename ) = 0;
};

class DefaultFilesystem : public Filesystem
{
public:
   std::unique_ptr<std::istream> open( const std::string& filename ) override
   {
      return std::make_unique<std::ifstream>( filename );
   }
};
