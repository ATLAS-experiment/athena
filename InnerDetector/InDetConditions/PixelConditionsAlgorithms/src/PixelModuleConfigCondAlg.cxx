/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelModuleConfigCondAlg.h"
#include "Identifier/IdentifierHash.h"
#include "GaudiKernel/EventIDRange.h"
#include <memory>
#include <sstream>

#include "PathResolver/PathResolver.h"


PixelModuleConfigCondAlg::PixelModuleConfigCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
  ::AthCondAlgorithm(name, pSvcLocator)
{
}

StatusCode 
PixelModuleConfigCondAlg::initialize() {
  ATH_MSG_DEBUG("PixelModuleConfigCondAlg::initialize()");
  return m_writeKey.initialize();
}

StatusCode PixelModuleConfigCondAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("PixelModuleConfigCondAlg::execute()");

  SG::WriteCondHandle<PixelModuleData> writeHandle(m_writeKey, ctx);
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid.. In theory this should not be called, but may happen if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS;
  }

  // Construct the output Cond Object and fill it in
  std::unique_ptr<PixelModuleData> writeCdo(std::make_unique<PixelModuleData>());

  const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT,                     0,
                                              0, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop {EventIDBase::UNDEFNUM,   EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1,
                          EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};

  writeCdo -> setDefaultBarrelAnalogThreshold(m_BarrelAnalogThreshold);
  writeCdo -> setDefaultEndcapAnalogThreshold(m_EndcapAnalogThreshold);
  writeCdo -> setDefaultBarrelAnalogThresholdSigma(m_BarrelAnalogThresholdSigma);
  writeCdo -> setDefaultEndcapAnalogThresholdSigma(m_EndcapAnalogThresholdSigma);
  writeCdo -> setDefaultBarrelAnalogThresholdNoise(m_BarrelAnalogThresholdNoise);
  writeCdo -> setDefaultEndcapAnalogThresholdNoise(m_EndcapAnalogThresholdNoise);
  writeCdo -> setDefaultBarrelInTimeThreshold(m_BarrelInTimeThreshold);
  writeCdo -> setDefaultEndcapInTimeThreshold(m_EndcapInTimeThreshold);
  writeCdo -> setBarrelThermalNoise(m_BarrelThermalNoise);
  writeCdo -> setEndcapThermalNoise(m_EndcapThermalNoise);
  
  // FEI3-specific parameters... only write if they are set
  if (!m_FEI3BarrelLatency.empty()) {
    // Use the above as the check... worth adding checks to see that all FEI3 vectors are non-zero?
    writeCdo -> setFEI3BarrelLatency(m_FEI3BarrelLatency);
    writeCdo -> setFEI3BarrelTimingSimTune(m_FEI3BarrelTimingSimTune);
    writeCdo -> setFEI3EndcapLatency(m_FEI3EndcapLatency);
    writeCdo -> setFEI3EndcapTimingSimTune(m_FEI3EndcapTimingSimTune);
  }

  // DBM-specific parameters... only write if they are set
  if (!m_DBMToTThreshold.empty()) {
    writeCdo -> setDefaultDBMAnalogThreshold(m_DBMAnalogThreshold);
    writeCdo -> setDefaultDBMAnalogThresholdSigma(m_DBMAnalogThresholdSigma);
    writeCdo -> setDefaultDBMAnalogThresholdNoise(m_DBMAnalogThresholdNoise);
    writeCdo -> setDefaultDBMInTimeThreshold(m_DBMInTimeThreshold);
    writeCdo -> setDBMThermalNoise(m_DBMThermalNoise);
  }

  // Charge calibration parameters
  writeCdo -> setDefaultQ2TotA(m_CalibrationParameterA);
  writeCdo -> setDefaultQ2TotE(m_CalibrationParameterE);
  writeCdo -> setDefaultQ2TotC(m_CalibrationParameterC);

  // DCS parameters
  writeCdo -> setDefaultBiasVoltage(m_biasVoltage);


  // Cabling parameters
  writeCdo -> setCablingMapToFile(m_cablingMapToFile);
  writeCdo -> setCablingMapFileName(m_cablingMapFileName);

  // mapping files for radiation damage simulation
  std::vector<std::string> mapsPath_list;
  std::vector<std::string> mapsPath_list3D;

  writeCdo -> setBarrelToTThreshold(m_BarrelToTThreshold);
  writeCdo -> setBarrelCrossTalk(m_BarrelCrossTalk);
  writeCdo -> setBarrelNoiseOccupancy(m_BarrelNoiseOccupancy);
  writeCdo -> setBarrelDisableProbability(m_BarrelDisableProbability);

  writeCdo -> setEndcapToTThreshold(m_EndcapToTThreshold);
  writeCdo -> setEndcapCrossTalk(m_EndcapCrossTalk);
  writeCdo -> setEndcapNoiseOccupancy(m_EndcapNoiseOccupancy);
  writeCdo -> setEndcapDisableProbability(m_EndcapDisableProbability);

  writeCdo -> setBarrelNoiseShape({m_InnermostNoiseShape, m_NextInnermostNoiseShape, m_PixelNoiseShape, m_PixelNoiseShape, m_PixelNoiseShape});
  writeCdo -> setEndcapNoiseShape(std::vector<std::vector<float>>(m_EndcapToTThreshold.size(), m_PixelNoiseShape));

  // Radiation damage simulation
  writeCdo -> setFluenceLayer(m_BarrelFluence);
  for (size_t i=0; i<m_BarrelFluenceMap.size(); i++) {
    mapsPath_list.push_back(PathResolverFindCalibFile(m_BarrelFluenceMap[i]));
  }

  // Radiation damage simulation for 3D sensor
  writeCdo -> setFluenceLayer3D(m_3DFluence);
  for (size_t i=0; i<m_3DFluenceMap.size(); i++) {
    mapsPath_list3D.push_back(PathResolverFindCalibFile(m_3DFluenceMap[i]));
  }

  //=======================
  // Combine time interval
  //=======================
  EventIDRange rangeW{start, stop};
  if (rangeW.stop().isValid() && rangeW.start()>rangeW.stop()) {
    ATH_MSG_FATAL("Invalid intersection rangeW: " << rangeW);
    return StatusCode::FAILURE;
  }

  if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
    ATH_MSG_FATAL("Could not record PixelModuleData " << writeHandle.key() << " with EventRange " << rangeW << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

  return StatusCode::SUCCESS;
}
