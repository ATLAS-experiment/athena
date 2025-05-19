/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_BUILDNSWREADOUTGEOMETRY_H
#define MUON_BUILDNSWREADOUTGEOMETRY_H

#include <vector>

namespace MuonGM {
    class MuonDetectorManager;
}


class BuildNSWReadoutGeometry {
public:
    BuildNSWReadoutGeometry();

    static bool BuildReadoutGeometry(MuonGM::MuonDetectorManager* mgr) ;

private:
};

#endif
