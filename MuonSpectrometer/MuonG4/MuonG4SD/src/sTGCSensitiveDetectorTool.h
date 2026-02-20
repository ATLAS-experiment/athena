/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef sTGCSensitiveDetectorTool_H
#define sTGCSensitiveDetectorTool_H

#include "G4AtlasTools/SensitiveDetectorBase.h"

class sTGCSensitiveDetectorTool : public SensitiveDetectorBase {

public:
    /** construction/destruction */
    sTGCSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface *parent);
    virtual StatusCode SetupEvent(HitCollectionMap& hitCollections) override;
    virtual StatusCode Gather(HitCollectionMap& hitCollections) override;
protected:
    G4VSensitiveDetector* makeSD() const override final;
    Gaudi::Property<bool> m_onSqLite{this, "onSqLite", false, "Runs on Sqlite -> adapt base depth of the detector to 1"};
};

#endif
