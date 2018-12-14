#pragma once

#include <string>
#include <vector>

class CArgumentParser
{
public:
   class CParameter
   {
      std::string& mValue;
      std::string mShortName;
   public:
      CParameter( std::string& value )
         : mValue( value )
      { }

      CParameter& shortName( const std::string& name )
      {
         mShortName = name;
         return *this;
      }
   };

private:
   std::vector<CParameter> mParameters;

public:
   CParameter& addParameter( std::string& value )
   {
      mParameters.emplace_back( value );
      return mParameters.back();
   }

   void parseArguments( const std::vector<std::string>& args )
   {
   }
};
