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
      bool mValueWasSet = false;
   public:
      void setValue( const std::string& value )
      {
         mValueWasSet = true;
         doSetValue( value );
      }

      bool hasValue() const
      {
         return mValueWasSet;
      }

   protected:
      virtual void doSetValue( const std::string& value ) = 0;
   };

   class String: public Value
   {
      std::optional<std::string>& mValue;
   public:
      String( std::optional<std::string>& value )
         : mValue( value )
      {}

   protected:
      void doSetValue( const std::string& value ) override
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

   protected:
      void doSetValue( const std::string& value ) override
      {
         mValue = std::stol( value );
      }
   };

   class Option
   {
   private:
      std::unique_ptr<Value> mpValue;
      std::string mShortName;
      std::string mLongName;
      bool mHasArgument = false;
      bool mIsRequired = false;

   public:
      Option( std::optional<std::string>& value )
         : mpValue( std::make_unique<String>(value) )
      {}

      Option( std::optional<long>& value )
         : mpValue( std::make_unique<Int>(value) )
      {}

      Option& shortName( const std::string& name )
      {
         mShortName = name;
         return *this;
      }

      Option& longName( const std::string& name )
      {
         mLongName = name;
         return *this;
      }

      Option& hasArgument( bool hasArg=true )
      {
         mHasArgument = hasArg;
         return *this;
      }

      Option& required( bool isRequired=true )
      {
         mIsRequired = isRequired;
         return *this;
      }

      bool isRequired() const
      {
         return mIsRequired;
      }

      bool isArgumentExpected() const
      {
         return mHasArgument;
      }

      const std::string& name() const
      {
         return mLongName.empty() ? mShortName : mLongName;
      }

      bool hasName( std::string_view name ) const
      {
         return name == mShortName || name == mLongName;
      }

      void setValue( const std::string& value )
      {
         mpValue->setValue( value );
      }

      bool hasValue() const
      {
         return mpValue->hasValue();
      }
   };

   // Errors known by the parser
   enum EError {
      UNKNOWN_OPTION,
      MISSING_OPTION,
      MISSING_ARGUMENT,
      CONVERSION_ERROR
   };

   struct ParseError
   {
      std::string option;
      int errorCode;
      ParseError( std::string_view optionName, int code )
         : option( optionName ), errorCode( code )
      {}
   };

   struct ParseResult
   {
      std::vector<std::string> freeArguments;
      std::vector<ParseError> errors;

   public:
      void clear()
      {
         freeArguments.clear();
         errors.clear();
      }
   };

private:
   class Parser
   {
      CArgumentParser& mArgParser;
      bool mIgnoreOptions = false;
      // The active option will receive additional argument(s)
      int mActiveOption = -1;
      ParseResult mResult;

   public:
      Parser( CArgumentParser& argParser )
         : mArgParser( argParser )
      {}

      void startOption( std::string_view name )
      {
         if ( mActiveOption >= 0 ) {
            auto& option = mArgParser.mOptions[mActiveOption];
            if ( !option.hasValue() ) {
               addError( option.name(), MISSING_ARGUMENT );
               closeOption();
               return;
            }
         }

         auto nopt = mArgParser.mOptions.size();
         for ( unsigned i = 0; i < nopt; ++i ) {
            auto& option = mArgParser.mOptions[i];
            if ( option.hasName( name ) ) {
               if ( option.isArgumentExpected() )
                  mActiveOption = i;
               else
                  setValue( option, "1" );
               return;
            }
         }
         addError( name, UNKNOWN_OPTION );
         closeOption();
      }

      void closeOption()
      {
         mActiveOption = -1;
      }

      void addFreeArgument( const std::string& arg )
      {
         mResult.freeArguments.push_back( arg );
      }

      void addError( std::string_view optionName, int errorCode )
      {
         mResult.errors.emplace_back( optionName, errorCode );
      }

      void setValue( Option& option, const std::string& value )
      {
         try {
            option.setValue( value );
         }
         catch( std::invalid_argument ) {
            addError( option.name(), CONVERSION_ERROR );
         }
         catch( std::out_of_range ) {
            addError( option.name(), CONVERSION_ERROR );
         }
      }

      ParseResult parse( const std::vector<std::string>& args )
      {
         mResult.clear();
         for ( auto& arg : args ) {
            if ( arg == "--" ) {
               mIgnoreOptions = true;
               continue;
            }

            if ( mIgnoreOptions ){
               addFreeArgument( arg );
               continue;
            }

            auto arg_view = std::string_view( arg );
            if ( arg_view.substr( 0, 2 ) == "--" )
               startOption( arg.substr( 2 ) );
            else if ( arg_view.substr( 0, 1 ) == "-" ) {
               for ( int i = 1; i < arg_view.size(); ++i )
                  startOption( arg_view.substr( i, 1 ));
            }
            else {
               if ( mActiveOption >= 0 ) {
                  auto& option = mArgParser.mOptions[mActiveOption];
                  if ( option.isArgumentExpected() )
                     setValue(option, arg );
                  // NOTE: For now we assume there is at most one argument per option
                  closeOption();
               }
               else
                  addFreeArgument( arg );
            }
         }

         return std::move( mResult );
      }
   };

private:
   std::vector<Option> mOptions;

public:
   Option& addOption( std::optional<std::string>& value )
   {
      mOptions.emplace_back( value );
      return mOptions.back();
   }

   Option& addOption( std::optional<long>& value )
   {
      mOptions.emplace_back( value );
      return mOptions.back();
   }

   ParseResult parseArguments( const std::vector<std::string>& args )
   {
      Parser parser( *this );
      auto result = parser.parse( args );
      reportMissingOptions( result );
      return result;
   }

private:
   void reportMissingOptions( ParseResult& result )
   {
      for ( auto& option : mOptions )
         if ( option.isRequired() && !option.hasValue() )
            result.errors.emplace_back( option.name(), MISSING_OPTION );
   }
};
