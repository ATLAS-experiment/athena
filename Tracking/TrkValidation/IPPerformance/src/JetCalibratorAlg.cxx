#include <IPPerformance/JetCalibratorAlg.h>

#include <xAODCore/ShallowCopy.h>


JetCalibratorAlg ::
JetCalibratorAlg(const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
{}



StatusCode JetCalibratorAlg ::initialize()
{
  ANA_CHECK(m_calibrationTool.retrieve());
  ANA_CHECK(m_inputJets.initialize());
  ANA_CHECK(m_outputJets.initialize());

  return StatusCode::SUCCESS;
}



StatusCode JetCalibratorAlg ::execute()
{

  SG::ReadHandle<xAOD::JetContainer> jets(m_inputJets);

  if (!jets.isValid())
  {
    ATH_MSG_ERROR("Cannot retrieve jets: " << m_inputJets.key());
    return StatusCode::FAILURE;
  }


  auto calibratedJets = xAOD::shallowCopyContainer(*jets);


  ANA_CHECK(m_calibrationTool->applyCalibration(*calibratedJets.first));


  SG::WriteHandle<xAOD::JetContainer> output(m_outputJets);

  ANA_CHECK(output.record(std::unique_ptr<xAOD::JetContainer>(calibratedJets.first),
                          std::unique_ptr<xAOD::ShallowAuxContainer>(calibratedJets.second)));


  return StatusCode::SUCCESS;

}

