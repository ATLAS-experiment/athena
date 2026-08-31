/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_STRIPINFORMATIONHELPER_H
#define ACTSTRK_DATAPREPARATION_STRIPINFORMATIONHELPER_H

#include "GeoPrimitives/GeoPrimitives.h"

#include "Acts/SpacePointFormation/StripSpacePointBuilder.hpp"

namespace ActsTrk {

  /// @brief Total number of neightbours and indices
  enum NeighbourIndices {ThisOne, Opposite, PhiMinus, PhiPlus, EtaMinus, EtaPlus, nNeighbours};

  class StripInformationHelper {

    /// @class StripInformationHelper
    /// This class contains the information on the strips used for
    /// space point formation.
    /// The quantities stored are used for mathematical evaluation of the
    /// space point location.

  public:
    /// @name Constructors with and without parameters
    //@{
    StripInformationHelper() = default;
    StripInformationHelper(const unsigned int& idHash,
			   const Amg::Vector3D& stripStart,
                           const Amg::Vector3D& stripEnd,
                           const Amg::Vector3D& beamSpotVertex,
                           const float& locx,
                           const size_t& clusterIndex,
                           const size_t& stripIndex);
    //@}

    /// @name Destructor, copy construcor, assignment operator
    //@{
    virtual ~StripInformationHelper() = default;
    StripInformationHelper(const StripInformationHelper&) = default;
    StripInformationHelper& operator = (const StripInformationHelper&) = default;
    //@}

    /// @name Public method to set strip properties
    //@{
    void set(const unsigned int& idHash,
	     const Amg::Vector3D& stripStart,
             const Amg::Vector3D& stripEnd,
             const Amg::Vector3D& beamSpotVertex,
             const float& locx,
             const size_t& clusterIndex,
             const size_t& stripIndex);
    //@}

    /// @name Public methods to return strip properties
    //@{
    const unsigned int& idHash() const {return m_idHash;}
    const size_t& clusterIndex() const {return m_clusterIndex;}
    const Amg::Vector3D& stripCenter () const {return m_cache.mid ;}
    const Amg::Vector3D& stripDirection () const {return m_cache.btmToTop ;}
    const Amg::Vector3D& trajDirection () const {return m_cache.vtxToMid2 ;}
    const Amg::Vector3D& normal() const {return m_cache.normal;}
    const double& oneOverStrip() const {return m_cache.invLength;}
    /// The same quantities in the layout Acts::StripSpacePointBuilder consumes
    const Acts::StripSpacePointBuilder::ConstrainedStripCache& constrainedCache() const {return m_cache;}
    const float& locX() const {return m_locX;}
    const size_t& stripIndex() const {return m_stripIndex;}
    Amg::Vector3D position(const double& shift) const;
    //@}

  private:

    /// @name Private members
    /// @param m_cache The strip geometry, in the layout Acts::StripSpacePointBuilder
    /// wants it, evaluated in the setting function. See the accessors above for the
    /// correspondence with the Athena names.
    Acts::StripSpacePointBuilder::ConstrainedStripCache m_cache {};
    /// @param m_locX Location X of cluster
    float         m_locX {0.};
    /// @param m_stripIndex index of the strip corresponding to location
    size_t        m_stripIndex {0};
    /// @param m_clusterIndex xAOD::StripCluster index in container
    size_t        m_clusterIndex {0};
    /// @param m_idHash xAOD::StripCluster idHash for connection to detector element
    unsigned int  m_idHash {0};
  };

} // end of name space

#endif  // ACTSTRKSPACEPOINTFORMATIONTOOL_STRIPINFORMATIONHELPER_H
