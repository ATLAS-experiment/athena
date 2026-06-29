/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file PixelDigitization/RD53SimTool.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date January, 2020
 * @brief RD53 simulation
 */

#ifndef PIXELDIGITIZATION_RD53SimTool_H
#define PIXELDIGITIZATION_RD53SimTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "FrontEndSimTool.h"
#include "InDetRawData/PixelRDO_Collection.h" //typedef
#include "PixelConditionsData/ITkPixSimulationParameters.h" 


class SiChargedDiodeCollection;

namespace CLHEP{
  class HepRandomEngine;
}

class RD53SimTool: public FrontEndSimTool {
public:
  RD53SimTool(const std::string& type, const std::string& name, const IInterface* parent);

  virtual StatusCode initialize() override;
  virtual void process(const EventContext& ctx,
                       SiChargedDiodeCollection& chargedDiodes, PixelRDO_Collection& rdoCollection,
                       CLHEP::HepRandomEngine* rndmEngine) const override;
private:
  
   ITkPixSimulationParameters m_chipSim{};

  RD53SimTool();
  Gaudi::Property<bool> m_doTimeWalk {
    this, "DoTimeWalk",false,"include time-walk effects"
      };
  Gaudi::Property<int> m_overDrive {
    this, "OverDrive",150,"value of overdrive (in-time threshold - absolute threshold) in electrons"
      };
};

#endif // PIXELDIGITIZATION_RD53SimTool_H
