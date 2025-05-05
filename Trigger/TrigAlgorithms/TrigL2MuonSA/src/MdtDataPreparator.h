/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRIGL2MUONSA_MDTDATAPREPARATOR_H
#define  TRIGL2MUONSA_MDTDATAPREPARATOR_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"

#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"
#include "IRegionSelector/IRegSelTool.h"
#include "Identifier/IdentifierHash.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonCablingData/MuonMDT_CablingMap.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"

#include "MdtData.h"
#include "MdtRegionDefiner.h"

#include "RpcFitResult.h"
#include "TgcFitResult.h"
#include <unordered_set>

namespace MuonGM{
     class MdtReadoutElement;
     class MuonStation;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

namespace TrigL2MuonSA {

  class MdtDataPreparator: public AthAlgTool
  {
  public:

    static const InterfaceID& interfaceID();

    using AthAlgTool::AthAlgTool;

    virtual StatusCode initialize() override;

    StatusCode prepareData(const TrigRoiDescriptor* p_roids,
			    const TrigL2MuonSA::RpcFitResult& rpcFitResult,
			    TrigL2MuonSA::MuonRoad&           muonRoad,
			    TrigL2MuonSA::MdtRegion&          mdtRegion,
			    TrigL2MuonSA::MdtHits&            mdtHits) const;

    StatusCode prepareData(const TrigRoiDescriptor* p_roids,
			    const TrigL2MuonSA::TgcFitResult& tgcFitResult,
			    TrigL2MuonSA::MuonRoad&           muonRoad,
			    TrigL2MuonSA::MdtRegion&          mdtRegion,
			    TrigL2MuonSA::MdtHits&            mdtHits) const;

    void setRpcGeometry(bool use_rpc) {m_mdtRegionDefiner->setRpcGeometry(use_rpc);};
    void setRoIBasedDataAccess(bool use_RoIBasedDataAccess){m_use_RoIBasedDataAccess = use_RoIBasedDataAccess;};

  private:

    StatusCode getMdtHits(
        const TrigRoiDescriptor* p_roids,
			  TrigL2MuonSA::MuonRoad& muonRoad,
			  TrigL2MuonSA::MdtHits& mdtHits) const;

    StatusCode collectMdtHitsFromPrepData(const EventContext& ctx,
            const std::vector<IdentifierHash>& v_idHash,
					  TrigL2MuonSA::MdtHits& mdtHits,
					  const TrigL2MuonSA::MuonRoad& muonRoad) const;

    void initDeadChannels(const MuonGM::MdtReadoutElement* mydetEl);

    // Geometry Services
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    // Region Selector
    ToolHandle<IRegSelTool> m_regionSelector{this, "RegSel_MDT", "RegSelTool/RegSelTool_MDT"};

    // MdtRegionDefiner
    ToolHandle<MdtRegionDefiner> m_mdtRegionDefiner {
      this, "MdtRegionDefiner", "TrigL2MuonSA::MdtRegionDefiner"};

    // handles to data access
    SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_mdtPrepContainerKey{
	this, "MDTPrepDataContainer","MDT_DriftCircles", "Name of the MDTContainer to read in"};
    
    Gaudi::Property<bool> m_isPhase2{this, "isPhase2", false, "if the phase 2 geometry is setup"};
    Gaudi::Property<bool> m_use_RoIBasedDataAccess{this, "use_RoIBasedDataAccess", false};
    
    int  m_BMGid{0};
    std::unordered_set<Identifier> m_DeadChannels{};
  };

} // namespace TrigL2MuonSA

#endif  //
