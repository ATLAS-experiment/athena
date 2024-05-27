/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file PixelDigitization/RadDamageUtil.h
 * @author Ben Nachman <Ben.Nachman@cern.ch>
 * @date January, 2020
 * @brief Utility tools for radiation damage simulation
 */

#ifndef PIXELDIGITIZATION_RADDAMAGEUTIL_H
#define PIXELDIGITIZATION_RADDAMAGEUTIL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Property.h"


#include "EfieldInterpolator.h" //ToolHandle template argument
#include <utility> // for std::pair
namespace InDetDD{
  class PixelModuleDesign;
}

class TH3F;
class TH2F;
class TH1F;

//====================
//  C L A S S   D E F
//====================
class RadDamageUtil: public AthAlgTool {
public:
  RadDamageUtil(const std::string& type, const std::string& name, const IInterface* parent);

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  virtual ~RadDamageUtil();
  StatusCode initTools();
  const StatusCode generateRamoMap(TH3F* ramPotentialMap, InDetDD::PixelModuleDesign* module);
  const StatusCode generateEfieldMap(TH1F*& eFieldMap, InDetDD::PixelModuleDesign* module);
  StatusCode generateEfieldMap(TH1F*& eFieldMap, InDetDD::PixelModuleDesign* module, double fluence, double biasVoltage,
                               int layer, const std::string& TCAD_list, bool interpolate);
  const StatusCode generateDistanceTimeMap(TH2F*& distanceMap_e, TH2F*& distanceMap_h, TH1F*& timeMap_e,
                                           TH1F*& timeMap_h, TH2F*& lorentzMap_e, TH2F*& lorentzMap_h, TH1F*& eFieldMap,
                                           InDetDD::PixelModuleDesign* module);

  const std::pair<double, double> getTrappingTimes(double fluence) const;
  static const std::pair<double, double> getMobility(double electricField, double temperature) ;
  static double getTanLorentzAngle(double electricField, double temperature, double bField, bool isHole) ;

  bool saveDebugMaps();
private:
  RadDamageUtil();

  Gaudi::Property<int> m_defaultRamo
  {
    this, "defaultRamo", 1, "Mapping strategy of Ramo potential"
  };

  Gaudi::Property<double> m_betaElectrons
  {
    this, "betaElectrons", 4.5e-16, "Used in trapping time calculation"
  };

  Gaudi::Property<double> m_betaHoles
  {
    this, "betaHoles", 6.0e-16, "Used in trapping time calculation"
  };

  Gaudi::Property<bool> m_saveDebugMaps
  {
    this, "saveDebugMaps", false, "Flag to save map"
  };

  static double alpha(int n, int Nrep, double a); //Poisson solution factor
  static double weighting3D(double x, double y, double z, int n, int m, int Nrep, double a, double b);
  static double weighting2D(double x, double z, double Lx, double sensorThickness);

  ToolHandle<EfieldInterpolator> m_EfieldInterpolator
  {
    this, "EfieldInterpolator", "EfieldInterpolator",
    "Create an Efield for fluence and bias volatge of interest based on TCAD samples"
  };
};

#endif //PIXELDIGITIZATION_RADDAMAGEUTIL_H
