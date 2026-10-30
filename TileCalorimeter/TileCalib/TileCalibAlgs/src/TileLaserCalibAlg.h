/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILECALIBALGS_TILELASERCALIBALG_H
#define TILECALIBALGS_TILELASERCALIBALG_H

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

// Athena includes
#include "AthenaBaseComps/AthAlgorithm.h"

#include "TileCalibAlgs/ITileCalibTool.h"

#include <string>

class TFile;
class IROBDataProviderSvc;

class TileLaserCalibAlg: public AthAlgorithm {
  public:
    TileLaserCalibAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~TileLaserCalibAlg();

    // Functions
    StatusCode initialize();
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();

  private:

    typedef ToolHandleArray<ITileCalibTool> lastools_t;

    lastools_t m_lasTools{this,
      "Tools",{},"Tile laser calibration tools"};

    ServiceHandle<IROBDataProviderSvc> m_RobSvc;

    // jobOptions

    // Name of the output ROOT file
    // Trees names are defined in the respective tools (Default,...)

    TFile* m_outfile{nullptr};
    Gaudi::Property<std::string> m_fileName{this, "FileName", "LaserCalib.root"};

    //Run number and type
    // Type is defined as follows:
    //
    // 0  : Physics
    // 1  : Calibration
    // 2  : Cosmics
    // 15 : Test

    int m_runNo{0};
    int m_runType{0};

};

#endif // #ifndef TILECALIBALGS_TILELASERCALIBALG_H
