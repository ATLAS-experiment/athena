/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file PixelDigitization/FEI4SimTool.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date January, 2020
 * @brief FEI4 simulation
 */

#ifndef PIXELDIGITIZATION_FEI4SimTool_H
#define PIXELDIGITIZATION_FEI4SimTool_H

#include "FrontEndSimTool.h"
#include "InDetRawData/PixelRDO_Collection.h" //typedef
#include "PixelConditionsData/PixelModuleData.h"  //ReadCondHandleKey template param

class SiChargedDiodeCollection;
namespace CLHEP{
  class HepRandomEngine;
}

class FEI4SimTool: public FrontEndSimTool {
public:
  FEI4SimTool(const std::string& type, const std::string& name, const IInterface* parent);

  virtual StatusCode initialize();
  virtual StatusCode finalize();
  virtual ~FEI4SimTool();
  virtual void process(SiChargedDiodeCollection& chargedDiodes, PixelRDO_Collection& rdoCollection,
                       CLHEP::HepRandomEngine* rndmEngine) const;
private:
   SG::ReadCondHandleKey<PixelModuleData> m_moduleDataKey{
    this, "PixelModuleData", "PixelModuleData", "Pixel module data"
  };

  FEI4SimTool();
};

#endif // PIXELDIGITIZATION_FEI4SimTool_H
