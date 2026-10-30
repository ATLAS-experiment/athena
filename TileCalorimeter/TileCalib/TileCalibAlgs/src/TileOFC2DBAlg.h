/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TileCalibAlgs_TileOFC2DBAlg_h
#define TileCalibAlgs_TileOFC2DBAlg_h

/***************************************************************************
 TileOFC2DBAlg
 -----------------------------------------
 
 This algorithm writes OFC for specified phase to sqlite file to
 be stored afterwards in COOL DB.
 It uses TileCondToolOfc to calculate OFCs. 

 2008-05-13    Andrei.Artamonov@cern.ch
 ***************************************************************************/

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

// Athena includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadHandleKey.h"
#include "AthenaKernel/IOVTime.h"

//=== IOVDbTest includes
#include "RegistrationServices/IIOVRegistrationSvc.h"

// Tile includes
#include "TileConditions/ITileCondToolOfc.h"
#include "TileConditions/TileCondToolTiming.h"

class TileOFC2DBAlg: public AthAlgorithm {

  public:
    TileOFC2DBAlg(const std::string& name, ISvcLocator* pSvcLocator);

    ~TileOFC2DBAlg();

    StatusCode initialize();
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();

  private:
    StatusCode registerCondObjects();
    StatusCode printCondObjects();

    //===  storegate
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this,
      "EventInfo", "EventInfo", "EventInfo key"};

    //===  properties
    Gaudi::Property<unsigned int> m_runIOVSince{this, "RunIOVSince", IOVTime::MINRUN};
    Gaudi::Property<unsigned int> m_runIOVUntil{this, "RunIOVUntil", IOVTime::MAXRUN};
    Gaudi::Property<unsigned int> m_lbnIOVSince{this, "LbnIOVSince", IOVTime::MINEVENT};
    Gaudi::Property<unsigned int> m_lbnIOVUntil{this, "LbnIOVUntil", IOVTime::MAXEVENT};
    Gaudi::Property<unsigned int> m_maxChan{this, "MaxChan", 1, "Number of channels for which OFCs will be created with fixed phases"};
    Gaudi::Property<bool> m_of2{this, "OF2", true, "true => OF2, false => OF1"};
    Gaudi::Property<bool> m_fixedPhases{this, "FixedPhases", true, "calculate OFCs for fixed phases"};
    Gaudi::Property<std::string> m_runType{this, "RunType", "PHY", "PHY or LAS"};

    //=== non-property members
    ServiceHandle<IIOVRegistrationSvc> m_regSvc;
    ToolHandle<TileCondToolTiming> m_tileToolTiming{this,
      "TileCondToolTiming", "TileCondToolTiming", "Tile timing tool"};
    ToolHandle<ITileCondToolOfc> m_tileCondToolOfc{this,
      "TileCondToolOfc", "TileCondToolOfc", "Tile OFC tool"};

    //=== store all folders to be registered
    std::set<std::string> m_folders;

    Gaudi::Property<int> m_nFixedPhases{this, "FixedPhasesNumber", 100};
    Gaudi::Property<double> m_phaseStep{this, "PhaseStep", 0.5};
    Gaudi::Property<std::vector<std::string>> m_modules{this, "Modules", {"AUX01"}, "Modules for which OFC should be stored in DB"};
    std::vector<unsigned int> m_drawerIdxs;
    Gaudi::Property<bool> m_creatAllModules{this, "CreateAllModules", true, "All missing modules is written to DB with zero size (empty)"};
};

#endif //TileCalibAlgs_TileOFC2DBAlg_h
