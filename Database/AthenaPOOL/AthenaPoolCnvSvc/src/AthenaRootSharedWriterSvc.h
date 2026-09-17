/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAROOTSHAREDWRITERSVC_H
#define ATHENAROOTSHAREDWRITERSVC_H

/** @file AthenaRootSharedWriterSvc.h
 *  @brief This file contains the class definition for the AthenaRootSharedWriterSvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "AthenaBaseComps/AthService.h"
#include "AthenaKernel/IAthenaSharedWriterSvc.h"
#include "AthenaPoolSharedIOCnvSvc.h"

#include "THashTable.h"
#include <unordered_map>

class TServerSocket;
class TMonitor;

/** @class AthenaRootSharedWriterSvc
 *  @brief This class provides an example for writing event data objects to Pool.
 **/
class AthenaRootSharedWriterSvc : public extends<AthService, IAthenaSharedWriterSvc> {
   // Allow the factory class access to the constructor
   friend class SvcFactory<AthenaRootSharedWriterSvc>;

public: // Constructor and Destructor
   /// Standard Service Constructor
   AthenaRootSharedWriterSvc(const std::string& name, ISvcLocator* pSvcLocator);
   /// Destructor
   virtual ~AthenaRootSharedWriterSvc() = default;

public:
/// Gaudi Service Interface method implementations:
   virtual StatusCode initialize() override;
   virtual StatusCode stop() override;
   virtual StatusCode finalize() override;

   virtual StatusCode share(int numClients = 0, bool motherClient = false) override;

private:
   ServiceHandle<AthenaPoolSharedIOCnvSvc> m_cnvSvc{this,"AthenaPoolSharedIOCnvSvc","AthenaPoolSharedIOCnvSvc"};

   TServerSocket* m_rootServerSocket;
   TMonitor* m_rootMonitor;
   /// Path of the UNIX domain socket file, if used, removed on finalize
   std::string m_socketPath;
   THashTable m_rootMergers;
   std::unordered_map<TClass*, void*> m_cachedObjects;
   int m_rootClientIndex;
   int m_rootClientCount;
   int m_numberOfStreams;
};

#endif
