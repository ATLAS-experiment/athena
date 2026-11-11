/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileFCSmStepToTileHitVec.h
// Author   : Sergey Karpov <Sergey.Karpov@cern.ch>
// Created  : Nov. 2013
//
// DESCRIPTION
//   TileFCSmStepToTileHitVec copies all TileHits from FCS_StepInfoCollection container
//   to TileHitVector_FCS with/without applying of new U-shape
//
// Properties (JobOption Parameters):
//   ISF_FCS_Parametrization::FCS_StepInfoCollection    string   Name of FCS_StepInfo container to read
//   TileHitVec_FCS                                     string   Name of TileHitVector to write
//   TileInfoName                                       string   Name of object in TDS with all parameters
//
// History:
//   01 Nov 2013 - Created from TileCellIDCToCell.h
//   28 Nov 2013 - Work with U-shape was added (Sasha Solodkov)
//
// BUGS:
//
//****************************************************************************

#ifndef TILEFASTCALOSIM_TILEFCSMSTEPTOTILEHITVEC_H
#define TILEFASTCALOSIM_TILEFCSMSTEPTOTILEHITVEC_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "TileG4Interfaces/ITileCalculator.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "Gaudi/Property.h"

class TileID;
class TileHit;
class TileInfo;
class TileDetDescrManager;
class TileGeoG4SDCalc;
class FCS_StepInfoCollection;

// C++ STL includes
#include <string>
#include <vector>

class TileFCSmStepToTileHitVec: public AthAlgorithm {
public:
  // Constructor
  TileFCSmStepToTileHitVec(const std::string& name, ISvcLocator* pSvcLocator);

  // Destructor
  virtual ~TileFCSmStepToTileHitVec();

  // Gaudi Hooks
  StatusCode initialize();
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();

private:
  int getUshapeFromGM() const;

  ServiceHandle<IGeoModelSvc> m_geoModSvc{this, "GeoModelSvc", "GeoModelSvc"};
  Gaudi::Property<std::string> m_FCS_StepInfo{this, "StepInfoCollection", "MergedEventSteps", "Name of input container (default=TileHitCnt)"};
  Gaudi::Property<std::string> m_hitVec{this, "TileHitVector", "TileHitVec_FCS", "Name of output hit vector (default=TileHitVec)"};
  Gaudi::Property<std::string> m_infoName{this, "TileInfoName", "TileInfo", "Name of TileInfo store (default=TileInfo"};

  const TileID*               m_tileID{nullptr};
  const TileInfo*             m_tileInfo{nullptr};
  const TileDetDescrManager*  m_tileMgr{nullptr};
  ServiceHandle<ITileCalculator> m_calc{this, "TileCalculator", "TileGeoG4SDCalc"};

  Gaudi::Property<float>   m_deltaT{this, "DeltaT", 0.5 * Gaudi::Units::nanosecond, "Minimal Time granularity in TileHit (default=0.5ns)"};
  std::vector<TileHit*> m_allHits;

  int m_uShape{-1};
};

#endif // TILERECALGS_TILEFCSMSTEPTOTILEHITVEC_H
