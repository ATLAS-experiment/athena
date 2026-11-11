/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>

#include "CxxUtils/checker_macros.h"
#include "PoolSvc/FileCatalogUtils.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "StorageSvc/SimpleUtilityBase.h"


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
{
   initGaudi();
}

   
bool
InsertFileToCatalogApplication::parseArguments()
{
  unsigned int excludedArgument = 0;
  for ( unsigned int iArg = 0; iArg < m_args.size(); ++iArg ) {
     if ( iArg > 0 && iArg == excludedArgument )
	continue;
     const std::string& arg = m_args[iArg];
     if ( arg == "-u" ) {
	unsigned int nextArgumentIndex = iArg + 1;
	if( nextArgumentIndex < m_args.size() ) {
	   excludedArgument = nextArgumentIndex;
	   m_catalogFN = m_args[nextArgumentIndex];
	}
     }
     else if( arg == "-t" ) {
	unsigned int nextArgumentIndex = iArg + 1;
	if ( nextArgumentIndex < m_args.size() ) {
	   excludedArgument = nextArgumentIndex;
	   m_technologyName = m_args[nextArgumentIndex];
	}
     }
     else {
	m_fileNames.push_back( arg );
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
   SmartIF<Gaudi::IFileCatalogMgr> catalogMgr = Gaudi::svcLocator()->service<Gaudi::IFileCatalogMgr>( "Gaudi::MultiFileCatalog" );
   SmartIF<Gaudi::IFileCatalog> catalog( catalogMgr );
   FileCatalogUtils::addCatalog( *catalogMgr, m_catalogFN, true );
   catalog->init();

   for( const auto& fp : m_fidAndPfn ) {
      std::string fid = fp.first; // can't be const
      FileCatalogUtils::registerPFN(*catalog, fp.second, m_technologyName, fid);
   }    
   catalog->commit();
}


void
InsertFileToCatalogApplication::printSyntax()
{
  std::cout << "Syntax : " << m_executableName << " [-u fileCatalog] [-t technologyType] files" << std::endl;
}



int main ATLAS_NOT_THREAD_SAFE ( int argc, char* argv[] )
{
   InsertFileToCatalogApplication	app( argc, argv );
   return app.run();
}
