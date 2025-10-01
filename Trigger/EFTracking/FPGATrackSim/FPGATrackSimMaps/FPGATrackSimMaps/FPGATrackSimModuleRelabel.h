// Copyright (C) 2023-2025 CERN for the benefit of the ATLAS collaboration

// Module relabel object for remapping pixel endcap hits.
// This small header contains a very simple object, which implements
// the geokey -> ring index lookup for use in multiple places and provides
// a function for actually remapping pixel endcap hits.

#ifndef FPGATRACKSIMMODULERELABEL_H
#define FPGATRACKSIMMODULERELABEL_H
#include "AthenaBaseComps/AthMessaging.h" //inheritance


#include <map>
#include <vector>
#include <string>

class FPGATrackSimHit;

// this doesn't necessarily have to live in a namespace, but it would
// if it were in FPGATrackSimMacros.h.
// New supported geometries should be added here.
namespace htt {
    const std::map<const std::string, const std::vector<uint>> ringIndices = {
        {"ATLAS-P2-ITK-22-02-00",  {0, 17, 47, 58, 66}},
        {"ATLAS-P2-ITK-23-00-01",  {0, 15, 44, 50, 61, 69, 77, 86}},
        {"ATLAS-P2-RUN4-03-00-00", {0, 15, 21, 44, 50, 61, 69, 77, 86}}
    };
}

class FPGATrackSimModuleRelabel : public AthMessaging{

public:

    // Constructor, maps geokey -> ringIndex.
    FPGATrackSimModuleRelabel(const std::string & geokey, bool remapModules);

    bool remap(FPGATrackSimHit& hit) const;

private:

    // Geometry version, set by constructor.
    std::string m_geoKey;

    // Should we remap modules in addition to layers.
    bool m_remapModules = false;

    // Vector of size (nrings): used to remap (ring, module) -> (layer).
    // Indexed by ring number.
    const std::vector<uint>* m_ringIndex = nullptr;

};

#endif
