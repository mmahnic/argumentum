// Copyright (c) 2018, 2019 Marko Mahnič
// License: MPL2. See LICENSE in the root of the project.

#pragma once

#include "commands.h"
#include "environment.h"
#include "exceptions.h"
#include "groups.h"
#include "helpformatter_i.h"
#include "options.h"
#include "parseresult.h"
#include "values.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace argparse {

class argument_parser;

class argument_parser
{
public:
   class ParserConfig
   {
   public:
      struct Data
      {
         std::string program;
         std::string usage;
         std::string description;
         std::string epilog;
         std::ostream* pOutStream = nullptr;
      };

   private:
      Data mData;

   public:
      const Data& data() const
      {
         return mData;
      }

      ParserConfig& program( std::string_view program )
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

      // NOTE: The @p stream must outlive the parser.
      ParserConfig& cout( std::ostream& stream )
      {
         mData.pOutStream = &stream;
         return *this;
      }
   };

private:
   class Parser
   {
      argument_parser& mArgParser;
      ParseResultBuilder& mResult;

      bool mIgnoreOptions = false;
      int mPosition = 0;
      // The active option will receive additional argument(s)
      Option* mpActiveOption = nullptr;

   public:
      Parser( argument_parser& argParser, ParseResultBuilder& result )
         : mArgParser( argParser )
         , mResult( result )
      {}

      void parse( std::vector<std::string>::const_iterator ibegin,
            std::vector<std::string>::const_iterator iend )
      {
         mResult.clear();
         for ( auto iarg = ibegin; iarg != iend; ++iarg ) {
            if ( *iarg == "--" ) {
               mIgnoreOptions = true;
               continue;
            }

            if ( mIgnoreOptions ) {
               addFreeArgument( *iarg );
               continue;
            }

            auto arg_view = std::string_view( *iarg );
            if ( arg_view.substr( 0, 2 ) == "--" )
               startOption( *iarg );
            else if ( arg_view.substr( 0, 1 ) == "-" ) {
               if ( iarg->size() == 2 )
                  startOption( *iarg );
               else {
                  auto opt = std::string{ "--" };
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
                     setValue( option, *iarg );
                     if ( !option.willAcceptArgument() )
                        closeOption();
                  }
               }
               else {
                  auto pCommand = mArgParser.findCommand( *iarg );
                  if ( pCommand ) {
                     mArgParser.parseCommandArguments( *pCommand, iarg, iend, mResult );
                     break;
                  }
                  else
                     addFreeArgument( *iarg );
               }
            }

            if ( mResult.wasExitRequested() )
               break;
         }

         if ( haveActiveOption() )
            closeOption();
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
                  setValue( option, std::string{ arg } );
               else
                  addError( pOption->getHelpName(), FLAG_PARAMETER );
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
               addError( option.getHelpName(), MISSING_ARGUMENT );
            else if ( option.willAcceptArgument() && !option.wasAssignedThroughThisOption() )
               setValue( option, option.getFlagValue() );
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

         mResult.addIgnored( arg );
      }

      void addError( std::string_view optionName, int errorCode )
      {
         mResult.addError( optionName, errorCode );
      }

      Option* findOption( std::string_view optionName ) const
      {
         return mArgParser.findOption( optionName );
      }

      void setValue( Option& option, const std::string& value )
      {
         try {
            auto env = Environment{ option, mResult };
            option.setValue( value, env );
         }
         catch ( const InvalidChoiceError& ) {
            addError( option.getHelpName(), INVALID_CHOICE );
         }
         catch ( const std::invalid_argument& ) {
            addError( option.getHelpName(), CONVERSION_ERROR );
         }
         catch ( const std::out_of_range& ) {
            addError( option.getHelpName(), CONVERSION_ERROR );
         }
      }
   };

private:
   ParserConfig mConfig;
   std::vector<Command> mCommands;
   std::vector<Option> mOptions;
   std::vector<Option> mPositional;
   std::set<std::string> mHelpOptionNames;
   std::vector<std::shared_ptr<Options>> mTargets;
   std::map<std::string, std::shared_ptr<OptionGroup>> mGroups;
   std::shared_ptr<OptionGroup> mpActiveGroup;

public:
   /**
    * Get a reference to the parser configuration through which the parser can
    * be configured.
    */
   ParserConfig& config()
   {
      return mConfig;
   }

   /**
    * Get a reference to the parser configuration for inspection.
    */
   const ParserConfig::Data& getConfig() const
   {
      return mConfig.data();
   }

   CommandConfig add_command( const std::string& name, Command::options_factory_t factory )
   {
      auto command = Command( name, factory );
      return tryAddCommand( command );
   }

   template<typename TValue, typename = std::enable_if_t<std::is_base_of<Value, TValue>::value>>
   OptionConfigA<TValue> add_argument(
         TValue value, const std::string& name = "", const std::string& altName = "" )
   {
      auto option = Option( value );
      return OptionConfigA<TValue>( tryAddArgument( option, { name, altName } ) );
   }

   /**
    * Add an argument with names @p name and @p altName and store the reference
    * to @p value that will receive the parsed parameter(s).
    */
   template<typename TValue, typename = std::enable_if_t<!std::is_base_of<Value, TValue>::value>>
   OptionConfigA<TValue> add_argument(
         TValue& value, const std::string& name = "", const std::string& altName = "" )
   {
      auto option = Option( value );
      return OptionConfigA<TValue>( tryAddArgument( option, { name, altName } ) );
   }

   /**
    * Add the @p pOptions structure and call its add_arguments method to add
    * the arguments to the parser.  The pointer to @p pOptions is stored in the
    * parser so that the structure outlives the parser.
    */
   void add_arguments( std::shared_ptr<Options> pOptions )
   {
      if ( pOptions ) {
         mTargets.push_back( pOptions );
         pOptions->add_arguments( *this );
      }
   }

   /**
    * Add default help options --help and -h that will display the help and
    * terminate the parser.
    *
    * The method will throw an invalid_argument exception if none of the option
    * names --help and -h can be used.
    *
    * This method will be called from parse_args if neither it nor the method
    * add_help_option were called before parse_args.
    */
   VoidOptionConfig add_default_help_option()
   {
      const auto shortName = "-h";
      const auto longName = "--help";
      auto pShort = findOption( shortName );
      auto pLong = findOption( longName );

      if ( !pShort && !pLong )
         return add_help_option( shortName, longName );
      if ( !pShort )
         return add_help_option( shortName );
      if ( !pLong )
         return add_help_option( longName );

      throw std::invalid_argument( "The default help options are hidden by other options." );
   }

   /**
    * Add a special option that will display the help and terminate the parser.
    *
    * If neither this method nor add_default_help_option is called, the default
    * help options --help and -h will be used as long as they are not used for
    * other purposes.
    */
   VoidOptionConfig add_help_option( const std::string& name, const std::string& altName = "" )
   {
      if ( !name.empty() && name[0] != '-' || !altName.empty() && altName[0] != '-' )
         throw std::invalid_argument( "A help argument must be an option." );

      auto value = VoidValue{};
      auto option = Option( value );
      auto optionConfig = VoidOptionConfig( tryAddArgument( option, { name, altName } ) )
                                .help( "Display this help message and exit." );

      if ( !name.empty() )
         mHelpOptionNames.insert( name );
      if ( !altName.empty() )
         mHelpOptionNames.insert( altName );

      return optionConfig;
   }

   GroupConfig add_group( const std::string& name )
   {
      auto pGroup = findGroup( name );
      if ( pGroup ) {
         if ( pGroup->isExclusive() )
            throw MixingGroupTypes( name );
         mpActiveGroup = pGroup;
      }
      else
         mpActiveGroup = addGroup( name, false );

      return GroupConfig( mpActiveGroup );
   }

   GroupConfig add_exclusive_group( const std::string& name )
   {
      auto pGroup = findGroup( name );
      if ( pGroup ) {
         if ( !pGroup->isExclusive() )
            throw MixingGroupTypes( name );
         mpActiveGroup = pGroup;
      }
      else
         mpActiveGroup = addGroup( name, true );

      return GroupConfig( mpActiveGroup );
   }

   void end_group()
   {
      mpActiveGroup = nullptr;
   }

   ParseResult parse_args( int argc, char** argv, int skip_args = 1 )
   {
      if ( !argv ) {
         auto res = ParseResultBuilder{};
         res.addError( "argv", INVALID_ARGV );
         return res.getResult();
      }

      std::vector<std::string> args;
      for ( int i = std::max( 0, skip_args ); i < argc; ++i )
         args.emplace_back( argv[i] );

      return parse_args( std::begin( args ), std::end( args ) );
   }

   ParseResult parse_args( const std::vector<std::string>& args, int skip_args = 0 )
   {
      auto ibegin = std::begin( args );
      if ( skip_args > 0 )
         ibegin += std::min<size_t>( skip_args, args.size() );

      return parse_args( ibegin, std::end( args ) );
   }

   ParseResult parse_args( std::vector<std::string>::const_iterator ibegin,
         std::vector<std::string>::const_iterator iend )
   {
      verifyDefinedOptions();
      ParseResultBuilder result;
      parse_args( ibegin, iend, result );

      if ( result.hasArgumentProblems() ) {
         result.signalErrorsShown();
         auto res = std::move( result.getResult() );
         describe_errors( res );
         return std::move( res );
      }

      return std::move( result.getResult() );
   }

   ArgumentHelpResult describe_argument( std::string_view name ) const
   {
      bool isPositional = name.substr( 0, 1 ) != "-";
      const auto& args = isPositional ? mPositional : mOptions;
      for ( auto& opt : args )
         if ( opt.hasName( name ) )
            return describeOption( opt );

      throw std::invalid_argument( "Unknown option." );
   }

   std::vector<ArgumentHelpResult> describe_arguments() const
   {
      std::vector<ArgumentHelpResult> descriptions;

      for ( auto& opt : mOptions )
         descriptions.push_back( describeOption( opt ) );

      for ( auto& opt : mPositional )
         descriptions.push_back( describeOption( opt ) );

      for ( auto& cmd : mCommands )
         descriptions.push_back( describeCommand( cmd ) );

      return descriptions;
   }

private:
   void parse_args( std::vector<std::string>::const_iterator ibegin,
         std::vector<std::string>::const_iterator iend, ParseResultBuilder& result )
   {
      if ( ibegin == iend && hasRequiredArguments() ) {
         generate_help();
         result.signalHelpShown();
         result.requestExit();
         return;
      }

      for ( auto& option : mOptions )
         option.resetValue();

      for ( auto& option : mPositional )
         option.resetValue();

      for ( auto iarg = ibegin; iarg != iend; ++iarg ) {
         if ( mHelpOptionNames.count( *iarg ) > 0 ) {
            generate_help();
            result.signalHelpShown();
            result.requestExit();
            return;
         }
      }

      Parser parser( *this, result );
      parser.parse( ibegin, iend );
      if ( result.wasExitRequested() ) {
         result.addError( {}, EXIT_REQUESTED );
         return;
      }

      assignDefaultValues();

      reportMissingOptions( result );
      reportExclusiveViolations( result );
      reportMissingGroups( result );
   }

   Option* findOption( std::string_view optionName )
   {
      for ( auto& option : mOptions )
         if ( option.hasName( optionName ) )
            return &option;

      return nullptr;
   }

   void assignDefaultValues()
   {
      for ( auto& option : mOptions )
         if ( !option.wasAssigned() && option.hasDefault() )
            option.assignDefault();

      for ( auto& option : mPositional )
         if ( !option.wasAssigned() && option.hasDefault() )
            option.assignDefault();
   }

   void verifyDefinedOptions()
   {
      // Check if any options are defined and add the default if not.
      if ( mHelpOptionNames.empty() ) {
         end_group();
         try {
            add_default_help_option();
         }
         catch ( const std::invalid_argument& ) {
            // TODO: write a warning through a logging system proxy:
            // argparser::logger().warn( "...", __FILE__, __LINE__ );
         }
      }

      // A required option can not be in an exclusive group.
      for ( auto& opt : mOptions ) {
         if ( opt.isRequired() ) {
            auto pGroup = opt.getGroup();
            if ( pGroup && pGroup->isExclusive() )
               throw RequiredExclusiveOption( opt.getName(), pGroup->getName() );
         }
      }
   }

   void reportMissingOptions( ParseResultBuilder& result )
   {
      for ( auto& option : mOptions )
         if ( option.isRequired() && !option.wasAssigned() )
            result.addError( option.getHelpName(), MISSING_OPTION );

      for ( auto& option : mPositional )
         if ( option.needsMoreArguments() )
            result.addError( option.getHelpName(), MISSING_ARGUMENT );
   }

   bool hasRequiredArguments() const
   {
      for ( auto& option : mOptions )
         if ( option.isRequired() )
            return true;

      for ( auto& option : mPositional )
         if ( option.isRequired() )
            return true;

      return false;
   }

   void reportExclusiveViolations( ParseResultBuilder& result )
   {
      std::map<std::string, std::vector<std::string>> counts;
      for ( auto& option : mOptions ) {
         auto pGroup = option.getGroup();
         if ( pGroup && pGroup->isExclusive() && option.wasAssigned() )
            counts[pGroup->getName()].push_back( option.getHelpName() );
      }

      for ( auto& c : counts )
         if ( c.second.size() > 1 )
            result.addError( c.second.front(), EXCLUSIVE_OPTION );
   }

   void reportMissingGroups( ParseResultBuilder& result )
   {
      std::map<std::string, int> counts;
      for ( auto& option : mOptions ) {
         auto pGroup = option.getGroup();
         if ( pGroup && pGroup->isRequired() )
            counts[pGroup->getName()] += option.wasAssigned() ? 1 : 0;
      }

      for ( auto& c : counts )
         if ( c.second < 1 )
            result.addError( c.first, MISSING_OPTION_GROUP );
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
         mPositional.push_back( std::move( newOption ) );
         auto& option = mPositional.back();
         option.setLongName( names.empty() ? "arg" : names[0] );
         option.setRequired( true );

         if ( option.hasVectorValue() )
            option.setMinArgs( 0 );
         else
            option.setNArgs( 1 );

         // Positional parameters are required so they can't be in an exclusive
         // group.  We simply ignore them.
         if ( mpActiveGroup && !mpActiveGroup->isExclusive() )
            option.setGroup( mpActiveGroup );

         return { mPositional, mPositional.size() - 1 };
      }
      else if ( isOption( names ) ) {
         trySetNames( newOption, names );
         ensureIsNewOption( newOption.getLongName() );
         ensureIsNewOption( newOption.getShortName() );

         mOptions.push_back( std::move( newOption ) );
         auto& option = mOptions.back();

         if ( mpActiveGroup )
            option.setGroup( mpActiveGroup );

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

   void ensureIsNewOption( const std::string& name )
   {
      if ( name.empty() )
         return;

      auto pOption = findOption( name );
      if ( pOption ) {
         auto groupName = pOption->getGroup() ? pOption->getGroup()->getName() : "";
         throw DuplicateOption( groupName, name );
      }
   }

   CommandConfig tryAddCommand( Command& command )
   {
      if ( command.getName().empty() )
         throw std::invalid_argument( "A command must have a name." );
      if ( !command.hasFactory() )
         throw std::invalid_argument( "A command must have an options factory." );
      if ( command.getName()[0] == '-' )
         throw std::invalid_argument( "Command name must not start with a dash." );

      ensureIsNewCommand( command.getName() );
      mCommands.push_back( std::move( command ) );

      return { mCommands, mCommands.size() - 1 };
   }

   void ensureIsNewCommand( const std::string& name )
   {
      auto pCommand = findCommand( name );
      if ( pCommand )
         throw DuplicateCommand( name );
   }

   Command* findCommand( std::string_view commandName )
   {
      for ( auto& command : mCommands )
         if ( command.hasName( commandName ) )
            return &command;

      return nullptr;
   }

   void parseCommandArguments( Command& command, std::vector<std::string>::const_iterator ibegin,
         std::vector<std::string>::const_iterator iend, ParseResultBuilder& result )
   {
      auto parser = argument_parser{};
      parser.add_arguments( command.createOptions() );
      parser.parse_args( ibegin, iend, result );
   }

   std::shared_ptr<OptionGroup> addGroup( std::string name, bool isExclusive )
   {
      if ( name.empty() )
         throw std::invalid_argument( "A group must have a name." );

      std::transform( name.begin(), name.end(), name.begin(), tolower );
      assert( mGroups.count( name ) == 0 );

      auto pGroup = std::make_shared<OptionGroup>( name, isExclusive );
      mGroups[name] = pGroup;
      return pGroup;
   }

   std::shared_ptr<OptionGroup> findGroup( std::string name ) const
   {
      std::transform( name.begin(), name.end(), name.begin(), tolower );
      auto igrp = mGroups.find( name );
      if ( igrp == mGroups.end() )
         return {};
      return igrp->second;
   }

   ArgumentHelpResult describeOption( const Option& option ) const
   {
      ArgumentHelpResult help;
      help.help_name = option.getHelpName();
      help.short_name = option.getShortName();
      help.long_name = option.getLongName();
      help.metavar = option.getMetavar();
      help.help = option.getRawHelp();
      help.isRequired = option.isRequired();

      if ( option.acceptsAnyArguments() ) {
         const auto& metavar = help.metavar;
         auto [mmin, mmax] = option.getArgumentCounts();
         std::string res;
         if ( mmin > 0 ) {
            res = metavar;
            for ( int i = 1; i < mmin; ++i )
               res = res + " " + metavar;
         }
         if ( mmax < mmin ) {
            auto opt = ( res.empty() ? "[" : " [" ) + metavar + " ...]";
            res += opt;
         }
         else if ( mmax - mmin == 1 )
            res += "[" + metavar + "]";
         else if ( mmax > mmin ) {
            auto opt = ( res.empty() ? "[" : " [" ) + metavar + " {0.."
                  + std::to_string( mmax - mmin ) + "}]";
            res += opt;
         }

         help.arguments = std::move( res );
      }

      auto pGroup = option.getGroup();
      if ( pGroup ) {
         help.group.name = pGroup->getName();
         help.group.title = pGroup->getTitle();
         help.group.description = pGroup->getDescription();
         help.group.isExclusive = pGroup->isExclusive();
         help.group.isRequired = pGroup->isRequired();
      }

      return help;
   }

   ArgumentHelpResult describeCommand( const Command& command ) const
   {
      ArgumentHelpResult help;
      help.isCommand = true;
      help.help_name = command.getName();
      help.long_name = command.getName();
      help.help = command.getHelp();

      return help;
   }

   void generate_help()
   {
      // TODO: The formatter should be configurable
      auto formatter = HelpFormatter();
      auto pStream = getConfig().pOutStream;
      if ( !pStream )
         pStream = &std::cout;

      formatter.format( *this, *pStream );
   }

   void describe_errors( ParseResult& result )
   {
      auto pStream = getConfig().pOutStream;
      if ( !pStream )
         pStream = &std::cout;

      for ( const auto& e : result.errors ) {
         switch ( e.errorCode ) {
            case UNKNOWN_OPTION:
               *pStream << "Error: Unknown option: '" << e.option << "'\n";
               break;
            case EXCLUSIVE_OPTION:
               *pStream << "Error: Only one option from an exclusive group can be set. '"
                        << e.option << "'\n";
               break;
            case MISSING_OPTION:
               *pStream << "Error: A required option is missing: '" << e.option << "'\n";
               break;
            case MISSING_OPTION_GROUP:
               *pStream << "Error: A required option from a group is missing: '" << e.option
                        << "'\n";
               break;
            case MISSING_ARGUMENT:
               *pStream << "Error: An argument is missing: '" << e.option << "'\n";
               break;
            case CONVERSION_ERROR:
               *pStream << "Error: The argument could not be converted: '" << e.option << "'\n";
               break;
            case INVALID_CHOICE:
               *pStream << "Error: The value is not in the list of valid values: '" << e.option
                        << "'\n";
               break;
            case FLAG_PARAMETER:
               *pStream << "Error: Flag options do not accep parameters: '" << e.option << "'\n";
               break;
            case EXIT_REQUESTED:
               break;
            case ACTION_ERROR:
               *pStream << "Error: " << e.option << "\n";
               break;
            case INVALID_ARGV:
               *pStream << "Error: Parser input is invalid.\n";
               break;
         }
      }

      if ( !result.ignoredArguments.empty() ) {
         auto it = result.ignoredArguments.begin();
         *pStream << "Error: Ignored arguments: " << *it;
         for ( ++it; it != result.ignoredArguments.end(); ++it )
            *pStream << ", " << *it;
         *pStream << "\n";
      }
   }
};

}   // namespace argparse

#include "helpformatter.h"
