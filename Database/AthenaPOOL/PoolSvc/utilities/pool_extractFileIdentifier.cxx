/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>

#include "CxxUtils/checker_macros.h"
#include "StorageSvc/SimpleUtilityBase.h"


class ATLAS_NOT_THREAD_SAFE ExtractFileIdentifierApplication  : public SimpleUtilityBase {
public:
  ExtractFileIdentifierApplication( int argc, char* argv[] );

  bool parseArguments();
  void execute();
  void printSyntax();
};


ExtractFileIdentifierApplication::ExtractFileIdentifierApplication( int argc, char* argv[] )
  : SimpleUtilityBase( argc, argv )
{ }


bool
ExtractFileIdentifierApplication::parseArguments()
{
  unsigned int excludedArgument = 0;
  for ( unsigned int iArg = 0; iArg < m_args.size(); ++iArg ) {
    if ( iArg > 0 && iArg == excludedArgument )
      continue;
    const std::string& arg = m_args[iArg];
    if ( arg == "-t" ) {
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
ExtractFileIdentifierApplication::execute()
{
   startSession();
   readFileGUIDs();
   for( std::vector< std::pair< std::string, std::string > >::const_iterator iFile = m_fidAndPfn.begin();
	iFile != m_fidAndPfn.end(); ++iFile ) {
      std::cout << iFile->first << " (" << iFile->second << ")" << std::endl; 
   }
}


void
ExtractFileIdentifierApplication::printSyntax()
{
  std::cerr << "Syntax : " << m_executableName << " [-t technologyType] files" << std::endl;
}




int main ATLAS_NOT_THREAD_SAFE ( int argc, char* argv[] )
{
   ExtractFileIdentifierApplication	app( argc, argv );
   return app.run();
}
