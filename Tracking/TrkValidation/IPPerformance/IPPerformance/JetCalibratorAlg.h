#ifndef IPPerformance_JetCalibrator_H
#define IPPerformance_JetCalibrator_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include "JetCalibTools/JetCalibrationTool.h"

#include <xAODJet/JetContainer.h>

#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>


class JetCalibratorAlg : public AthAlgorithm
{

public:

  JetCalibratorAlg(const std::string& name, ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;


private:

  ToolHandle<JetCalibrationTool> m_calibrationTool{this, "calibrationTool","Jet calibration tool"};

  SG::ReadHandleKey<xAOD::JetContainer> m_inputJets{this, "InputJets", "AntiKt4EMTopoJets", "Input jet container"};

  SG::WriteHandleKey<xAOD::JetContainer> m_outputJets{this, "OutputJets", "Jets_Calib", "Output jet container"};

};


#endif
