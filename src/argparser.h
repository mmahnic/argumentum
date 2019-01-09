// Copyright (c) 2018, 2019 Marko Mahnič
// License: MIT. See LICENSE in the root of the project.

#pragma once

#include "helpformatter_i.h"

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <algorithm>
#include <cctype>
#include <iostream>

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

class InvalidChoiceError: public std::invalid_argument
{
public:
   InvalidChoiceError( const std::string& value )
      : std::invalid_argument( value )
   {}
};

class ArgumentParser;

class Options
{
public:
   virtual void add_arguments( ArgumentParser& parser ) = 0;
};

class ArgumentParser
{
public:
   class Value
   {
      int mAssignCount = 0;
      int mOptionAssignCount = 0;
      bool mHasErrors = false;
   public:
      void setValue( const std::string& value )
      {
         ++mAssignCount;
         ++mOptionAssignCount;
         doSetValue( value );
      }

      void markBadArgument()
      {
         // Increase the assign count so that flagValue will not be used.
         ++mOptionAssignCount;
         mHasErrors = true;
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
      std::string mHelp;
      std::string mFlagValue = "1";
      std::vector<std::string> mChoices;
      int mMinArgs = 0;
      int mMaxArgs = 0;
      bool mIsRequired = false;
      bool mIsVectorValue = false;

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

         mIsVectorValue = true;
      }

      void setShortName( std::string_view name )
      {
         mShortName = name;
      }

      void setLongName( std::string_view name )
      {
         mLongName = name;
      }

      void setHelp( std::string_view help )
      {
         mHelp = help;
      }

      void setNArgs( int count )
      {
         mMinArgs = std::max( 0, count );
         mMaxArgs = mMinArgs;
      }

      void setMinArgs( int count )
      {
         mMinArgs = std::max( 0, count );
         mMaxArgs = -1;
      }

      void setMaxArgs( int count )
      {
         mMinArgs = 0;
         mMaxArgs = std::max( 0, count );
      }

      void setRequired( bool isRequired=true )
      {
         mIsRequired = isRequired;
      }

      void setFlagValue( std::string_view value )
      {
         mFlagValue = value;
      }

      void setChoices( const std::vector<std::string>& choices )
      {
         mChoices = choices;
      }

      bool isRequired() const
      {
         return mIsRequired;
      }

      const std::string& getName() const
      {
         return mLongName.empty() ? mShortName : mLongName;
      }

      const std::string& getShortName() const
      {
         return mShortName;
      }

      const std::string& getLongName() const
      {
         return mLongName;
      }

      bool hasName( std::string_view name ) const
      {
         return name == mShortName || name == mLongName;
      }

      const std::string& getRawHelp() const
      {
         return mHelp;
      }

      void setValue( const std::string& value )
      {
         if ( !mChoices.empty() && std::none_of( mChoices.begin(), mChoices.end(),
                  [&value]( auto v ) { return v == value; } ) )
         {
            mpValue->markBadArgument();
            throw InvalidChoiceError( value );
         }

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

      bool hasVectorValue() const
      {
         return mIsVectorValue;
      }

      /**
       * @returns true if the value was assigned through any option that shares
       * this option's value.
       */
      bool wasAssigned() const
      {
         return mpValue->getAssignCount() > 0;
      }

      bool wasAssignedThroughThisOption() const
      {
         return mpValue->getOptionAssignCount() > 0;
      }

      const std::string& getFlagValue() const
      {
         return mFlagValue;
      }
   };

   class OptionConfig
   {
      std::vector<Option>& mOptions;
      size_t mIndex = 0;
      bool mCountWasSet = false;

   public:
      OptionConfig( std::vector<Option>& options, size_t index )
         : mOptions( options ), mIndex( index )
      {}

      OptionConfig& setShortName( std::string_view name )
      {
         mOptions[mIndex].setShortName( name );
         return *this;
      }

      OptionConfig& setLongName( std::string_view name )
      {
         mOptions[mIndex].setLongName( name );
         return *this;
      }

      OptionConfig& help( std::string_view help )
      {
         mOptions[mIndex].setHelp( help );
         return *this;
      }

      OptionConfig& nargs( int count )
      {
         ensureCountWasNotSet();
         mOptions[mIndex].setNArgs( count );
         mCountWasSet = true;
         return *this;
      }

      OptionConfig& minargs( int count )
      {
         ensureCountWasNotSet();
         mOptions[mIndex].setMinArgs( count );
         mCountWasSet = true;
         return *this;
      }

      OptionConfig& maxargs( int count )
      {
         ensureCountWasNotSet();
         mOptions[mIndex].setMaxArgs( count );
         mCountWasSet = true;
         return *this;
      }

      OptionConfig& required( bool isRequired=true )
      {
         mOptions[mIndex].setRequired( isRequired );
         return *this;
      }

      OptionConfig& flagValue( std::string_view value )
      {
         mOptions[mIndex].setFlagValue( value );
         return *this;
      }

      OptionConfig& choices( const std::vector<std::string>& choices )
      {
         mOptions[mIndex].setChoices( choices );
         return *this;
      }

   private:
      void ensureCountWasNotSet() const
      {
         if ( mCountWasSet )
            throw std::invalid_argument( "Only one of nargs, minargs and maxargs can be used." );
      }
   };

   class ParserConfig
   {
   public:
      struct Data
      {
         std::string program;
         std::string usage;
         std::string description;
         std::string epilog;
      };

   private:
      Data mData;

   public:
      const Data& data() const
      {
         return mData;
      }

      ParserConfig& prog( std::string_view program )
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
   };

   // Errors known by the parser
   enum EError {
      UNKNOWN_OPTION,
      MISSING_OPTION,
      MISSING_ARGUMENT,
      CONVERSION_ERROR,
      INVALID_CHOICE,
      // Flags do not accept parameters
      FLAG_PARAMETER

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
               startOption( arg );
            else if ( arg_view.substr( 0, 1 ) == "-" ) {
               if ( arg.size() == 2 )
                  startOption( arg );
               else {
                  auto opt = std::string{"--"};
                  for ( int i = 1; i < arg_view.size(); ++i ) {
                     opt[1] = arg_view[i];
                     startOption( opt );
                  }
               }
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

         std::string_view arg;
         auto eqpos = name.find( "=" );
         if ( eqpos != std::string::npos ) {
            arg = name.substr( eqpos + 1 );
            name = name.substr( 0, eqpos );
         }

         auto pOption = findOption( name );
         if ( pOption ) {
            auto& option = *pOption;
            option.onOptionStarted();
            if ( option.willAcceptArgument() )
               mpActiveOption = pOption;
            else
               setValue( option, option.getFlagValue() );

            if ( !arg.empty() ) {
               if ( option.willAcceptArgument() )
                  setValue( option, std::string{ arg } ); // TODO: params should be string_view-s
               else
                  addError( name, FLAG_PARAMETER );
            }
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
            else if ( option.willAcceptArgument() && !option.wasAssignedThroughThisOption() )
               option.setValue( option.getFlagValue() );
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
         catch( const InvalidChoiceError& ) {
            addError( option.getName(), INVALID_CHOICE );
         }
         catch( const std::invalid_argument& ) {
            addError( option.getName(), CONVERSION_ERROR );
         }
         catch( const std::out_of_range& ) {
            addError( option.getName(), CONVERSION_ERROR );
         }
      }
   };

private:
   ParserConfig mConfig;
   std::vector<Option> mOptions;
   std::vector<Option> mPositional;
   std::shared_ptr<Options> mpTargets;

public:
   /**
    * Create and return an argument parser.  If @p pOptions is not nullptr
    * the arguments will be registered by calling its add_arguments method.
    *
    * The argument parser takes references to the variables that will hold the
    * parsed values.  The variables must outlive the argument parser.
    *
    * The parser stores the pointer to @p pOptions. If it is not nullptr and
    * all the registered variables are in the memory space used by *pOptions
    * the variables will outlive the parser.
    */
   static ArgumentParser create_unsafe( std::shared_ptr<Options> pOptions=nullptr )
   {
      auto parser = ArgumentParser( pOptions );
      if ( pOptions )
         pOptions->add_arguments( parser );
      return parser;
   }

   /**
    * Create and return an argument parser.  The arguments will be registered
    * by calling the add_arguments method of @p pOptions which must not be
    * nullptr.
    *
    * The argument parser takes references to the variables that will hold the
    * parsed values.  The variables must outlive the argument parser.
    *
    * The parser stores the pointer to @p pOptions. If all the registered
    * variables are in the memory space used by *pOptions the variables will
    * outlive the parser.
    *
    * NOTE: This method is essentially the same as create_unsafe().  The safety
    * of the memory usage is not checked (it can not be because of possible
    * custom Value arguments).  It is still the programmer's responsibility to
    * check that valid memory space is being used.  The naming is just a way of
    * documenting that the programmer made an effort to make the use of the
    * parser safe.
    */
   static ArgumentParser create( std::shared_ptr<Options> pOptions )
   {
      if ( !pOptions )
         throw std::invalid_argument(
               "The create_safe factory method requires a pointer to an Options structure." );

      return create_unsafe( pOptions );
   }

   ParserConfig& config()
   {
      return mConfig;
   }

   const ParserConfig::Data& getConfig() const
   {
      return mConfig.data();
   }

   template<typename TValue, typename = std::enable_if_t<std::is_base_of<Value, TValue>::value> >
   OptionConfig add_argument( TValue value, const std::string& name="", const std::string& altName="" )
   {
      auto option = Option( value );
      return tryAddArgument( option, { name, altName } );
   }

   template<typename TValue, typename = std::enable_if_t<!std::is_base_of<Value, TValue>::value> >
   OptionConfig add_argument( TValue &value, const std::string& name="", const std::string& altName="" )
   {
      auto option = Option( value );
      return tryAddArgument( option, { name, altName } );
   }

   ParseResult parse_args( const std::vector<std::string>& args )
   {
      Parser parser( *this );
      auto result = parser.parse( args );
      reportMissingOptions( result );
      return result;
   }

   ArgumentHelpResult describe_argument( std::string_view name ) const
   {
      const auto& args = (name.substr( 0, 1 ) == "-") ? mOptions : mPositional;
      for ( auto& opt : args )
         if ( opt.hasName( name ) )
            return describeOption( opt );

      throw std::invalid_argument( "Unknown option" );
   }

   std::vector<ArgumentHelpResult> describe_arguments() const
   {
      std::vector<ArgumentHelpResult> descriptions;

      for ( auto& opt : mOptions )
         descriptions.push_back( describeOption( opt ) );

      for ( auto& opt : mPositional )
         descriptions.push_back( describeOption( opt ) );

      return descriptions;
   }

private:
   ArgumentParser() = default;

   ArgumentParser( std::shared_ptr<Options> pOptions )
      : mpTargets( pOptions )
   {}

   void reportMissingOptions( ParseResult& result )
   {
      for ( auto& option : mOptions )
         if ( option.isRequired() && !option.wasAssigned() )
            result.errors.emplace_back( option.getName(), MISSING_OPTION );

      for ( auto& option : mPositional )
         if ( option.needsMoreArguments() )
            result.errors.emplace_back( option.getName(), MISSING_ARGUMENT );
   }

   OptionConfig tryAddArgument( Option& newOption, std::vector<std::string_view> names )
   {
      // Remove empty names
      auto is_empty = [&]( auto name ) { return name.empty(); };
      names.erase( std::remove_if( names.begin(), names.end(), is_empty ), names.end() );

      if ( names.empty() )
         throw std::invalid_argument( "An argument must have a name." );

      for ( auto& name : names )
         for ( auto ch : name )
            if ( std::isspace( ch ) )
               throw std::invalid_argument( "Argument names must not contain spaces." );

      auto has_dash = []( auto name ) { return name[0] == '-'; };

      auto isOption = [&]( auto names ) -> bool {
         return std::all_of( names.begin(), names.end(), has_dash );
      };

      auto isPositional = [&]( auto names ) -> bool {
         return std::none_of( names.begin(), names.end(), has_dash );
      };

      if ( isPositional( names ) ) {
         mPositional.push_back( std::move(newOption) );
         auto& option = mPositional.back();
         option.setLongName( names.empty() ? "arg" : names[0] );

         if ( option.hasVectorValue() )
            option.setMinArgs( 0 );
         else
            option.setNArgs( 1 );

         return { mPositional, mPositional.size() - 1 };
      }
      else if ( isOption( names ) ) {
         mOptions.push_back( std::move(newOption) );
         auto& option = mOptions.back();
         trySetNames( option, names );

         return { mOptions, mOptions.size() - 1 };
      }

      throw std::invalid_argument( "The argument must be either positional or an option." );
   }

   void trySetNames( Option& option, const std::vector<std::string_view>& names ) const
   {
      for ( auto name : names ) {
         if ( name.empty() || name == "-" || name == "--" || name[0] != '-' )
            continue;

         if ( name.substr( 0, 2 ) == "--" )
            option.setLongName( name );
         else if ( name.substr( 0, 1 ) == "-" ) {
            if ( name.size() > 2 )
               throw std::invalid_argument( "Short option name has too many characters." );
            option.setShortName( name );
         }
      }

      if ( option.getName().empty() )
            throw std::invalid_argument( "An option must have a name." );
   }

   ArgumentHelpResult describeOption( const Option& option ) const
   {
      ArgumentHelpResult help;
      help.short_name = option.getShortName();
      help.long_name = option.getLongName();
      help.help = option.getRawHelp();
      return help;
   }

};

}
