// -*- C++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHEXSTOREGATEEXAMPLE_READDATA_H
#define ATHEXSTOREGATEEXAMPLE_READDATA_H 1

#include <string>
#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthExStoreGateExample/MyDataObj.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"


/**
 * Example of a legacy Algorithm in single-threaded athena.
 *
 * IMPORTANT: This is no longer recommended. See ReadDataReentrant instead.
 */
class ReadData:public AthAlgorithm {
public:
  using AthAlgorithm::AthAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

private:
  //Properties
  Gaudi::Property<std::string> m_DataProducer{this, "DataProducer", ""};
  SG::ReadHandle<MyDataObj> m_dobj3;
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EvtInfo", "EventInfo", "EventInfo name"};
};

#endif // not ATHEXSTOREGATEEXAMPLE_READDATA_H








