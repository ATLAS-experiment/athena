/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file PixelDigitization/FEI3SimTool.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date January, 2020
 * @brief FEI3 simulation
 */

#ifndef PIXELDIGITIZATION_FEI3SimTool_H
#define PIXELDIGITIZATION_FEI3SimTool_H

#include "FrontEndSimTool.h"
#include "InDetRawData/PixelRDO_Collection.h" //typedef
#include "PixelConditionsData/PixelModuleData.h"  //ReadCondHandleKey template param

class SiChargedDiodeCollection;
class SiTotalCharge;
class PixelModuleData;

namespace CLHEP{
  class HepRandomEngine;
}

class FEI3SimTool: public FrontEndSimTool {
public:
  FEI3SimTool(const std::string& type, const std::string& name, const IInterface* parent);

  virtual StatusCode initialize();
  virtual StatusCode finalize();
  virtual ~FEI3SimTool();
  virtual void process(SiChargedDiodeCollection& chargedDiodes, PixelRDO_Collection& rdoCollection,
                       CLHEP::HepRandomEngine* rndmEngine) const;
private:
   SG::ReadCondHandleKey<PixelModuleData> m_moduleDataKey{
    this, "PixelModuleData", "PixelModuleData", "Pixel module data"
  };


  Gaudi::Property<bool> m_duplication{
    this, "HitDuplication", false, "Turn on Hit Duplication in subsequent timebin for small hits"
  };

  int relativeBunch2009(const double threshold, const double intimethreshold, const SiTotalCharge& totalCharge,
                        CLHEP::HepRandomEngine* rndmEngine) const;

  double getProbability(const std::vector<float> &bounds, const std::vector<float> &probs, const double &val) const;

};

#endif // PIXELDIGITIZATION_FEI3SimTool_H
