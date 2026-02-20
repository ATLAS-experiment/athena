/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelUtilities/GeoAlignmentStore.h"

void GeoAlignmentStore::setDelta(const GeoAlignableTransform* axf, const GeoTrf::Transform3D& xf) { 
    setDelta(axf, std::make_shared<GeoTrf::Transform3D>(xf));
}

void GeoAlignmentStore::setDelta(const GeoAlignableTransform* axf, std::shared_ptr<const GeoTrf::Transform3D> trans) {
    m_deltas->setTransform(axf, std::move(trans));
}
void GeoAlignmentStore::setAbsPosition(const GeoNodePositioning* fpv, const GeoTrf::Transform3D& xf) { 
    m_absPositions->setTransform(fpv, xf); 
}
void GeoAlignmentStore::setDefAbsPosition(const GeoNodePositioning* fpv, const GeoTrf::Transform3D& xf) {
    m_defAbsPositions->setTransform(fpv, xf);
}
void GeoAlignmentStore::lockDelta() {
    m_deltas->lock();
}
void GeoAlignmentStore::lockPosCache() {
   m_absPositions->lock();
   m_defAbsPositions->lock();
}


bool GeoAlignmentStore::posCacheLocked() const { return m_deltas->isLocked(); }
bool GeoAlignmentStore::deltaCacheLocked() const { return m_absPositions->isLocked(); }


bool GeoAlignmentStore::append(const GeoAlignmentStore& other) {
    bool allGood{m_deltas->append(*other.m_deltas)};
    allGood = m_absPositions->append(*other.m_absPositions) && allGood;
    allGood = m_defAbsPositions->append(*other.m_defAbsPositions) && allGood;
    return allGood;
}

GeoAlignmentStore::DeltaMapPtr GeoAlignmentStore::getDeltas() const { 
    return m_deltas; 
}
GeoAlignmentStore::PositioningMapPtr GeoAlignmentStore::getAbsPositions() const { 
    return m_absPositions; 
}
GeoAlignmentStore::PositioningMapPtr GeoAlignmentStore::getDefAbsPositions() const { 
    return m_defAbsPositions; 
}
void GeoAlignmentStore::clearPosCache() {
    m_absPositions = std::make_unique<PositioningMap>();
    m_defAbsPositions = std::make_unique<PositioningMap>();
}
