/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file PixelDigitization/SensorSimTool.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date January, 2020
 * @brief Base class of pixel sensor simulation
 */

#ifndef PIXELDIGITIZATION_SensorSimTool_H
#define PIXELDIGITIZATION_SensorSimTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "InDetSimEvent/SiHit.h"

#include "HitManagement/TimedHitPtr.h" //template
#include "PixelConditionsData/PixelModuleData.h"
#include "PixelConditionsData/PixelRadiationDamageFluenceMapData.h"
#include "SiPropertiesTool/ISiPropertiesTool.h"

namespace InDetDD{
  class SiDetectorElement;
  class PixelModuleDesign;
}

namespace CLHEP{
  class HepRandomEngine;
}

class SiChargedDiodeCollection;


static const InterfaceID IID_ISensorSimTool("SensorSimTool", 1, 0);

class SensorSimTool: public AthAlgTool, virtual public IAlgTool {
public:

  enum RadiationDamageSimulationType {
    NO_RADIATION_DAMAGE = 0,
    RAMO_POTENTIAL = 1,
    TEMPLATE_CORRECTION = 2
  };

  SensorSimTool(const std::string& type, const std::string& name, const IInterface* parent) :
    AthAlgTool(type, name, parent) {
    declareInterface<SensorSimTool>(this);
  }

  static const InterfaceID& interfaceID() {return IID_ISensorSimTool;}

  virtual StatusCode initialize() {
    ATH_CHECK(AthAlgTool::initialize());
    ATH_CHECK(m_siPropertiesTool.retrieve());
    ATH_CHECK(m_moduleDataKey.initialize());
    ATH_CHECK(m_fluenceDataKey.initialize(m_radiationDamageSimulationType != RadiationDamageSimulationType::NO_RADIATION_DAMAGE && !m_fluenceDataKey.empty()));

    return StatusCode::SUCCESS;
  }

  virtual StatusCode finalize() {return StatusCode::FAILURE;}
  virtual ~SensorSimTool() {}
  virtual StatusCode induceCharge(const TimedHitPtr<SiHit>& phit,
                                  SiChargedDiodeCollection& chargedDiodes,
                                  const InDetDD::SiDetectorElement& Module,
                                  const InDetDD::PixelModuleDesign& p_design,
                                  std::vector< std::pair<double, double> >& trfHitRecord,
                                  std::vector<double>& initialConditions,
                                  CLHEP::HepRandomEngine* rndmEngine,
                                  const EventContext &ctx) const = 0;
private:
  SensorSimTool();
protected:
  ToolHandle<ISiPropertiesTool> m_siPropertiesTool
  {
    this, "SiPropertiesTool", "SiPropertiesTool", "Tool to retrieve SiProperties"
  };

  SG::ReadCondHandleKey<PixelModuleData> m_moduleDataKey
  {
    this, "PixelModuleData", "PixelModuleData", "Pixel module data"
  };

  Gaudi::Property<int> m_radiationDamageSimulationType
  {
    this, "RadiationDamageSimulationType", RadiationDamageSimulationType::NO_RADIATION_DAMAGE, "Option to simualte the effects of radiation damage"
  };

  Gaudi::Property<std::string> m_templateCorrectionRootFile
  {
    this, "TemplateCorrectionROOTfile", "",
    "Path to the ROOT file with histograms for radiation damage template corrections"
  };
  
  Gaudi::Property<std::vector<std::string> > m_lorentzAngleCorrectionHistos
  {
    this, "LorentzAngleCorrectionHistos", {},
    "Paths to the histograms inside the ROOT file for Lorentz angle correction"
  };

  Gaudi::Property<std::vector<std::string> > m_chargeCorrectionHistos
  {
    this, "ChargeCorrectionHistos", {},
    "Paths to the histograms inside the ROOT file for radiation damage charge correction"
  };

  Gaudi::Property<std::vector<std::string> > m_distanceCorrectionHistos
  {
    this, "DistanceCorrectionHistos", {},
    "Paths to the histograms inside the ROOT file for radiation damage distance correction"
  };

  SG::ReadCondHandleKey<PixelRadiationDamageFluenceMapData> m_fluenceDataKey
  {
    this, "PixelRadiationDamageFluenceMapData", "PixelRadiationDamageFluenceMapData", "Pixel fluence map data for radiation damage"
  };

  
  Gaudi::Property<bool> m_digitizeITk3Das3D
  {
    this, "DigitizeITk3Das3D", false,
    "Flag to tell the code if the 3D sensors for ITK should be treated as 3D or as planar sensors for digitization"
  };

};

#endif // PIXELDIGITIZATION_SensorSimTool_H
