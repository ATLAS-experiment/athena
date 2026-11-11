/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STORAGESVC_SIMPLEUTILITYBASE
#define STORAGESVC_SIMPLEUTILITYBASE

#include <iostream>
#include <stdexcept>
#include <vector>

#include "StorageSvc/IStorageSvc.h"
#include "StorageSvc/DbType.h"
#include "StorageSvc/DbOption.h"
#include "StorageSvc/FileDescriptor.h"

#include "CxxUtils/checker_macros.h"

   
/// Common base class for POOL utilities
class SimpleUtilityBase {
    public:
     /// constructor taking commandline parmeters
     SimpleUtilityBase( int argc = 0, char* argv[] = 0 );
     virtual ~SimpleUtilityBase();

     virtual int	run();

     virtual bool 	parseArguments(); 
     virtual void 	startSession ATLAS_NOT_THREAD_SAFE ();
     virtual void 	readFileGUIDs();
     virtual std::string readFileGUID( const std::string& pfn );

     virtual void 	execute() {}
     virtual void 	printSyntax() {}

    /// initialize the Gaudi framework for standalone executables
    static bool initGaudi();

    protected:
     std::string	              m_executableName;
     std::string                m_technologyName;
     IStorageSvc*               m_storageSvc = nullptr;

     std::vector< std::string > m_args;
     std::vector< std::string > m_fileNames;
     std::vector< std::pair< std::string, std::string > > m_fidAndPfn;
};


/// shortcut for reading file GUID from python
/* this is a class and not a namespace so it can be autoloaded by ROOT */
class Utils {
  public:
    static std::string        readFileGUID ATLAS_NOT_THREAD_SAFE ( const std::string& pfn );
};

#endif
