/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "StripInformationHelper.h"

namespace ActsTrk {

    StripInformationHelper::StripInformationHelper (const unsigned int& idHash,
						    const Amg::Vector3D& stripStart,
                                                    const Amg::Vector3D& stripEnd,
                                                    const Amg::Vector3D& beamSpotVertex,
                                                    const float& locx,
                                                    const size_t& clusterIndex,
                                                    const size_t& stripIndex)
    {
        this->set(idHash, stripStart, stripEnd, beamSpotVertex, locx, clusterIndex, stripIndex);
    }

    void StripInformationHelper::set(const unsigned int& idHash,
				     const Amg::Vector3D& stripStart,
                                     const Amg::Vector3D& stripEnd,
                                     const Amg::Vector3D& beamSpotVertex,
                                     const float& locx,
                                     const size_t& clusterIndex,
                                     const size_t& stripIndex)
    {
        m_cache.mid       = 0.5*(stripStart+stripEnd);
        m_cache.btmToTop  = stripStart-stripEnd;
        m_cache.vtxToMid2 = 2.*(m_cache.mid-beamSpotVertex);
        m_cache.normal    = m_cache.btmToTop.cross(m_cache.vtxToMid2);
        m_cache.invLength = 1./m_cache.btmToTop.mag();
        m_locX         = locx;
        m_clusterIndex = clusterIndex;
        m_stripIndex   = stripIndex;
	m_idHash       = idHash;
    }

    Amg::Vector3D StripInformationHelper::position(const double& shift) const
    {
        return (m_cache.mid+(0.5*shift)*m_cache.btmToTop);
    }
}
