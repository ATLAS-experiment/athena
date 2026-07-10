/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EFTRACKINGDATATRANSFER_ASYNCMPIOMPUTEALG_H
#define EFTRACKINGDATATRANSFER_ASYNCMPIOMPUTEALG_H

// Framework includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// STL includes
#include <string>

/**
 * @class AsyncMPIomputeAlg
 * @brief 
 **/
class AsyncMPIomputeAlg : public AthReentrantAlgorithm {
public:
  AsyncMPIomputeAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~AsyncMPIomputeAlg() override;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& context) const override;
  virtual StatusCode finalize() override;

private:
  //Gaudi::Property<int> m_myInt{this, "MyInt", 0, "An Integer"};
};

#endif // EFTRACKINGDATATRANSFER_ASYNCMPIOMPUTEALG_H
