/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENASHAREDMEMORYTOOL_H
#define ATHENASHAREDMEMORYTOOL_H

/** @file AthenaSharedMemoryTool.h
 *  @brief This file contains the class definition for the AthenaSharedMemoryTool class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/IAthenaIPCTool.h"

#include <memory>
#include <set>
#include <string>

// Forward declarations.
class IIncidentSvc;

namespace boost {
   namespace interprocess {
      class shared_memory_object;
      class mapped_region;
   }
}

/** @class AthenaSharedMemoryTool
 *  @brief This class provides the IPCTool for SharedMemory objects
 **/
class AthenaSharedMemoryTool : public extends<::AthAlgTool, IAthenaIPCTool> {
public: 
   /// Standard Service Constructor
   AthenaSharedMemoryTool(const std::string& type, const std::string& name, const IInterface* parent);
   /// Destructor
   virtual ~AthenaSharedMemoryTool();

   /// Gaudi Service Interface method implementations:
   virtual StatusCode initialize() override;
   virtual StatusCode stop() override;
   virtual StatusCode finalize() override;

   virtual StatusCode makeServer(int num, const std::string& streamPortSuffix) override;
   virtual bool isServer() const override;
   virtual StatusCode makeClient(int num, std::string& streamPortSuffix) override;
   virtual bool isClient() const override;

   virtual StatusCode putEvent ATLAS_NOT_THREAD_SAFE (long eventNumber, const void* source, size_t nbytes, unsigned int status) const override;
   virtual StatusCode getLockedEvent(void** target, unsigned int& status) const override;
   virtual StatusCode lockEvent(long eventNumber) const override;

   virtual StatusCode putObject(const void* source, size_t nbytes, int num = 0) override;
   virtual StatusCode getObject(void** target, size_t& nbytes, int num = 0) override;
   virtual StatusCode clearObject(const char** tokenString, int& num) override;
   virtual StatusCode lockObject(const char* tokenString, int num = 0) override;

private:
   Gaudi::Property<std::string> m_sharedMemory{this, "SharedMemoryName", {}};

   const size_t m_maxSize{64 * 1024 * 1024};
   const int m_maxDataClients{256};
   int m_num{-1};
   int m_lastClient{-1};
   std::set<int> m_dataClients;
   std::unique_ptr<boost::interprocess::mapped_region> m_payload;
   std::unique_ptr<boost::interprocess::mapped_region> m_status;
   long m_fileSeqNumber{0};
   bool m_isServer{false};
   bool m_isClient{false};
   ServiceHandle<IIncidentSvc> m_incidentSvc;
};

#endif
