/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConfigCondAlg.h"
#include "Identifier/IdentifierHash.h"
#include "GaudiKernel/EventIDRange.h"
#include <memory>
#include <sstream>
#include <fstream>
#include "PixelConditionsData/PixelModuleDataStream.h"
#include "PathResolver/PathResolver.h"


PixelConfigCondAlg::PixelConfigCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
  ::AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode PixelConfigCondAlg::initialize() {
  ATH_MSG_DEBUG("PixelConfigCondAlg::initialize()");

  ATH_CHECK(m_writeKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode PixelConfigCondAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("PixelConfigCondAlg::execute()");

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
  writeCdo -> setDefaultDBMAnalogThreshold(m_DBMAnalogThreshold);
  writeCdo -> setDefaultBarrelAnalogThresholdSigma(m_BarrelAnalogThresholdSigma);
  writeCdo -> setDefaultEndcapAnalogThresholdSigma(m_EndcapAnalogThresholdSigma);
  writeCdo -> setDefaultDBMAnalogThresholdSigma(m_DBMAnalogThresholdSigma);
  writeCdo -> setDefaultBarrelAnalogThresholdNoise(m_BarrelAnalogThresholdNoise);
  writeCdo -> setDefaultEndcapAnalogThresholdNoise(m_EndcapAnalogThresholdNoise);
  writeCdo -> setDefaultDBMAnalogThresholdNoise(m_DBMAnalogThresholdNoise);
  writeCdo -> setDefaultBarrelInTimeThreshold(m_BarrelInTimeThreshold);
  writeCdo -> setDefaultEndcapInTimeThreshold(m_EndcapInTimeThreshold);
  writeCdo -> setDefaultDBMInTimeThreshold(m_DBMInTimeThreshold);
  writeCdo -> setBarrelThermalNoise(m_BarrelThermalNoise);
  writeCdo -> setEndcapThermalNoise(m_EndcapThermalNoise);
  writeCdo -> setDBMThermalNoise(m_DBMThermalNoise);
  
  // Charge calibration parameters
  writeCdo -> setDefaultQ2TotA(m_CalibrationParameterA);
  writeCdo -> setDefaultQ2TotE(m_CalibrationParameterE);
  writeCdo -> setDefaultQ2TotC(m_CalibrationParameterC);
  writeCdo -> setPIXLinearExtrapolation(m_doPIXLinearExtrapolation);

  // DCS parameters
  writeCdo -> setDefaultBiasVoltage(m_biasVoltage);

  // Cabling parameters
  writeCdo -> setCablingMapToFile(m_cablingMapToFile);
  writeCdo -> setCablingMapFileName(m_cablingMapFileName);

  // Year-dependent conditions
  int currentRunNumber = ctx.eventID().run_number();
  std::string filename = getFileName(currentRunNumber);
  if (filename.empty()) {
    return StatusCode::FAILURE;
  }
  std::ifstream indata(filename.c_str());
  SoshiFormat formatter(currentRunNumber < m_Run1IOV);
  //stream through Soshi format to writeCdo
  indata>> formatter >>(*writeCdo);
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

std::string 
PixelConfigCondAlg::getFileName(const int currentRunNumber) const {
  if (m_usePrivateFileName.empty()) {
    std::string fname = PathResolverFindCalibFile(static_cast<std::string>(m_conditionsFolder)+m_conditionsFileName);
    if (fname.empty()) {
      return fname;
    }
    std::ifstream indata(fname);
    int runNumber = 0;
    std::string subfilename;
    indata >> runNumber;
    if (runNumber<0 or runNumber>1'000'000) {
      ATH_MSG_ERROR("Run number outside sensible range: "<<runNumber);
      return "";
    }
    while (currentRunNumber>=runNumber) {
      indata >> subfilename;
      if (indata.eof()) { break; }
      indata >> runNumber;
    }
    ATH_MSG_DEBUG("PixelConfigCondAlg::getFileName() RunNumber=" << currentRunNumber << " IOV=" << runNumber << " filename=" << subfilename);
    return PathResolverFindCalibFile(m_conditionsFolder+subfilename);
  } else {
    return m_usePrivateFileName;
  }
}

