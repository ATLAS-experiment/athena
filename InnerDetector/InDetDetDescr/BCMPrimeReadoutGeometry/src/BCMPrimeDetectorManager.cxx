/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BCMPrimeReadoutGeometry/BCMPrimeDetectorManager.h"


namespace InDetDD {

    BCMPrimeDetectorManager::BCMPrimeDetectorManager(const std::string & name)
    {
        setName(name);
    }

    unsigned int BCMPrimeDetectorManager::getNumTreeTops() const {
        return m_volume.size();
    }

    PVConstLink BCMPrimeDetectorManager::getTreeTop(unsigned int i) const {
        return m_volume[i];
    }

    void BCMPrimeDetectorManager::addTreeTop(PVConstLink vol) {
        m_volume.push_back(vol);
    }

    void BCMPrimeDetectorManager::addAlignableTransform(int /*id*/, 
                                                        GeoAlignableTransform * /*transform*/,
                                                        const GeoVPhysVol * /*child*/)
    {
        // Here alignment transforms will be added
    }

    StatusCode BCMPrimeDetectorManager::align( IOVSVC_CALLBACK_ARGS_P( /*I*/, /*keys*/) ) const {
        // Here alignment transform deltas will be set
        return StatusCode::SUCCESS;
    }

} // namespace InDetDD
