/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "HGTD_ReadoutGeometry/HGTD_DetectorManager.h"

#include "StoreGate/StoreGateSvc.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"

#include "InDetReadoutGeometry/ExtendedAlignableTransform.h"
#include "Identifier/IdentifierHash.h"
#include "GeoModelKernel/GeoVAlignmentStore.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"

using InDetDD::HGTD_DetectorElementCollection;
using InDetDD::HGTD_DetectorElement;
using InDetDD::SiCommonItems;

HGTD_DetectorManager::HGTD_DetectorManager(StoreGateSvc* detStore)
    : AthMessaging("HGTD_DetectorManager"),
      m_idHelper(0)
{
    setName("HGTD");

    //
    // Initialize the Identifier helper
    //
    StatusCode sc = detStore->retrieve(m_idHelper,"HGTD_ID");
    if (sc.isFailure() ) {
        ATH_MSG_ERROR ("Could not retrieve HGTD id helper");
    }

    // Initialize the collections
    if (m_idHelper) {
        m_elementCollection.resize(m_idHelper->wafer_hash_max());
        m_alignableTransforms.resize(m_idHelper->wafer_hash_max());
    }
    ATH_MSG_INFO("HGTD_DetectorManager initialized");
}

HGTD_DetectorManager::~HGTD_DetectorManager() = default;

unsigned int HGTD_DetectorManager::getNumTreeTops() const
{
    return m_volume.size();
}

PVConstLink HGTD_DetectorManager::getTreeTop(unsigned int i) const
{
    return m_volume[i];
}

void HGTD_DetectorManager::addTreeTop(PVConstLink vol){
    m_volume.push_back(vol);
}

const HGTD_DetectorElement* HGTD_DetectorManager::getDetectorElement(const Identifier & id) const
{
    // Make sure it is a wafer Id
    Identifier waferId =  m_idHelper->wafer_id(id);
    IdentifierHash idHash = m_idHelper->wafer_hash(waferId);
    if (idHash.is_valid()) {
        return m_elementCollection[idHash];
    } else {
        return 0;
    }
}

const HGTD_DetectorElement* HGTD_DetectorManager::getDetectorElement(const IdentifierHash & idHash) const
{
    return m_elementCollection[idHash];
}

const HGTD_DetectorElement* HGTD_DetectorManager::getDetectorElement(int endcap, int layer, int phi_module, int eta_module) const
{
    return getDetectorElement(m_idHelper->wafer_id(endcap, layer, phi_module, eta_module));
}

const HGTD_DetectorElementCollection* HGTD_DetectorManager::getDetectorElementCollection() const
{
    return &m_elementCollection;
}

void HGTD_DetectorManager::addDetectorElement(HGTD_DetectorElement * element)
{
    IdentifierHash idHash = element->identifyHash();
    if (idHash >=  m_elementCollection.size()) {
        throw std::runtime_error(
            "HGTD_DetectorManager: Error adding detector element."
        );
    }
    m_elementCollection[idHash] = element;
}

// Register alignable transform
void HGTD_DetectorManager::addAlignableTransform(int level,
                                                 const Identifier& id,
                                                 GeoAlignableTransform* transform,
                                                 const GeoVFullPhysVol* child)
{
    (void)level;

    if (!m_idHelper) return;

    IdentifierHash idHash = m_idHelper->wafer_hash(id);

    if (!idHash.is_valid()) {
        ATH_MSG_WARNING("Invalid idHash for alignable transform");
        return;
    }
    
    ATH_MSG_DEBUG("STORE ALIGNABLE:"
                << " hash=" << idHash
                << " transform ptr=" << transform
                << " child ptr=" << child);

    m_alignableTransforms[idHash] =
        std::make_unique<InDetDD::ExtendedAlignableTransform>(transform, child);
    
    ATH_MSG_DEBUG("Alignable container size = " 
                << m_alignableTransforms.size());

    ATH_MSG_DEBUG("HGTD ALIGNABLE ADDED: idHash = " << idHash);

    int count = 0;
    for (const auto& t : m_alignableTransforms) {
        if (t) count++;
    }

    ATH_MSG_DEBUG("HGTD alignable transforms registered: " << count);
}

bool HGTD_DetectorManager::setAlignableTransformDelta(int level,
                                                      const Identifier& id,
                                                      const Amg::Transform3D& delta,
                                                      GeoVAlignmentStore* alignStore) const
{
    (void)level;
    
    IdentifierHash idHash = m_idHelper->wafer_hash(id);

    if (!idHash.is_valid()) return false;

    ATH_MSG_DEBUG("idHash = " << idHash);

    auto* transform = m_alignableTransforms[idHash].get();
    ATH_MSG_DEBUG("RETRIEVE ALIGNABLE:"
             << " hash=" << idHash
             << " ext transform ptr=" << transform
             << " geo alignable ptr="
             << (transform ? transform->alignableTransform() : nullptr));

    ATH_MSG_DEBUG("transform ptr = " << transform);
    
    if (!transform){
        ATH_MSG_ERROR("NO ALIGNABLE FOUND");
        return false;
    }
    ATH_MSG_DEBUG("SETTING DELTA dx="
                << delta.translation().x()
                << " dy="
                << delta.translation().y()
                << " dz="
                << delta.translation().z());
                
    alignStore->setDelta(transform->alignableTransform(), delta);

    return true;

}

StatusCode HGTD_DetectorManager::align(
    const AlignableTransformContainer* container,
    GeoVAlignmentStore* alignStore) const
{
    ATH_MSG_DEBUG("Applying HGTD alignment");

    ATH_MSG_DEBUG("Entered align()");
    if (!container) {
        ATH_MSG_ERROR("Null AlignableTransformContainer");
        return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Container pointer = " << container);
    ATH_MSG_DEBUG("Container size = " << container->size());

    if (container->empty()) {
        ATH_MSG_WARNING("AlignableTransformContainer is empty");
        return StatusCode::SUCCESS;
    }

    ATH_MSG_DEBUG("AlignableTransformContainer has "
                << container->size()
                << " AlignableTransform collection(s)");

    // Use only the last tag of each AlignableTransform, exactly like InDet
    std::map<std::string,const AlignableTransform*> transforms;

    for (const auto* pat : *container) {

        if (!pat) {
            ATH_MSG_WARNING("Null AlignableTransform pointer");
            continue;
        }

        ATH_MSG_DEBUG("--------------------------------");
        ATH_MSG_DEBUG("Collection tag = " << pat->tag());
        ATH_MSG_DEBUG("Collection size = " << pat->size());
        transforms[pat->tag()] = pat;

    }

    for (const auto& entry : transforms) {

        const AlignableTransform* transformCollection = entry.second;

        ATH_MSG_DEBUG("Processing tag " << entry.first);

        for (AlignableTransform::AlignTransMem_citr transIter =
                 transformCollection->begin();
             transIter != transformCollection->end();
             ++transIter)
        {

            Identifier id = transIter->identify();
            
            ATH_MSG_DEBUG("--------------------------------");
            ATH_MSG_DEBUG("Identifier = " << id.get_compact());
     
            Amg::Transform3D delta =
                Amg::CLHEPTransformToEigen(transIter->transform());

            ATH_MSG_DEBUG("Translation = ("
                        << delta.translation().x() << ", "
                        << delta.translation().y() << ", "
                        << delta.translation().z() << ")");

            bool ok =
                setAlignableTransformDelta(
                    0,
                    id,
                    delta,
                    alignStore);

            ATH_MSG_DEBUG("setAlignableTransformDelta returned "
                        << std::boolalpha << ok);

            if (!ok) {
                ATH_MSG_WARNING("Failed to apply alignment for identifier "
                                << id.get_compact());
            }
        }
    }

    return StatusCode::SUCCESS;
}

const HGTD_ID* HGTD_DetectorManager::getIdHelper() const
{
    return m_idHelper;
}

void HGTD_DetectorManager::invalidateAll()
{
  for (HGTD_DetectorElement* element : m_elementCollection) {
    if (element) {
      element->invalidate();
    }
  }
}

void HGTD_DetectorManager::updateAll() const
{
  for (const HGTD_DetectorElement* element : m_elementCollection) {
    if (element) {
      element->updateCache();
    }
  }
}

void HGTD_DetectorManager::setCommonItems(std::unique_ptr<const SiCommonItems>&& commonItems)
{
    m_commonItems = std::move(commonItems);
}
