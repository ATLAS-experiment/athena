/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BCMPrimeReadoutGeometry/BCMPrimeDetectorManager.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "InDetIdentifier/BCMPrime_ID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "StoreGate/StoreGateSvc.h"

#include <stdexcept>

namespace InDetDD {

    BCMPrimeDetectorManager::BCMPrimeDetectorManager(StoreGateSvc* detStore, const std::string& name)
      : SiDetectorManager(detStore, name)
    {
        const BCMPrime_ID* idHelper = nullptr;
        StatusCode sc = detStore->retrieve(idHelper, "BCMPrime_ID");
        if (sc.isFailure()) {
            ATH_MSG_ERROR("Could not retrieve BCMPrime_ID helper");
        }
        m_idHelper = idHelper;
        if (m_idHelper) {
            m_elementCollection.resize(m_idHelper->wafer_hash_max());
            m_alignableTransforms.resize(m_idHelper->wafer_hash_max());
        }
    }

    unsigned int BCMPrimeDetectorManager::getNumTreeTops() const {
        return m_volume.size();
    }

    PVConstLink BCMPrimeDetectorManager::getTreeTop(unsigned int i) const {
        return m_volume[i];
    }

    void BCMPrimeDetectorManager::addTreeTop(const PVConstLink& vol) {
        m_volume.push_back(vol);
    }

    const SiDetectorElement* BCMPrimeDetectorManager::getDetectorElement(const Identifier& id) const {
        Identifier waferId = m_idHelper->wafer_id(id);
        IdentifierHash idHash = m_idHelper->wafer_hash(waferId);
        if (idHash.is_valid()) {
            return m_elementCollection[idHash];
        }
        return nullptr;
    }

    const SiDetectorElement* BCMPrimeDetectorManager::getDetectorElement(const IdentifierHash& idHash) const {
        return m_elementCollection[idHash];
    }

    const SiDetectorElement* BCMPrimeDetectorManager::getDetectorElement(int barrelEndcap,
                                                                         int layerWheel,
                                                                         int phiModule,
                                                                         int etaModule) const {
        return getDetectorElement(m_idHelper->wafer_id(barrelEndcap, layerWheel, phiModule, etaModule));
    }

    const SiDetectorElementCollection* BCMPrimeDetectorManager::getDetectorElementCollection() const {
        return &m_elementCollection;
    }

    SiDetectorElementCollection::const_iterator BCMPrimeDetectorManager::getDetectorElementBegin() const {
        return m_elementCollection.begin();
    }

    SiDetectorElementCollection::const_iterator BCMPrimeDetectorManager::getDetectorElementEnd() const {
        return m_elementCollection.end();
    }

    SiDetectorElementCollection::iterator BCMPrimeDetectorManager::getDetectorElementBegin() {
        return m_elementCollection.begin();
    }

    SiDetectorElementCollection::iterator BCMPrimeDetectorManager::getDetectorElementEnd() {
        return m_elementCollection.end();
    }

    void BCMPrimeDetectorManager::addDetectorElement(SiDetectorElement* element) {
        IdentifierHash idHash = element->identifyHash();
        if (idHash >= m_elementCollection.size()) {
            throw std::runtime_error("BCMPrimeDetectorManager: Error adding detector element.");
        }
        m_elementCollection[idHash] = element;
    }

    void BCMPrimeDetectorManager::initNeighbours() {
        // BCMPrime pad neighbours are not yet needed by the G4 hit path.
    }

    bool BCMPrimeDetectorManager::identifierBelongs(const Identifier& id) const {
        return m_idHelper && m_idHelper->is_lumi(id);
    }

    void BCMPrimeDetectorManager::addAlignableTransform(int,
                                                        const Identifier&,
                                                        GeoAlignableTransform*) {
        // BCMPrime alignment is not wired yet.
    }

    bool BCMPrimeDetectorManager::processSpecialAlignment(const std::string&,
                                                          InDetDD::AlignFolderType) {
        return false;
    }

    bool BCMPrimeDetectorManager::processSpecialAlignment(const std::string&,
                                                          const CondAttrListCollection*,
                                                          GeoVAlignmentStore*) const {
        return false;
    }

    bool BCMPrimeDetectorManager::setAlignableTransformDelta(int,
                                                             const Identifier&,
                                                             const Amg::Transform3D&,
                                                             FrameType,
                                                             GeoVAlignmentStore*) const {
        return false;
    }

    const PixelID* BCMPrimeDetectorManager::getIdHelper() const {
        return m_idHelper;
    }

    unsigned int BCMPrimeDetectorManager::getNumDetectorElements() const {
        unsigned int count = 0;
        for (const SiDetectorElement* element : m_elementCollection) {
            if (element) {
                ++count;
            }
        }
        return count;
    }

} // namespace InDetDD
