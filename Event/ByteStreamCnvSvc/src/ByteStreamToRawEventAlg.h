/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef BYTESTREAMCNVSVC_BYTESTREAMTORAWEVENT_H
#define BYTESTREAMCNVSVC_BYTESTREAMTORAWEVENT_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "ByteStreamCnvSvcBase/IByteStreamCnvSvc.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "CxxUtils/checker_macros.h"
#include <Gaudi/Property.h>

/**
 * @class ByteStreamToRawEventAlg
 *
 * @brief Algorithm that converts detector byte streams into a raw event and
 * feeds it to a IROBDataProviderSvc.
 *
 * It's useful for testing; e.g.:
 * RDO -> detector specific encoder -> BS -> **ByteStreamToRawEventAlg** ->
 * ROBDataProvider -> BS -> detector specific decoder -> RDO'
 * Then one can check that RDO == RDO'
 *
 * @author Krzysztof Ciesla <krzysztof.marcin.ciesla@cern.ch>
 * Slightly modified by Fabrice Le Goff <flegoff@cern.ch>
 */

class ByteStreamToRawEventAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  ServiceHandle<IByteStreamCnvSvc> m_byteStreamCnvSvc{this,
    "ByteStreamConvertionService", "ByteStreamCnvSvc", ""};

  ServiceHandle<IROBDataProviderSvc> m_robsSvc{this,
    "ROBDataProvider", "ROBDataProviderSvc/ROBDataProviderSvc"};

  Gaudi::Property<std::vector<std::string>> m_RawContainers{this,
    "RawContainers", {}, "Names of the containers holding the byte stream data"};

  struct Cache {
    std::mutex m_mutex;
    RawEvent m_event;
    std::vector<uint32_t> m_data;
  };

  mutable SG::SlotSpecificObj<Cache> m_eventsCache ATLAS_THREAD_SAFE; // Access to Cache objects must be protected by their internal mutex
};

#endif  // BYTESTREAMCNVSVC_BYTESTREAMTORAWEVENT_H