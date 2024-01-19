/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/MdtTubeLayer.h>
#include <GeoModelKernel/GeoTube.h>
#include <GeoModelKernel/GeoAccessVolumeAction.h>

namespace MuonGMR4{

MdtTubeLayer::MdtTubeLayer(const PVConstLink layer,
                           const Amg::Transform3D& layTrf):
    m_layerNode{std::move(layer)},
    m_layTrf{layTrf} {}

const Amg::Transform3D& MdtTubeLayer::layerTransform() const {
    return m_layTrf;
}
PVConstLink MdtTubeLayer::getTubeNode(unsigned int tube) const {
     if (tube >= nTubes()) {
        std::stringstream except{};
        except<<__FILE__<<":"<<__LINE__<<" "<<m_layerNode->getLogVol()->getName()<<" has only "<<nTubes()<<" tubes. But "<<tube<<" is requested. Please check.";
        throw std::out_of_range(except.str());
    }
    return m_layerNode->getChildVol(tube);
}
const Amg::Transform3D MdtTubeLayer::tubeTransform(const unsigned int tube) const {    
    if (tube >= nTubes()) {
        std::stringstream except{};
        except<<__FILE__<<":"<<__LINE__<<" "<<m_layerNode->getLogVol()->getName()<<" has only "<<nTubes()<<" tubes. But "<<tube<<" is requested. Please check.";
        throw std::out_of_range(except.str()); 
    }
    GeoAccessVolumeAction volAcc{tube, nullptr};
    m_layerNode->exec(&volAcc);
    return layerTransform() * volAcc.getDefTransform();
}
const Amg::Vector3D MdtTubeLayer::tubePosInLayer(const unsigned int tube) const {
    return  layerTransform().inverse() * tubeTransform(tube).translation();
}
unsigned int MdtTubeLayer::nTubes() const {
    return m_layerNode->getNChildVols();
}
double MdtTubeLayer::tubeHalfLength(const unsigned int tube) const {
    const PVConstLink child = getTubeNode(tube);
    const GeoShape* shape = child->getLogVol()->getShape();
    const GeoTube* tubeShape = static_cast<const GeoTube*>(shape);
    return tubeShape->getZHalfLength();    
}   
}

