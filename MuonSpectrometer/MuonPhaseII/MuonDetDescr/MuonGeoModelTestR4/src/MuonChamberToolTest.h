/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_MUONCHAMBERTOOLTEST_H
#define MUONGEOMODELTESTR4_MUONCHAMBERTOOLTEST_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/MuonChamber.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <MuonReadoutGeometryR4/TgcReadoutElement.h>

#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <ActsGeometryInterfaces/IDetectorVolumeSvc.h>
#include <StoreGate/ReadCondHandleKey.h>

namespace MuonGMR4 { 


class MuonChamberToolTest: public AthReentrantAlgorithm {
    public:
        MuonChamberToolTest(const std::string& name, ISvcLocator* pSvcLocator);

        ~MuonChamberToolTest() = default;

        StatusCode execute(const EventContext& ctx) const override;        
        StatusCode initialize() override;        

        bool isReEntrant() const override final {return false;}   
    
    private:
        StatusCode pointInside(const MuonChamber& chamb,
                               const Acts::Volume& boundVol,
                               const Amg::Vector3D& point,
                               const std::string& descr,
                               const Identifier& channelId) const;

        /// Test that all Mdts are inside the chamber volume
        StatusCode testMdt(const ActsGeometryContext& gctx,
                           const MdtReadoutElement& readOutEle,
                           const MuonChamber& chamb,
                           const Acts::Volume& boundVol) const;
        
        StatusCode testRpc(const ActsGeometryContext& gctx,
                           const RpcReadoutElement& readoutEle,
                           const MuonChamber& chamber,
                           const Acts::Volume& boundVol) const;

        StatusCode testTgc(const ActsGeometryContext& gctx,
                           const TgcReadoutElement& readoutEle,
                           const MuonChamber& chamber,
                           const Acts::Volume& boundVol) const;

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

        ServiceHandle<ActsTrk::IDetectorVolumeSvc> m_detVolSvc{this,"DetectorVolumeSvc", "DetectorVolumeSvc"};
        
        const MuonDetectorManager* m_detMgr{nullptr};

};
}
#endif