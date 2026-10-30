/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILETOPCALIBALG_H
#define TILETOPCALIBALG_H

// ********************************************************************
//
// NAME:     TileTopCalibAlg.h
// PACKAGE:  TileCalibAlgs
//
// AUTHOR:   Alexander Solodkov (Sanya.Solodkov@cern.ch)
// 
// May 2008
// ********************************************************************

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "TileCalibAlgs/ITileCalibTool.h"

class TFile;

class TileTopCalibAlg : public AthAlgorithm
{
 public:

  TileTopCalibAlg(const std::string& name,ISvcLocator* pSvcLocator);
  virtual ~TileTopCalibAlg();

  StatusCode initialize();
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();

 private:

  /**
   * Run number
   */
  Gaudi::Property<int> m_runNumber{this, "RunNumber", 0};

  /**
   * Run type
   */
  Gaudi::Property<int> m_runType{this, "RunType", 0};

  /**
   * Name of output file with ntuples
   */
  Gaudi::Property<std::string> m_fileName{this, "FileName", "TileCalibNtuple.root"};

  /**
   * Vector of algtools to call
   */
  ToolHandleArray<ITileCalibTool> m_tileCalibToolList{this,
    "TileCalibTools", {}, "Tile calib tools"};
 
  /**
   * Pointer to the ROOT file
   */
  TFile *m_rootFile{nullptr};

};

#endif // TILETOPCALIBALG_H
