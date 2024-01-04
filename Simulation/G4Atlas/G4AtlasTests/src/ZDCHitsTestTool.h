/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4AT_ZDCHITSTESTTOOL
#define G4AT_ZDCHITSTESTTOOL

#include "SimTestToolBase.h"
#include "ZdcIdentifier/ZdcID.h"


class ZDCHitsTestTool : public SimTestToolBase {


public:

  ZDCHitsTestTool(const std::string& type, const std::string& name, const IInterface* parent);

  StatusCode initialize(); 

  StatusCode processEvent();
 private:

  // globals
  TH1 *m_zdc[2][4],*m_rpd[2][16];
  const ZdcID* m_ZdcID = nullptr;

};

#endif
