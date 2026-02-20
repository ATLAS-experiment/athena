/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDTSENSITIVEDETECTORTOOL_H
#define MDTSENSITIVEDETECTORTOOL_H

#include "G4AtlasTools/SensitiveDetectorBase.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

class MDTSensitiveDetectorTool : public SensitiveDetectorBase {

public:
    MDTSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface *parent);
    virtual StatusCode SetupEvent(HitCollectionMap& hitCollections) override;
    virtual StatusCode Gather(HitCollectionMap& hitCollections) override;
protected:
    G4VSensitiveDetector* makeSD() const override final;
private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
};

#endif
