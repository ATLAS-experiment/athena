/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include <string>
#include <cstdlib>

#include "DataQualityInterfaces/HanConfig.h"
#include "DataQualityInterfaces/ConditionsSingleton.h"

#include "CxxUtils/checker_macros.h"
#include "CxxUtils/ubsan_suppress.h"
#include "TInterpreter.h"

namespace {

int usage( const std::string& command_name, int exit_code );

struct CmdLineArgs {
  int parse( int argc, char *argv[] );

  std::string m_connectionString =  "sqlite://;schema=/afs/cern.ch/user/a/atlasdqm/dqmdisk1/cherrypy-devel/RefDB.db;dbname=REFDB";
  long m_runNumber = 2147483646;
  bool m_bulk = false; 
  
  std::string command;
  std::string mconfig;
  std::string conds;
};

} // unnamed namespace


int main ATLAS_NOT_THREAD_SAFE ( int argc, char *argv[] )
{
  CxxUtils::ubsan_suppress ([]() { TInterpreter::Instance(); });

  CmdLineArgs arg;
  int rc = arg.parse( argc, argv );
  if (rc!=0) return rc;

  std::string infileName( arg.mconfig );
  dqi::ConditionsSingleton::getInstance().setCondition(arg.conds);
  //std::cout<<__PRETTY_FUNCTION__<<"Input Conditions="<<dqi::ConditionsSingleton::getInstance().getCondition()
  //<<std::endl;
  std::string outfileName;
  
  std::string::size_type s = infileName.size();
  std::string::size_type p = infileName.rfind( '.', s );
  outfileName = infileName.substr( 0, p );
  outfileName += ".hcfg";
  
  dqi::HanConfig config;
  config.AssembleAndSave( infileName, outfileName, arg.m_connectionString, arg.m_runNumber, arg.m_bulk);
  return 0;
}


// ************************************************************
// Private Functions
// ************************************************************

namespace {

int CmdLineArgs::parse( int argc, char *argv[] )
{
  command = argv[0];
  if( argc > 10 ) return usage( command, 1 );
  if( argc < 2 ) return usage( command, 0 );
  
  mconfig = argv[1];
  
  if(argc >= 3) {
    int ic = 2;
    while(ic < argc) {
      int ir = argc - ic;
      std::string par0 = argv[ic];
      if(par0 == "-c") {
      	if(ir > 1) {
      	  m_connectionString = argv[ic + 1];
          std::cout << "Setting DB connection string to " << m_connectionString << std::endl;
	        ++ic;
	      }
      } else if(par0 == "-r") {
	      if(ir > 1) {
	        try {
	          m_runNumber = std::stol(argv[ic + 1]);
            std::cout << "Setting run number to " << m_runNumber << std::endl;
	          ++ic;
	        } catch(std::exception& e) {
	          std::cout << "Error with input arguments, specified run " << argv[ic+1] << " not a long" << '\n';
	        }
	      }
      } else if(par0 == "-b") {
	      if(ir > 1) {
	        m_bulk = (strcmp(argv[ic+1], "true") == 0);
          std::cout << "Setting bulk processing mode to " << (m_bulk ? "true" : "false") << std::endl;
	        ++ic;
	      }
      } else {
	      conds = argv[ic];
      }
      ++ic;
    }
  }
  return 0;
}


int usage( const std::string& command_name, int exit_code )
{
  std::string message;
  message += "\n";
  message += "This program takes as an argument the name of a file containing\n";
  message += "a MiniConfig configuration and produces a similarly named Root file\n";
  message += "containing a configuration for the \'han\' application\n";

  std::string::size_type s = command_name.size();
  std::string::size_type p = command_name.rfind( '/', s );
  std::string short_name = command_name.substr( p+1, s );

  std::cout << "\n";
  std::cout << "Usage: " << short_name << " <filename>\n";
  std::cout << message << "\n";
  return exit_code;
}


} // unnamed namespace
