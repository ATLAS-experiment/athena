/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_STRIPLAYER_H
#define MUONREADOUTGEOMETRYR4_STRIPLAYER_H

#include <MuonReadoutGeometryR4/StripDesign.h>
namespace MuonGMR4{
    class StripLayer {
        public:
          StripLayer(const Amg::Transform3D& layerTransform,
                     StripDesignPtr design,
                     const IdentifierHash hash);
      
          /// Returns the transformation to go from the strip layer center 
          /// to the origin of the Strip chamber
          const Amg::Transform3D& toOrigin() const;           
          /// Returns the underlying strip design
          const StripDesign& design() const;
          /// Returns the hash of the strip layer
          const IdentifierHash hash() const;

          /// Returns the position of the strip expressed in the chamber frame
          Amg::Vector3D stripPosition(unsigned int stripNum) const;
          /// Returns the position of the strip expressed in the local frame
          Amg::Vector3D localStripPos(unsigned int stripum) const;
          /// Returns the position of the left strip edge  (positive local y) expressed in the local frame
          Amg::Vector3D localStripLeftEdge(unsigned int stripNum) const;
          /// Returns the position of the right strip edge (negative local y) exoressed in the local frame
          Amg::Vector3D localStripRightEdge(unsigned int stripNum) const;
      
        private:
           Amg::Transform3D m_transform{Amg::Transform3D::Identity()};
           StripDesignPtr m_design{};
           IdentifierHash m_hash{};
    };
    std::ostream& operator<<(std::ostream& ostr, const StripLayer& lay);
}

#include <MuonReadoutGeometryR4/StripLayer.icc>
#endif