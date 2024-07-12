/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/MdtTubeLayer.h>
#include <GeoModelKernel/GeoTube.h>
#include <GeoModelKernel/GeoAccessVolumeAction.h>

#include <GeoModelHelpers/TransformSorter.h>
#include <GeoModelHelpers/GeoPhysVolSorter.h>
#include <GeoModelHelpers/getChildNodesWithTrf.h>
#include <GeoModelUtilities/GeoVisitVolumes.h>


namespace MuonGMR4{
   bool operator<(const MdtTubeLayer::CutTubes& a, const unsigned int tube){
        return a.lastTube < tube;
    }
    bool operator<(const unsigned int tube, const MdtTubeLayer::CutTubes& a) {
        return tube < a.firstTube;
    }
    bool operator<(const MdtTubeLayer::TubePositioner&a,const unsigned int tube) {
        return a.lastTube < tube;
    }
    bool operator<(const unsigned int tube, const MdtTubeLayer::TubePositioner& a) {
        return tube < a.firstTube;
    }
 
    bool MdtTubeLayerSorter::operator()(const MdtTubeLayer& a, const MdtTubeLayer& b) const{
        // Don't bother calling the underlying comparisons
        // if the objects are the same.  This saves a lot of time.
        if (a.m_layTrf != b.m_layTrf) {
          static const GeoTrf::TransformSorter trfSort{};
          const int trfCmp = trfSort.compare(a.layerTransform(), b.layerTransform());
          if (trfCmp) return trfCmp < 0;
        }
        if (a.m_layerNode != b.m_layerNode) {
          static const GeoPhysVolSorter physSort{};
          return physSort(a.m_layerNode, b.m_layerNode);
        }
        return false;
    }
    bool MdtTubeLayerSorter::operator()(const MdtTubeLayerPtr&a, const MdtTubeLayerPtr& b) const{
        return (*this)(*a, *b); 
    }
    

    MdtTubeLayer::MdtTubeLayer(const PVConstLink layer,
                               const GeoIntrusivePtr<const GeoTransform> toLayTrf,
                               const CutTubeSet& cutTubes):
        m_layerNode{std::move(layer)},
        m_layTrf{std::move(toLayTrf)},
        m_cutTubes{cutTubes} {

        const std::vector<GeoChildNodeWithTrf> children = getChildrenWithRef(m_layerNode, true);
        unsigned int firstTube{0};
        bool firstTubeSet{false};
        for (const GeoChildNodeWithTrf& child : children) {
            TubePositioner positioner{};
            positioner.firstTube = firstTube;
            positioner.lastTube = firstTube + child.nCopies -1;
            positioner.firstTubePos = child.transform.translation();
            positioner.tubeVol = child.volume;
            if (!firstTubeSet && child.nCopies>1) {
              m_tubePitch = child.inductionRule.translation();
              firstTubeSet = true;
            }
            firstTube = positioner.lastTube + 1;
            m_tubePos.insert(std::move(positioner));
        }
    }

    const Amg::Transform3D& MdtTubeLayer::layerTransform() const {
        return m_layTrf->getDefTransform();
    }
    PVConstLink MdtTubeLayer::getTubeNode(unsigned int tube) const {
        TubePositionerSet::const_iterator tube_itr = m_tubePos.find(tube);
         if (tube_itr == m_tubePos.end()) {
           THROW_EXCEPTION(m_layerNode->getLogVol()->getName()<<" has only "<<nTubes()<<" tubes. But "
                          <<tube<<" is requested. Please check.");
        }
        return tube_itr->tubeVol;
    }
    const Amg::Transform3D MdtTubeLayer::tubeTransform(const unsigned int tube) const {    
        TubePositionerSet::const_iterator tube_itr = m_tubePos.find(tube);
        if (tube_itr == m_tubePos.end()) {
            THROW_EXCEPTION(m_layerNode->getLogVol()->getName()<<" has only "<<nTubes()<<" tubes. But "
                            <<tube<<" is requested. Please check.");
        }   
        return layerTransform() * Amg::getTranslate3D(tube_itr->firstTubePos)*
                                  Amg::getTranslate3D(1.*(tube-tube_itr->firstTube)* m_tubePitch);
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
    double MdtTubeLayer::uncutHalfLength(unsigned int tube) const{
        CutTubeSet::const_iterator itr = m_cutTubes.find(tube);
        if (itr!= m_cutTubes.end()) {
          return itr->unCutHalfLength;
        }
        return tubeHalfLength(tube);
    }
}