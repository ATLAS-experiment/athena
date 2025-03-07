/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STRIPTDSOFFLINETOOL_H
#define STRIPTDSOFFLINETOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGauss.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonDigitContainer/sTgcDigit.h"
#include "MuonDigitContainer/sTgcDigitContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "MuonSimData/MuonSimData.h"
#include "MuonSimData/MuonSimDataCollection.h"

#include "TrigT1NSWSimTools/IStripTdsTool.h"
#include "TrigT1NSWSimTools/PadTrigger.h"
#include "TrigT1NSWSimTools/TriggerTypes.h"
#include "TrigT1NSWSimTools/StripOfflineData.h"
#include "TrigT1NSWSimTools/PadOfflineData.h"

#include "CxxUtils/checker_macros.h"
#include <functional>
#include <algorithm>
#include <map>
#include <utility>

namespace MuonGM {
  class MuonDetectorManager;
}

namespace NSWL1 {

  /**
   *
   *   @short interface for the StripTDS tools
   *
   * This class implements the Strip TDS offline simulation. It loops over the input digits,
   * determines the BC tag and applies the additional processing of the VMM chip which is
   * not yet implemented in the digitization. The loop is executed over the full digit pool
   * once upon the first data request for an event and the STRIP data are internally cached
   * and collected per trigger sectors. The run ID and event ID are cached; the processing
   * status is also cached to be propagated via a StatusCode at each data request.
   *
   * Supported processing:
   *  Currently None;
   *
   * It returns a vector of StripData to input the StripTrigger simulation.
   *
   *  @author Jacob Searcy <jsearcy@cern.ch>
   *
   * ----------------------------------------------------------------------------------------
   * 2022 Update: the internal cache has been removed for the code to deal with parallel
   * processing (athenaMT) in Release 22. It has been replaced by an event-by-event cache,
   * passed by reference throughout the workflow.
   *
   *  @modified by Francesco Giuseppe Gravili <francesco.giuseppe.gravili@cern.ch>
   *
   */

  class StripHits;

  class StripTdsOfflineTool: virtual public IStripTdsTool,
                                   public AthAlgTool {

  public:
    StripTdsOfflineTool(const std::string& type,
                      const std::string& name,
                      const IInterface* parent);

    virtual ~StripTdsOfflineTool()=default;

    virtual StatusCode initialize() override;

    virtual StatusCode gather_strip_data(std::vector<std::unique_ptr<StripData>>& strips,const std::vector<std::unique_ptr<PadTrigger>>& padTriggers) const override;


  private:
    // methods implementing the internal data processing
    StatusCode fill_strip_cache(const std::vector<std::unique_ptr<PadTrigger>>& padTriggers, std::vector<std::unique_ptr<StripData>> &strip_cache) const;

    bool readStrip( StripData* ,const std::vector<std::unique_ptr<PadTrigger>>&) const;

    // needed Servives, Tools and Helpers
    const MuonGM::MuonDetectorManager* m_detManager;        //!< MuonDetectorManager
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    // activate MC-only processes
    Gaudi::Property<bool>         m_isMC            {this, "IsMC",                  true,       "This is MC"};

    SG::ReadHandleKey<sTgcDigitContainer> m_sTgcDigitContainer = {this,"sTGC_DigitContainerName","sTGC_DIGITS","the name of the sTGC digit container"};
    SG::ReadHandleKey<MuonSimDataCollection> m_sTgcSdoContainer = {this,"sTGC_SdoContainerName","sTGC_SDO","the name of the sTGC SDO container"};
  };  // end of StripTdsOfflineTool class
} // namespace NSWL1
#endif
