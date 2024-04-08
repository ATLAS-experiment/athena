/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONG4R4_TgcSENSITIVEDETECTORTOOL_H
#define MUONG4R4_TgcSENSITIVEDETECTORTOOL_H

#include <GeoPrimitives/GeoPrimitives.h>

#include <G4AtlasTools/SensitiveDetectorBase.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <xAODMuonSimHit/MuonSimHitContainer.h>

namespace MuonG4R4 {

class TgcSensitiveDetectorTool : public SensitiveDetectorBase {

public:
    TgcSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface *parent);
    ~TgcSensitiveDetectorTool()=default;

    StatusCode initialize() override final;
protected:
    G4VSensitiveDetector* makeSD() const override final;
private:
    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
    /** @brief: Key to the alignment / transform store per event. */
    Gaudi::Property<std::string> m_alignStoreKey{this, "AlignStoreKey", ""};

   
};
}

#endif
