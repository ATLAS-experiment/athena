/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 TileExpertDump
 -----------------------------------------
 Test algorithm to dump out calibration constants from the TileInfoSvc


 2007-05-14      nils.gollub@cern.ch
 2008-06-06      lukas.pribyl@cern.ch
 ***************************************************************************/

#ifndef TILECALIBALGS_TILEEXPERTDUMP_H
#define TILECALIBALGS_TILEEXPERTDUMP_H

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

// Athena includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadHandleKey.h"

#include "TileIdentifier/TileEmscaleCalibOptions.h"
#include "TileConditions/TileExpertToolEmscale.h"

class TileHWID;
class TileInfo;

/**
 ** Algorithn to dump out calibration constants from the TileInfoSvc
 **/

class TileExpertDump: public AthAlgorithm {
  public:
    TileExpertDump(const std::string& name, ISvcLocator* pSvcLocator);
    ~TileExpertDump();

    StatusCode initialize();
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();

    StatusCode setEmExpertOptions();

  private:

    TileEmscaleCalibOptions m_emExpertOptions{0};
    Gaudi::Property<bool> m_OflCisFitLin{this, "useOflCisFitLin", false, "Use calibration of OflCisFitLin"};
    Gaudi::Property<bool> m_OflCisFitNln{this, "useOflCisFitNln", false, "Use calibration of OflCisFitNln"};
    Gaudi::Property<bool> m_OflLasLin{this, "useOflLasLin", false, "Use calibration of OflLasLin"};
    Gaudi::Property<bool> m_OflLasNln{this, "useOflLasNln", false, "Use calibration of OflLasNln"};
    Gaudi::Property<bool> m_OflLasPart{this, "useOflLasPart", false, "Use calibration of OflLasPart"};
    Gaudi::Property<bool> m_OflCes{this, "useOflCes", false, "Use calibration of OflCes"};
    Gaudi::Property<bool> m_OflEms{this, "useOflEms", false, "Use calibration of OflEms"};
    Gaudi::Property<bool> m_OflLasLinPisa{this, "useOflLasLinPisa", false, "Use calibration of OflLasLinPisa"};
    Gaudi::Property<bool> m_OflLasNlnPisa{this, "useOflLasNlnPisa", false, "Use calibration of OflLasNlnPisa"};
    Gaudi::Property<bool> m_OflMinbias{this, "useOflMinbias", false, "Use calibration of OflMinbias"};
    Gaudi::Property<bool> m_OnlCisLin{this, "useOnlCisLin", false, "Use calibration of OnlCisLin"};
    Gaudi::Property<bool> m_OnlLasLin{this, "useOnlLasLin", false, "Use calibration of OnlLasLin"};
    Gaudi::Property<bool> m_OnlCes{this, "useOnlCes", false, "Use calibration of OnlCes"};
    Gaudi::Property<bool> m_OnlEms{this, "useOnlEms", false, "Use calibration of OnlEms"};
    void printExpertEmscale();
    void printExpertEmscaleOnl();
    Gaudi::Property<bool> m_printExpertEmscale{this, "PrintExpertEmscale", false, "Switch on expert calibration chain printout"};
    Gaudi::Property<bool> m_printExpertEmscaleOnl{this, "PrintExpertEmscaleOnl", false, "Switch on expert onl calibration chain printout"};

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this,
      "EventInfo", "EventInfo", "EventInfo key"};

    //=== non-property members
    ToolHandle<TileExpertToolEmscale> m_tileExpertToolEmscale{this,
      "TileExpertToolEmscale","TileExpertToolEmscale", "Tile expert tool em scale"};

    const TileHWID* m_tileHWID{nullptr};

    //=== TileInfo
    Gaudi::Property<std::string> m_infoName{this, "TileInfoName", "TileInfo"};
    const TileInfo* m_tileInfo{nullptr};
    int m_i_ADCmax = 0;
};

#endif 
