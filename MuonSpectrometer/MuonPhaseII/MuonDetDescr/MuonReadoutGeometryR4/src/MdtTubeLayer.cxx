/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/MdtTubeLayer.h>
#include <GeoModelKernel/GeoTube.h>
#include <GeoModelKernel/GeoAccessVolumeAction.h>

#include <GeoModelHelpers/TransformSorter.h>
#include <GeoModelHelpers/GeoPhysVolSorter.h>

namespace MuonGMR4{

 
    bool MdtTubeLayerSorter::operator()(const MdtTubeLayer& a, const MdtTubeLayer& b) const{
        static const GeoTrf::TransformSorter trfSort{};
        const int trfCmp = trfSort.compare(a.layerTransform(), b.layerTransform());
        if (trfCmp) return trfCmp < 0;
        static const GeoPhysVolSorter physSort{};
        return physSort(a.m_layerNode, b.m_layerNode);
    }
    bool MdtTubeLayerSorter::operator()(const MdtTubeLayerPtr&a, const MdtTubeLayerPtr& b) const{
        return (*this)(*a, *b); 
    }
    

MdtTubeLayer::MdtTubeLayer(const PVConstLink layer,
                           const GeoIntrusivePtr<const GeoTransform> toLayTrf):
    m_layerNode{std::move(layer)},
    m_layTrf{std::move(toLayTrf)} {}

const Amg::Transform3D& MdtTubeLayer::layerTransform() const {
    return m_layTrf->getDefTransform();
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

