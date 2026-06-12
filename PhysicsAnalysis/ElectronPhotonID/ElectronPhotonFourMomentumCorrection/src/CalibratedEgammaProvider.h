/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


//athena friendly provider of egamma calibrations - applies the egammaCalibTool to collections 
//author: will buttinger


#ifndef CALIBRATEDEGAMMAPROVIDER_H
#define CALIBRATEDEGAMMAPROVIDER_H

// Gaudi/Athena include(s):
#include "AthenaBaseComps/AthAlgorithm.h"
#include "AsgTools/ToolHandle.h"

// Local include(s):
#include "EgammaAnalysisInterfaces/IEgammaCalibrationAndSmearingTool.h"
#include "xAODCore/ShallowCopy.h"

#include "xAODEgamma/ElectronContainer.h" 
#include "xAODEgamma/PhotonContainer.h" 
#include "xAODEgamma/EgammaContainer.h" 
#include "xAODEventInfo/EventInfo.h"
namespace CP {

class CalibratedEgammaProvider : public AthAlgorithm {

public:
  /// Regular Algorithm constructor
  CalibratedEgammaProvider( const std::string& name, ISvcLocator* svcLoc );
  
  /// Function initialising the algorithm
  virtual StatusCode initialize();
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx);
  
private:
   SG::ReadHandleKey<xAOD::EventInfo> m_evtInfoKey{this, "EvtInfoKey","EventInfo", "Specify an Event info"};
   SG::ReadHandleKey<xAOD::EgammaContainer> m_inputKey{this, "Input","Electrons", "Electron or photon input collection to calibrate"};
   SG::WriteHandleKey<xAOD::EgammaContainer> m_outputKey{this, "Output","CalibratedElectrons", "Name of output collection. If same as input key, will try to modify in-situ"};
   ToolHandle<CP::IEgammaCalibrationAndSmearingTool> m_tool{this, "Tool", "", "Leave blank to get an autoconfigured instance"  };
   

}; // class 

} //namespace
#endif // CALIBRATEDEGAMMAPROVIDER_H
