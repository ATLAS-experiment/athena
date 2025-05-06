/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file PixelDigitization/SensorSim3DTool.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date January, 2020
 * @brief 3D pixel sensor simulation
 */

#ifndef PIXELDIGITIZATION_SensorSim3DTool_H
#define PIXELDIGITIZATION_SensorSim3DTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "SensorSimTool.h"

#include "GaudiKernel/ToolHandle.h"
#include "RadDamageUtil.h"
#include "PixelConditionsData/PixelHistoConverter.h"
#include "PixelReadoutDefinitions/PixelReadoutDefinitions.h" //PixelReadoutTechnology enum class
#include <vector>
#include <map> //std::multimap, and std::pair

namespace InDetDD{
  class SiDetectorElement;
  class PixelModuleDesign;
}

namespace CLHEP{
  class HepRandomEngine;
}

class EventContext;

class SensorSim3DTool: public SensorSimTool {
public:
  SensorSim3DTool(const std::string& type, const std::string& name, const IInterface* parent);
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual ~SensorSim3DTool();

  virtual StatusCode induceCharge(const TimedHitPtr<SiHit>& phit,
                                  SiChargedDiodeCollection& chargedDiodes,
                                  const InDetDD::SiDetectorElement& Module,
                                  const InDetDD::PixelModuleDesign& p_design,
                                  std::vector< std::pair<double, double> >& trfHitRecord,
                                  std::vector<double>& initialConditions,
                                  CLHEP::HepRandomEngine* rndmEngine,
                                  const EventContext &ctx) const override;


  // 3D sensor simulation using probability density map (used in RUN-2 (no radiation damage)
  StatusCode readProbMap(const std::string&);
  StatusCode printProbMap(const std::string&) const;

  double getMobility(double electricField, bool isHoleBit) const;
  std::vector<double> getDriftTime(bool isHoleBit, size_t number,
                                   CLHEP::HepRandomEngine* rndmEngine,
                                   double trappingTimeElectrons,
                                   double trappingTimeHoles) const;

private:
  SensorSim3DTool();

  double getProbMapEntry(const InDetDD::PixelReadoutTechnology &, int, int) const;

  // 3D sensor simulation using probability density map (used in RUN-2 (no radiation damage)
  std::multimap<std::pair<int, int>, double> m_probMapFEI4;
  std::multimap<std::pair<int, int>, double> m_probMapFEI3;

  std::vector<PixelHistoConverter> m_chargeCorrection;
  
  Gaudi::Property<std::string> m_cc_prob_file_fei3
  {
    this, "CCProbMapFileFEI3", "PixelDigitization/3DFEI3-3E-problist-1um_v1.txt",
    "Input probability file for 3D FEI3 sensor."
  };

  Gaudi::Property<std::string> m_cc_prob_file_fei4
  {
    this, "CCProbMapFileFEI4", "PixelDigitization/3DFEI4-2E-problist-1um_v0.txt",
    "Input probability file for 3D FEI4 sensor."
  };

  Gaudi::Property<int> m_numberOfSteps
  {
    this, "numberOfSteps", 50, "Number of steps for Pixel3D module"
  };

  Gaudi::Property<bool> m_doChunkCorrection
  {
    this, "doChunkCorrection", false, "doChunkCorrection bool: should be flag"
  };

  Gaudi::Property<double> m_temperature
  {
    this, "Temperature", 300.0, "Default temperature [K]"
  };

  ToolHandle<RadDamageUtil> m_radDamageUtil
  {
    this, "RadDamageUtil", "RadDamageUtil", "Rad Damage utility"
  };

};

#endif // PIXELDIGITIZATION_SensorSim3DTool_H
