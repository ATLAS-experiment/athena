/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONG4R4_TGCSensitiveDetector_H
#define MUONG4R4_TGCSensitiveDetector_H

#include "G4VSensitiveDetector.hh"

#include <StoreGate/WriteHandle.h>
#include <AthenaBaseComps/AthMessaging.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/TgcReadoutElement.h>
#include <xAODMuonSimHit/MuonSimHitContainer.h>

namespace MuonG4R4 {

class TgcSensitiveDetector : public G4VSensitiveDetector, public AthMessaging {


public:
    /** @brief Constructor
     *  @param name: Name of the Sensitive detctor / AthMessaging module
     *  @param output_key: Key under which the sim hits are written into store gate
     *  @param trf_storeKey: Location of the DetctorAlignmentStore holding the transformations per event
     *  @param detMgr: Pointer to the run-4 detector manager
    */
    TgcSensitiveDetector(const std::string& name, 
                         const std::string& output_key,
                         const std::string& trf_storKey,
                         const MuonGMR4::MuonDetectorManager* detMgr);
    
    ~TgcSensitiveDetector()=default;
    
    /** member functions */
    void   Initialize(G4HCofThisEvent* HCE) override final;
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override final;

    
private:
    /// Retrieves the matching readout element to a G4 hit
    const MuonGMR4::TgcReadoutElement* getReadoutElement(const G4TouchableHistory* touchHist) const;
    
    Identifier getIdentifier(const ActsGeometryContext& gctx,
                             const MuonGMR4::TgcReadoutElement* readOutEle, 
                             const Amg::Vector3D& hitAtGapPlane, bool phiGap) const;
    /* 
     *  For the moment use write handles because the sensitive detectors are 
     *  managed by a service which must not have a data dependency
    */
    SG::WriteHandle<xAOD::MuonSimHitContainer> m_writeHandle;
    /*
     *  ReadHandleKey to the DetectorAlignmentStore caching
     *  the relevant transformations needed in this event
    */
    SG::ReadHandleKey<ActsTrk::DetectorAlignStore> m_trfCacheKey;

    /// Pointer to the acts Geometry context
    const ActsGeometryContext m_gctx{};
    /// Pointer to the underlying detector manager
    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
};
}

#endif