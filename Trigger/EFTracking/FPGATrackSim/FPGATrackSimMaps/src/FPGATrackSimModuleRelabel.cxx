// Copyright (C) 2023-2024 CERN for the benefit of the ATLAS collaboration


#include "FPGATrackSimMaps/FPGATrackSimModuleRelabel.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"

using namespace std;
using namespace htt;

FPGATrackSimModuleRelabel::FPGATrackSimModuleRelabel(std::string geokey, bool remapModules) :
    AthMessaging("FPGATrackSimModuleRelabel"),
    m_geoKey(geokey),
    m_remapModules(remapModules)
{
    // Set up the ring index reference based on configured geokey.
    if (htt::ringIndices.count(m_geoKey) == 0) {
        m_ringIndex = nullptr;
        ATH_MSG_ERROR("Bad geometry version " << m_geoKey);
    } else {
        m_ringIndex = &(htt::ringIndices.at(m_geoKey));
        ATH_MSG_INFO("Allocating map for geometry " << m_geoKey << " diskIndex size=" << m_ringIndex->size());
    }
}

bool FPGATrackSimModuleRelabel::remap(FPGATrackSimHit& hit) const {

    if(hit.isRemapped()){
        return false;
    }
    // Quick sanity check, if we get here somehow without configuring a valid geokey.
    if (not m_ringIndex) {
        ATH_MSG_ERROR("No configured ring index relabel for geometry " << m_geoKey);
        return false;
    }

    // Re-assign layers in the pixel endcap to be each individual disk
    if (hit.isPixel() && !hit.isBarrel()) {
        if (hit.getPhysLayer() < m_ringIndex->size()) {
            unsigned newlayer = hit.getEtaModule() + (*m_ringIndex)[hit.getPhysLayer()];
            int newmodule = 100*newlayer + hit.getEtaIndex() / 10;

            hit.setPhysLayer(newlayer);
            if (m_remapModules) {
                hit.setEtaModule(newmodule);
            }
            else {
                hit.setEtaModule(hit.getEtaModule()); // allows us to save the old value
            }
        }
        else {
            ATH_MSG_ERROR("Error: requesting "<< hit.getPhysLayer() << " element in m_ringIndex which is of size " << m_ringIndex->size());
            return false;
        }
    }
    else {  // allows us to save the old value
        hit.setEtaModule(hit.getEtaModule());
        hit.setPhysLayer(hit.getPhysLayer());
    }

    hit.setRemap();
    return true;
}

