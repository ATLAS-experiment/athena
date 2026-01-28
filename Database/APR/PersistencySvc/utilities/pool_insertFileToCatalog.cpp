/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>

#include "CxxUtils/checker_macros.h"
#include "PersistencySvc/SimpleUtilityBase.h"
#include "FileCatalog/IFileCatalog.h"
#include "POOLCore/SystemTools.h"
//#include "POOLCore/DbPrint.h"

using namespace pool;

class ATLAS_NOT_THREAD_SAFE InsertFileToCatalogApplication : public SimpleUtilityBase {
public:
  InsertFileToCatalogApplication( int argc, char* argv[] );

  bool parseArguments(); 
  void execute();
  void printSyntax();

private:
  std::string 		m_catalogFN;
};



InsertFileToCatalogApplication::InsertFileToCatalogApplication( int argc, char* argv[] )
      : SimpleUtilityBase( argc, argv )
{ }

   
bool
InsertFileToCatalogApplication::parseArguments()
{
  unsigned int excludedArgument = 0;
  for ( unsigned int iArg = 0; iArg < args.size(); ++iArg ) {
     if ( iArg > 0 && iArg == excludedArgument )
	continue;
     const std::string& arg = args[iArg];
     if ( arg == "-u" ) {
	unsigned int nextArgumentIndex = iArg + 1;
	if( nextArgumentIndex < args.size() ) {
	   excludedArgument = nextArgumentIndex;
	   m_catalogFN = args[nextArgumentIndex];
	}
     }
     else if( arg == "-t" ) {
	unsigned int nextArgumentIndex = iArg + 1;
	if ( nextArgumentIndex < args.size() ) {
	   excludedArgument = nextArgumentIndex;
	   technologyName = args[nextArgumentIndex];
	}
     }
     else {
	fileNames.push_back( arg );
     }
  }
  return SimpleUtilityBase::parseArguments();
}

void
InsertFileToCatalogApplication::execute()
{
   startSession();
   readFileGUIDs();
  
  // Open the file catalog and insert the pfn/fid/technology
   pool::IFileCatalog   catalog;
   catalog.setWriteCatalog( m_catalogFN );

   catalog.connect();
   catalog.start();
   
   for( const auto& fp : fidAndPfn ) {
      std::string fid = fp.first; // can't be const
      catalog.registerPFN(fp.second, technologyName, fid);
   }    
   catalog.commit();
}


void
InsertFileToCatalogApplication::printSyntax()
{
  std::cout << "Syntax : " << executableName << " [-u fileCatalog] [-t technologyType] files" << std::endl;
}



int main ATLAS_NOT_THREAD_SAFE ( int argc, char* argv[] )
{
   SystemTools::initGaudi();
   InsertFileToCatalogApplication	app( argc, argv );
   return app.run();
}
