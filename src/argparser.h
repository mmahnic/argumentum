#pragma once

#include <string>
#include <vector>

class CArgumentParser
{
public:
   class CParameter
   {
      friend class CArgumentParser;
   private:
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
      int iparam = -1;
      for ( auto& arg : args ) {
         std::string name;
         if ( arg.substr( 0, 2 ) == "--" )
            name = arg.substr( 2 );
         else if ( arg.substr( 0, 1 ) == "-" )
            name = arg.substr( 1 );

         if ( name.empty() ) {
            if ( iparam >= 0 )
               mParameters[iparam].mValue = arg;
         }
         else {
            iparam = -1;
            for ( int i = 0; i < mParameters.size(); ++i ) {
               auto& param = mParameters[i];
               if ( param.mShortName == name )
                  iparam = i;
            }
         }
      }
   }
};
