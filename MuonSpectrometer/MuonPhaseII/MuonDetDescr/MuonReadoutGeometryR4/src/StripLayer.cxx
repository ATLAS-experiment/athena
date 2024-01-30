/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/StripLayer.h>
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
}