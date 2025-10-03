/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4SHOWERLIBSVC_G4SHOWERLIBSVCTEST_H
#define G4SHOWERLIBSVC_G4SHOWERLIBSVCTEST_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "LArG4ShowerLibSvc/ILArG4ShowerLibSvc.h"

class LArG4ShowerLibSvcTest : public AthAlgorithm {

public:

  LArG4ShowerLibSvcTest (const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~LArG4ShowerLibSvcTest () = default;

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual StatusCode execute() override;

private:
  ServiceHandle<ILArG4ShowerLibSvc> m_showerLibSvc{this, "LArG4ShowerLibSvc", "LArG4ShowerLibSvc"};
};

#endif // G4SHOWERLIBSVC_G4SHOWERLIBSVCTEST_H
