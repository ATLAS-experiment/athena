/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_MUONCHAMBERTOOLTEST_H
#define MUONGEOMODELTESTR4_MUONCHAMBERTOOLTEST_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <MuonStationGeoHelpers/IMuonStationLayerSurfaceTool.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <ActsGeometryInterfaces/IDetectorVolumeSvc.h>
#include <StoreGate/ReadCondHandleKey.h>

namespace MuonGMR4 { 

class ChambBoundaryNote;

class MuonChamberToolTest: public AthReentrantAlgorithm {
    public:
        MuonChamberToolTest(const std::string& name, ISvcLocator* pSvcLocator);

        ~MuonChamberToolTest() = default;

        StatusCode execute(const EventContext& ctx) const override;        
        StatusCode initialize() override;        

        bool isReEntrant() const override final {return false;}   
    
    private:
        /// Test that all Mdts are inside the chamber volume
        StatusCode testMdt(const ActsGeometryContext& gctx,
                           const MdtReadoutElement& readOutEle,
                           ChambBoundaryNote& chamb) const;
        
        StatusCode testRpc(const ActsGeometryContext& gctx,
                           const RpcReadoutElement& readoutEle,
                           ChambBoundaryNote& chamber) const;

        StatusCode testTgc(const ActsGeometryContext& gctx,
                           const TgcReadoutElement& readoutEle,
                           ChambBoundaryNote& chamber) const;

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        SG::ReadCondHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

        ServiceHandle<ActsTrk::IDetectorVolumeSvc> m_detVolSvc{this,"DetectorVolumeSvc", "DetectorVolumeSvc"};
        
        PublicToolHandle<MuonGMR4::IMuonStationLayerSurfaceTool> m_chambTool{this, "LayerGeoTool", "" };

        const MuonDetectorManager* m_detMgr{nullptr};

};
}
#endif