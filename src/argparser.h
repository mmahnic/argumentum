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

   class COption
   {
      friend class CArgumentParser;
   private:
      std::unique_ptr<Value> mpValue;
      std::string mShortName;
      std::string mLongName;
      bool mHasArgument = false;

   public:
      COption( std::optional<std::string>& value )
         : mpValue( std::make_unique<String>(value) )
      {}

      COption( std::optional<long>& value )
         : mpValue( std::make_unique<Int>(value) )
      {}

      COption& shortName( const std::string& name )
      {
         mShortName = name;
         return *this;
      }

      COption& longName( const std::string& name )
      {
         mLongName = name;
         return *this;
      }

      COption& hasArgument( bool hasArg=true )
      {
         mHasArgument = hasArg;
         return *this;
      }
   };

private:
   std::vector<COption> mOptions;

public:
   COption& addOption( std::optional<std::string>& value )
   {
      mOptions.emplace_back( value );
      return mOptions.back();
   }

   COption& addOption( std::optional<long>& value )
   {
      mOptions.emplace_back( value );
      return mOptions.back();
   }

   void parseArguments( const std::vector<std::string>& args )
   {
      bool ignoreOptions = false;
      int iparam = -1;

      auto handleOption = [this, &iparam]( std::string_view name ) {
         iparam = -1;
         for ( int i = 0; i < mOptions.size(); ++i ) {
            auto& option = mOptions[i];
            if ( option.mShortName == name || option.mLongName == name ) {
               if ( option.mHasArgument )
                  iparam = i;
               else
                  option.mpValue->setValue( "1" );
            }
         }
      };

      for ( auto& arg : args ) {
         if ( arg == "--" ) {
            ignoreOptions = true;
            continue;
         }

         if ( ignoreOptions ){
            addFreeArgument( arg );
            continue;
         }

         auto arg_view = std::string_view( arg );
         if ( arg_view.substr( 0, 2 ) == "--" )
            handleOption( arg.substr( 2 ) );
         else if ( arg_view.substr( 0, 1 ) == "-" ) {
            for ( int i = 1; i < arg_view.size(); ++i )
               handleOption( arg_view.substr( i, 1 ));
         }
         else {
            if ( iparam >= 0 && mOptions[iparam].mHasArgument )
               mOptions[iparam].mpValue->setValue( arg );
            else
               addFreeArgument( arg );

            // NOTE: For now we assume ther is at most one argument per option
            iparam = -1;
         }

      }
   }

private:
   void addFreeArgument( const std::string& arg )
   {
      // TODO: add arg to the list of free arguments
   }
};
