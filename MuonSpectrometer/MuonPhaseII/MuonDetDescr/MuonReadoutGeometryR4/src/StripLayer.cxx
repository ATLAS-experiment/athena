/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/StripLayer.h>
#include <GeoModelHelpers/TransformSorter.h>
namespace MuonGMR4{

    std::ostream& operator<<(std::ostream& ostr, const StripLayer& lay) {
        ostr<<"Strip layer transform: "<<Amg::toString(lay.toOrigin())<<", ";
        ostr<<lay.design()<<", ";
        ostr<<"Hash: "<<static_cast<unsigned int>(lay.hash());        
        return ostr;
    }
    StripLayer::StripLayer(const Amg::Transform3D& layerTransform,
                           StripDesignPtr design,
                           const IdentifierHash hash):
         m_transform{layerTransform},
         m_design{std::move(design)},
         m_hash{hash} {        
    }
    bool StripLayer::operator<(const StripLayer& other) const{
        if (hash() != other.hash()) {
            return hash() < other.hash();
        }
        if (other.m_design != m_design) {
           return (*m_design) < (*other.m_design);
        }
        const GeoTrf::TransformSorter sorter{};
        return sorter(m_transform, other.m_transform);
    }
}