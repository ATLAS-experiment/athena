/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMETRYCNV_MUONREADOUTGEOMCNVALG_H
#define MUONGEOMETRYCNV_MUONREADOUTGEOMCNVALG_H

#include "TrkSurfaces/Surface.h" // Work around cppcheck false positive
#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <StoreGate/WriteCondHandleKey.h>
#include <StoreGate/ReadCondHandleKey.h>

#include <MuonReadoutGeometry/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonStationGeoHelpers/IMuonStationLayerSurfaceTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <ActsGeometryInterfaces/ActsGeometryContext.h>

/** The MuonReadoutGeomCnvAlg converts the Run4 Readout geometry build from the GeoModelXML into the legacy MuonReadoutGeometry.
 *  The algorithm is meant to serve as an adapter allowing to dynamically exchange individual components in the Muon processing chain
 *  by their Run4 / Acts equivalents
 * 
*/

class MuonReadoutGeomCnvAlg : public AthReentrantAlgorithm {
    public:
        MuonReadoutGeomCnvAlg(const std::string& name, ISvcLocator* pSvcLocator);
        ~MuonReadoutGeomCnvAlg() = default;

        StatusCode execute(const EventContext& ctx) const override;
        StatusCode initialize() override;
        bool isReEntrant() const override { return false; }
    
    private:
        StatusCode buildMdt(const ActsGeometryContext& gctx,
                            MuonGM::MuonDetectorManager* mgr,
                            PVLink world) const;

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        PublicToolHandle<MuonGMR4::IMuonStationLayerSurfaceTool> m_surfaceProvTool{this, "LayerGeoTool", ""};

        SG::WriteCondHandleKey<MuonGM::MuonDetectorManager> m_writeKey{this, "WriteKey", "MuonDetectorManager"};
        
        SG::ReadCondHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", 
                                                              "Alignment key"};
        
        const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};


 
};

#endif
