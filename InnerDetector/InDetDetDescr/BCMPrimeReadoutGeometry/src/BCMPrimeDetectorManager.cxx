/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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

    void BCMPrimeDetectorManager::addTreeTop(const PVConstLink& vol) {
        m_volume.push_back(vol);
    }

} // namespace InDetDD
