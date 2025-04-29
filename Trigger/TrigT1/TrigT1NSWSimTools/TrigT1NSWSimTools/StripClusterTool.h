/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STRIPCLUSTERTOOL_H
#define STRIPCLUSTERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ServiceHandle.h"

#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "MuonDigitContainer/sTgcDigitContainer.h"
#include "MuonDigitContainer/sTgcDigit.h"
#include "MuonSimData/MuonSimDataCollection.h"
#include "MuonSimData/MuonSimData.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "TrigT1NSWSimTools/StripOfflineData.h"
#include "TrigT1NSWSimTools/IStripClusterTool.h"
#include "TrigT1NSWSimTools/PadTrigger.h"
#include "TrigT1NSWSimTools/StripTdsOfflineTool.h"
#include "TrigT1NSWSimTools/TriggerTypes.h"
#include "StripClusterOfflineData.h"

#include <functional>
#include <algorithm>
#include <map>
#include <utility>

namespace MuonGM {
  class MuonDetectorManager;
}


// namespace for the NSW LVL1 related classes
namespace NSWL1 {

  /**
   *
   *   @short interface for the StripTDS tools
   *
   * This class implements the Strip Clustering offline simulation. It loops over the hits,
   * readout from the StripTDSOffLineTool
   *
   *  @author Jacob Searcy <jsearcy@cern.ch>
   *
   *
   */

  class StripClusterTool: virtual public IStripClusterTool,
                                  public AthAlgTool {

  public:
    StripClusterTool(const std::string& type,
                     const std::string& name,
                     const IInterface* parent);
    virtual ~StripClusterTool()=default;
    virtual StatusCode initialize() override;
    virtual StatusCode cluster_strip_data(const EventContext& ctx, std::vector<std::unique_ptr<StripData>>& strips,std::vector<std::unique_ptr<StripClusterData>>& clusters) const override;

  private:
    StatusCode fill_strip_validation_id(const EventContext& ctx,
					std::vector<std::unique_ptr<StripClusterData>>& clusters,
                                        std::vector< std::shared_ptr<std::vector<std::unique_ptr<StripData> >>  > &cluster_cache) const;

    // needed Servives, Tools and Helpers
    ServiceHandle< IIncidentSvc > m_incidentSvc{this, "IncidentSvc", "IncidentSvc"};       //!< Athena/Gaudi incident Service
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detManagerKey{this, "MuonManagerKey", "MuonDetectorManager"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    Gaudi::Property<bool>         m_isMC            {this, "IsMC",                  true,       "This is MC"};
    SG::ReadHandleKey<MuonSimDataCollection> m_sTgcSdoContainerKey{this,"sTGC_SdoContainerName", "sTGC_SDO", "the name of the sTGC SDO container"};
  };  // end of StripClusterTool class
} // namespace NSWL1
#endif
