// Copyright (c) 2018 Marko Mahnič
// License: MIT. See LICENSE in the root of the project.

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <algorithm>

namespace argparse {

template<typename TValue>
struct convert_result
{
   using type = TValue;
};

template<typename TItem>
struct convert_result<std::optional<TItem>>
{
   using type = TItem;
};

template<typename TItem>
struct convert_result<std::vector<TItem>>
{
   using type = TItem;
};


class ArgumentParser
{
public:
   class Value
   {
      int mAssignCount = 0;
      int mOptionAssignCount = 0;
   public:
      void setValue( const std::string& value )
      {
         ++mAssignCount;
         ++mOptionAssignCount;
         doSetValue( value );
      }

      /**
       * The count of assignments through all the options that share this value.
       */
      int getAssignCount() const
      {
         return mAssignCount;
      }

      /**
       * The count of assignments through the current option.
       */
      int getOptionAssignCount() const
      {
         return mOptionAssignCount;
      }

      void onOptionStarted()
      {
         mOptionAssignCount = 0;
      }

   protected:
      virtual void doSetValue( const std::string& value ) = 0;
   };

   template<typename TValue>
   class ConvertedValue: public Value
   {
   protected:
      using result_t = typename convert_result<TValue>::type;
      using converter_t = std::function<result_t(const std::string&)>;
      TValue& mValue;
      converter_t mConvert = []( const std::string& ) { return {}; };

   public:
      ConvertedValue( TValue& value, converter_t converter )
         : mValue( value ), mConvert( converter )
      {}

   protected:
      void doSetValue( const std::string& value ) override
      {
         assign( mValue, value );
      }

      template<typename TVar>
      void assign( TVar& var, const std::string& value )
      {
         var = mConvert( value );
      }

      template<typename TVar>
      void assign( std::vector<TVar>& var, const std::string& value )
      {
         var.push_back( mConvert( value ) );
      }
   };

   class Option
   {
   private:
      std::unique_ptr<Value> mpValue;
      std::string mShortName;
      std::string mLongName;
      std::string mFlagValue = "1";
      int mMinArgs = 0;
      int mMaxArgs = 0;
      bool mIsRequired = false;

   public:
      template<typename TValue>
      Option( TValue& value )
      {
         if constexpr ( std::is_base_of<Value, TValue>::value ) {
            mpValue = std::make_unique<TValue>( value );
         }
         else if constexpr ( std::is_same<std::string, TValue>::value
               || std::is_same<std::optional<std::string>, TValue>::value ) {
            using wrap_type = ConvertedValue<TValue>;
            mpValue = std::make_unique<wrap_type>( value, []( const std::string& s ) { return s; } );
         }
         else if constexpr ( std::is_same<long, TValue>::value
               || std::is_same<std::optional<long>, TValue>::value ) {
            using wrap_type = ConvertedValue<TValue>;
            mpValue = std::make_unique<wrap_type>( value, []( const std::string& s ) { return stol( s ); } );
         }
         else if constexpr ( std::is_same<double, TValue>::value
               || std::is_same<std::optional<double>, TValue>::value ) {
            using wrap_type = ConvertedValue<TValue>;
            mpValue = std::make_unique<wrap_type>( value, []( const std::string& s ) { return stod( s ); } );
         }
         else {
            using wrap_type = ConvertedValue<std::string>;
            mpValue = std::make_unique<wrap_type>( value, []( const std::string& s ) { return s; } );
         }
      }

      template<typename TValue>
      Option( std::vector<TValue>& value )
      {
         using val_vector = std::vector<TValue>;
         if constexpr ( std::is_base_of<Value, TValue>::value ) {
            mpValue = std::make_unique<val_vector>( value );
         }
         else if constexpr ( std::is_same<std::string, TValue>::value
               || std::is_same<std::optional<std::string>, TValue>::value ) {
            using wrap_type = ConvertedValue<val_vector>;
            mpValue = std::make_unique<wrap_type>( value, []( const std::string& s ) { return s; } );
         }
         else if constexpr ( std::is_same<long, TValue>::value
               || std::is_same<std::optional<long>, TValue>::value ) {
            using wrap_type = ConvertedValue<val_vector>;
            mpValue = std::make_unique<wrap_type>( value, []( const std::string& s ) { return stol( s ); } );
         }
         else if constexpr ( std::is_same<double, TValue>::value
               || std::is_same<std::optional<double>, TValue>::value ) {
            using wrap_type = ConvertedValue<val_vector>;
            mpValue = std::make_unique<wrap_type>( value, []( const std::string& s ) { return stod( s ); } );
         }
         else {
            using wrap_type = ConvertedValue<std::string>;
            mpValue = std::make_unique<wrap_type>( value, []( const std::string& s ) { return s; } );
         }
      }

      Option& setShortName( std::string_view name )
      {
         mShortName = name;
         return *this;
      }

      Option& setLongName( std::string_view name )
      {
         mLongName = name;
         return *this;
      }

      Option& nargs( int count )
      {
         if ( count >= 0 ) {
            mMinArgs = count;
            mMaxArgs = count;
         }
         else {
            mMinArgs = 0;
            mMaxArgs = -1;
         }
      }

      Option& minargs( int count )
      {
         mMinArgs = std::max( 0, count );
         mMaxArgs = -1;
      }

      Option& maxargs( int count )
      {
         mMinArgs = 0;
         mMaxArgs = std::max( 0, count );
      }

      Option& hasArgument( bool hasArg=true )
      {
         nargs( 1 );
         return *this;
      }

      Option& required( bool isRequired=true )
      {
         mIsRequired = isRequired;
         return *this;
      }

      Option& flagValue( std::string_view value )
      {
         mFlagValue = value;
         return *this;
      }

      bool isRequired() const
      {
         return mIsRequired;
      }

      const std::string& getName() const
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

      void onOptionStarted()
      {
         mpValue->onOptionStarted();
      }

      bool willAcceptArgument() const
      {
         return mMaxArgs < 0 || mpValue->getOptionAssignCount() < mMaxArgs;
      }

      bool needsMoreArguments() const
      {
         return mpValue->getOptionAssignCount() < mMinArgs;
      }

      /**
       * @returns true if the value was assigned through any option that shares
       * this option's value.
       */
      bool wasAssigned() const
      {
         return mpValue->getAssignCount() > 0;
      }

      const std::string& getFlagValue() const
      {
         return mFlagValue;
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
      std::vector<std::string> ignoredArguments;
      std::vector<ParseError> errors;

   public:
      void clear()
      {
         ignoredArguments.clear();
         errors.clear();
      }
   };

private:
   class Parser
   {
      ArgumentParser& mArgParser;
      bool mIgnoreOptions = false;
      int mPosition = 0;
      // The active option will receive additional argument(s)
      Option* mpActiveOption = nullptr;
      ParseResult mResult;

   public:
      Parser( ArgumentParser& argParser )
         : mArgParser( argParser )
      {}

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
               if ( haveActiveOption() ) {
                  auto& option = *mpActiveOption;
                  if ( option.willAcceptArgument() ) {
                     setValue(option, arg );
                     if ( !option.willAcceptArgument() )
                        closeOption();
                  }
               }
               else
                  addFreeArgument( arg );
            }
         }

         if ( haveActiveOption() )
            closeOption();

         return std::move( mResult );
      }

   private:
      void startOption( std::string_view name )
      {
         if ( haveActiveOption() )
            closeOption();

         auto pOption = findOption( name );
         if ( pOption ) {
            auto& option = *pOption;
            option.onOptionStarted();
            if ( option.willAcceptArgument() )
               mpActiveOption = pOption;
            else
               setValue( option, option.getFlagValue() );
         }
         else
            addError( name, UNKNOWN_OPTION );
      }

      bool haveActiveOption() const
      {
         return mpActiveOption != nullptr;
      }

      void closeOption()
      {
         if ( haveActiveOption() ) {
            auto& option = *mpActiveOption;
            if ( option.needsMoreArguments() )
               addError( option.getName(), MISSING_ARGUMENT );
         }
         mpActiveOption = nullptr;
      }

      void addFreeArgument( const std::string& arg )
      {
         if ( mPosition < mArgParser.mPositional.size() ) {
            auto& option = mArgParser.mPositional[mPosition];
            if ( option.willAcceptArgument() ) {
               setValue( option, arg );
               return;
            }
            else {
               ++mPosition;
               while ( mPosition < mArgParser.mPositional.size() ) {
                  auto& option = mArgParser.mPositional[mPosition];
                  if ( option.willAcceptArgument() ) {
                     setValue( option, arg );
                     return;
                  }
                  ++mPosition;
               }
            }
         }

         mResult.ignoredArguments.push_back( arg );
      }

      void addError( std::string_view optionName, int errorCode )
      {
         mResult.errors.emplace_back( optionName, errorCode );
      }

      Option* findOption( std::string_view optionName ) const
      {
         for ( auto& option : mArgParser.mOptions )
            if ( option.hasName( optionName ) )
               return &option;

         return nullptr;
      }

      void setValue( Option& option, const std::string& value )
      {
         try {
            option.setValue( value );
         }
         catch( std::invalid_argument ) {
            addError( option.getName(), CONVERSION_ERROR );
         }
         catch( std::out_of_range ) {
            addError( option.getName(), CONVERSION_ERROR );
         }
      }
   };

private:
   std::vector<Option> mOptions;
   std::vector<Option> mPositional;

public:
   // TODO: Create a constructor that takes a shared pointer to a structure and
   // verify that the variables added with addOption are within that structure.
   /**
    * The argument parser takes references to the variables that will hold the
    * parsed values.  The variables must outlive the argument parser.
    */
   static ArgumentParser unsafe()
   {
      return ArgumentParser();
   }

   template<typename TValue, typename = std::enable_if_t<std::is_base_of<Value, TValue>::value> >
   Option& addOption( TValue value, const std::string& name="", const std::string& altName="" )
   {
      auto option = Option( value );
      return tryAddOption( option, { name, altName } );
   }

   template<typename TValue, typename = std::enable_if_t<!std::is_base_of<Value, TValue>::value> >
   Option& addOption( TValue &value, const std::string& name="", const std::string& altName="" )
   {
      auto option = Option( value );
      return tryAddOption( option, { name, altName } );
   }

   ParseResult parseArguments( const std::vector<std::string>& args )
   {
      Parser parser( *this );
      auto result = parser.parse( args );
      reportMissingOptions( result );
      return result;
   }

private:
   ArgumentParser() = default;

   void reportMissingOptions( ParseResult& result )
   {
      for ( auto& option : mOptions )
         if ( option.isRequired() && !option.wasAssigned() )
            result.errors.emplace_back( option.getName(), MISSING_OPTION );

      for ( auto& option : mPositional )
         if ( option.needsMoreArguments() )
            result.errors.emplace_back( option.getName(), MISSING_ARGUMENT );
   }

   Option& tryAddOption( Option& newOption, std::vector<std::string_view> names )
   {
      auto strip = []( std::string_view name ) {
         name.remove_prefix(std::min(name.find_first_not_of(" "), name.size()));
         name.remove_suffix(name.size() - std::min(name.find_last_not_of(" ") + 1, name.size()));
         return name;
      };
      auto isEmpty = [&]( auto name ) { return strip( name ) == ""; };

      // Remove empty names
      names.erase( std::remove_if( names.begin(), names.end(), isEmpty ), names.end() );

      if ( names.empty() )
         throw std::invalid_argument( "An argument must have a name." );

      auto hasDash = []( auto name ) { return name[0] == '-'; };

      auto isOption = [&]( auto names ) -> bool {
         return std::all_of( names.begin(), names.end(), hasDash );
      };

      auto isPositional = [&]( auto names ) -> bool {
         return std::none_of( names.begin(), names.end(), hasDash );
      };

      if ( isPositional( names ) ) {
         mPositional.push_back( std::move(newOption) );
         auto& option = mPositional.back();
         option.setLongName( names.empty() ? "arg" : names[0] );
         return option;
      }
      else if ( isOption( names ) ) {
         mOptions.push_back( std::move(newOption) );
         auto& option = mOptions.back();
         trySetNames( option, names );
         return option;
      }

      throw std::invalid_argument( "The argument must be either positional or an option." );
   }

   void trySetNames( Option& option, const std::vector<std::string_view>& names ) const
   {
      for ( auto name : names ) {
         if ( name.empty() || name == "-" || name == "--" )
            continue;

         bool forceShort = false;
         bool forceLong = false;
         if ( name.substr( 0, 2 ) == "--" ) {
            forceLong = true;
            name = name.substr( 2 );
         }
         else if ( name.substr( 0, 1 ) == "-" ) {
            forceShort = true;
            name = name.substr( 1 );
         }

         name.remove_prefix(std::min(name.find_first_not_of(" "), name.size()));
         name.remove_suffix(name.size() - std::min(name.find_last_not_of(" ") + 1, name.size()));

         if ( forceShort && name.size() > 1 )
            throw std::invalid_argument( "Short option name has too many characters." );

         if ( forceLong || name.size() > 1 )
            option.setLongName( name );
         else
            option.setShortName( name );
      }

      if ( option.getName().empty() )
            throw std::invalid_argument( "An option must have a name." );
   }
};

}
