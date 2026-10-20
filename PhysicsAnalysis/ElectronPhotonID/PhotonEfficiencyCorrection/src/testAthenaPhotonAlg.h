/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PHOTONEFFICIENCYCORRECTION_TESTATHENAPHOTONALG_H
#define PHOTONEFFICIENCYCORRECTION_TESTATHENAPHOTONALG_H 1

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "ColumnarMode/ColumnarMode.h"

// forward declarations
class IAsgPhotonEfficiencyCorrectionTool;

namespace ADD_CMODE(CP) {

class testAthenaPhotonAlg: public ::AthAlgorithm { 
 public: 
  testAthenaPhotonAlg( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~testAthenaPhotonAlg(); 

  virtual StatusCode  initialize();
  virtual StatusCode  execute(const EventContext& ctx);
  virtual StatusCode  finalize();

 private: 
 /// The tool handle to our photon efficiency correction
 ToolHandle< IAsgPhotonEfficiencyCorrectionTool > m_photonSF;
}; 

} // namespace ADD_CMODE(CP)

#endif //> !PHOTONEFFICIENCYCORRECTION_TESTATHENAPHOTONALG_H
