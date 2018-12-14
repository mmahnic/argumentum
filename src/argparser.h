#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>

class CArgumentParser
{
public:
   class Value
   {
   public:
      virtual void setValue( const std::string& value ) = 0;
   };

   class String: public Value
   {
      std::optional<std::string>& mValue;
   public:
      String( std::optional<std::string>& value )
         : mValue( value )
      {}

      void setValue( const std::string& value ) override
      {
         mValue = value;
      }
   };

   class Int: public Value
   {
      std::optional<long>& mValue;
   public:
      Int( std::optional<long>& value )
         : mValue( value )
      {}

      void setValue( const std::string& value ) override
      {
         mValue = std::stol( value );
      }
   };

   class CParameter
   {
      friend class CArgumentParser;
   private:
      std::unique_ptr<Value> mpValue;
      std::string mShortName;
      std::string mLongName;

   public:
      CParameter( std::optional<std::string>& value )
         : mpValue( std::make_unique<String>(value) )
      {}

      CParameter( std::optional<long>& value )
         : mpValue( std::make_unique<Int>(value) )
      {}

      CParameter& shortName( const std::string& name )
      {
         mShortName = name;
         return *this;
      }

      CParameter& longName( const std::string& name )
      {
         mLongName = name;
         return *this;
      }
   };

private:
   std::vector<CParameter> mParameters;

public:
   CParameter& addParameter( std::optional<std::string>& value )
   {
      mParameters.emplace_back( value );
      return mParameters.back();
   }

   CParameter& addParameter( std::optional<long>& value )
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
               mParameters[iparam].mpValue->setValue( arg );
         }
         else {
            iparam = -1;
            for ( int i = 0; i < mParameters.size(); ++i ) {
               auto& param = mParameters[i];
               if ( param.mShortName == name || param.mLongName == name )
                  iparam = i;
            }
         }
      }
   }
};
