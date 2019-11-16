// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include <stdexcept>
#include <string>

namespace argparse {

class InvalidChoiceError : public std::invalid_argument
{
public:
   InvalidChoiceError( std::string_view value )
      : std::invalid_argument( std::string{ value } )
   {}
};

class UncheckedParseResult : public std::exception
{
public:
   const char* what() const noexcept override
   {
      return "Unchecked parse result.";
   }
};

class MixingGroupTypes : public std::runtime_error
{
public:
   MixingGroupTypes( const std::string& groupName )
      : runtime_error( std::string( "Mixing group types in group '" ) + groupName + "'." )
   {}
};

class RequiredExclusiveOption : public std::runtime_error
{
public:
   RequiredExclusiveOption( const std::string& groupName, const std::string& optionName )
      : runtime_error( std::string( "Option '" ) + optionName + "' is required in exclusive group '"
              + groupName + "'." )
   {}
};

class DuplicateOption : public std::runtime_error
{
public:
   DuplicateOption( const std::string& groupName, const std::string& optionName )
      : runtime_error( std::string( "Option '" ) + optionName + "' is already defined in group '"
              + groupName + "'." )
   {}
};

class DuplicateCommand : public std::runtime_error
{
public:
   DuplicateCommand( const std::string& commandName )
      : runtime_error( std::string( "Command '" ) + commandName + "' is already defined." )
   {}
};

}   // namespace argparse
