// Copyright (c) 2018, 2019, 2020 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#ifndef ARGUMENTUM_BUILD_MODULE
#include "nomodule.h"

#include <stdexcept>
#include <string>
#endif

namespace argumentum {

ARGUMENTUM_EXPORT
class InvalidChoiceError : public std::invalid_argument
{
public:
   InvalidChoiceError( std::string_view value )
      : std::invalid_argument( std::string{ value } )
   {}
};

ARGUMENTUM_EXPORT
class UnsupportedTargetType : public std::invalid_argument
{
public:
   UnsupportedTargetType( std::string_view value )
      : std::invalid_argument( std::string{ value } )
   {}
};

ARGUMENTUM_EXPORT
class UncheckedParseResult : public std::exception
{
public:
   const char* what() const noexcept override
   {
      return "Unchecked parse result.";
   }
};

ARGUMENTUM_EXPORT
class MixingGroupTypes : public std::runtime_error
{
public:
   MixingGroupTypes( const std::string& groupName )
      : runtime_error( std::string( "Mixing group types in group '" ) + groupName + "'." )
   {}
};

ARGUMENTUM_EXPORT
class RequiredExclusiveOption : public std::runtime_error
{
public:
   RequiredExclusiveOption( const std::string& groupName, const std::string& optionName )
      : runtime_error( std::string( "Option '" ) + optionName + "' is required in exclusive group '"
              + groupName + "'." )
   {}
};

ARGUMENTUM_EXPORT
class DuplicateOption : public std::runtime_error
{
public:
   DuplicateOption( const std::string& groupName, const std::string& optionName )
      : runtime_error( std::string( "Option '" ) + optionName + "' is already defined in group '"
              + groupName + "'." )
   {}
};

ARGUMENTUM_EXPORT
class DuplicateCommand : public std::runtime_error
{
public:
   DuplicateCommand( const std::string& commandName )
      : runtime_error( std::string( "Command '" ) + commandName + "' is already defined." )
   {}
};

ARGUMENTUM_EXPORT
class MissingCommandOptions : public std::runtime_error
{
public:
   MissingCommandOptions( const std::string& commandName )
      : runtime_error( std::string( "Command '" ) + commandName + "' has no options." )
   {}
};

ARGUMENTUM_EXPORT
class MissingFilesystem : public std::exception
{
public:
   const char* what() const noexcept override
   {
      return "The filesystem for opening argument streams is not defined.";
   }
};

ARGUMENTUM_EXPORT
class IncludeDepthExceeded : public std::runtime_error
{
public:
   IncludeDepthExceeded( const std::string& streamName )
      : runtime_error( streamName )
   {}
};

}   // namespace argumentum
