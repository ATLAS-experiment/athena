/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CSCSENSITIVEDETECTORTOOL_H
#define CSCSENSITIVEDETECTORTOOL_H

#include "G4AtlasTools/SensitiveDetectorBase.h"

class CSCSensitiveDetectorTool : public SensitiveDetectorBase {

public:
    CSCSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface *parent);
    virtual StatusCode SetupEvent(HitCollectionMap& hitCollections) override;
    virtual StatusCode Gather(HitCollectionMap& hitCollections) override;
protected:
    G4VSensitiveDetector* makeSD() const override final;
};

#endif
