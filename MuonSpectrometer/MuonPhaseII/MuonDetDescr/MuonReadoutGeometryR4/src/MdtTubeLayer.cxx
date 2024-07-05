/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/MdtTubeLayer.h>
#include <GeoModelKernel/GeoTube.h>
#include <GeoModelKernel/GeoAccessVolumeAction.h>

#include <GeoModelHelpers/TransformSorter.h>
#include <GeoModelHelpers/GeoPhysVolSorter.h>
#include <GeoModelUtilities/GeoVisitVolumes.h>


namespace {
// Helper to find the Nth child volume, without keeping track
// of the transform.
class GeoGetChild : public IGeoVisitVolumesNoXformAction
{
public:
  GeoGetChild (size_t n) : m_n (n) {}
  virtual void operator() (GeoNodeAction& action,
                           int /*id*/,
                           const std::string& /*name*/,
                           const GeoVPhysVol* volume) override
  {
    if (m_n == 0) {
      m_node = volume;
      action.terminate();
    }
    else {
      --m_n;
    }
  }

  size_t m_n{0};
  const GeoVPhysVol* m_node{nullptr};
};
} // anonymous namespace

namespace MuonGMR4{

 
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
    m_cutTubes{cutTubes} {}

const Amg::Transform3D& MdtTubeLayer::layerTransform() const {
    return m_layTrf->getDefTransform();
}
PVConstLink MdtTubeLayer::getTubeNode(unsigned int tube) const {
     if (tube >= nTubes()) {
        std::stringstream except{};
        except<<__FILE__<<":"<<__LINE__<<" "<<m_layerNode->getLogVol()->getName()<<" has only "<<nTubes()<<" tubes. But "<<tube<<" is requested. Please check.";
        throw std::out_of_range(except.str());
    }

    // Don't use getChildVol.  Under the hood, that keeps track of the
    // transform to each child, even though that is not actually used.
    // This is slow, especially in debug builds where Eigen operations
    // tend to be expensive.
    GeoGetChild a (tube);
    GeoVisitVolumes visitor (a);
    m_layerNode->exec (&visitor);
    return a.m_node;
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
GeoVolumeCursor MdtTubeLayer::tubeCursor() const {
  return GeoVolumeCursor(m_layerNode);
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

