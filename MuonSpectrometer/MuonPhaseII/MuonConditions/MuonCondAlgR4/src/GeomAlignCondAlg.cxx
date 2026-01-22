/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GeomAlignCondAlg.h"

#include <StoreGate/ReadCondHandle.h>
#include <GeoModelKernel/GeoPerfUtils.h>
#include <GeoModelKernel/GeoClearAbsPosAction.h>
#include <AthenaKernel/IOVInfiniteRange.h>


#include "Acts/Utilities/Helpers.hpp"
using namespace MuonGMR4;

namespace MuonR4{
StatusCode GeomAlignCondAlg::initialize() {
    
    ATH_CHECK(m_readKeyALines.initialize(m_applyALines));
    ATH_CHECK(m_readKeyBLines.initialize(m_applyBLines));
    ATH_CHECK(m_readMdtAsBuiltKey.initialize(m_applyMdtAsBuilt));
    ATH_CHECK(m_readNswAsBuiltKey.initialize(m_applyNswAsBuilt));
    ATH_CHECK(m_readNswPassivKey.initialize(m_applyMmPassivation));
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    m_techs = m_detMgr->getDetectorTypes();
    if (m_techs.empty()) {
        ATH_MSG_FATAL("The detector manager does not contain any elements");
        return StatusCode::FAILURE;
    }
    auto hasDetector = [this](const ActsTrk::DetectorType d) -> bool {
        return Acts::rangeContainsValue(m_techs, d);
    };
    m_applyBLines = m_applyBLines && (hasDetector(ActsTrk::DetectorType::Mdt) ||
                                      hasDetector(ActsTrk::DetectorType::Mm) ||
                                      hasDetector(ActsTrk::DetectorType::sTgc));
    m_applyMdtAsBuilt = m_applyMdtAsBuilt && hasDetector(ActsTrk::DetectorType::Mdt);
    m_applyNswAsBuilt = m_applyNswAsBuilt && (hasDetector(ActsTrk::DetectorType::Mm) ||
                                              hasDetector(ActsTrk::DetectorType::sTgc)); 
    m_applyMmPassivation = m_applyMmPassivation && hasDetector(ActsTrk::DetectorType::Mm);

   

    for (const ActsTrk::DetectorType det : m_techs) {
        m_writeKeys.emplace_back(ActsTrk::to_string(det) + m_keyToken);
        ATH_MSG_INFO("Register new alignment container "<<m_writeKeys.back().fullKey());
    }
    ATH_MSG_INFO("Switched options "<<m_fillAlignStoreCache<<", "<<", "<<m_applyALines<<", "
                <<m_applyBLines<<", "<<m_applyMdtAsBuilt<<", "
                <<","<<m_applyNswAsBuilt<<", "<<m_applyMmPassivation);
    ATH_CHECK(m_writeKeys.initialize());
    return StatusCode::SUCCESS;
}

Identifier GeomAlignCondAlg::alignmentId(const MuonGMR4::MuonReadoutElement* re) const {
    if (re->detectorType() == ActsTrk::DetectorType::Mdt || 
        re->detectorType() == ActsTrk::DetectorType::Tgc) {
            return m_idHelperSvc->chamberId(re->identify());
    } else if (re->detectorType() == ActsTrk::DetectorType::Rpc) {
        /// The BML eta 7 stations have their own alignment. 
        if (!m_idHelperSvc->hasMDT() || 
            (std::abs(re->stationEta()) == 7 && m_idHelperSvc->stationNameString(re->identify()) == "BML")) {
            return m_idHelperSvc->rpcIdHelper().elementID(re->stationName(), re->stationEta(), re->stationPhi(), 1);
        /// The rest shares the same alignmnet constants with the Mdts
        } else {
            return m_idHelperSvc->mdtIdHelper().elementID(re->stationName(), re->stationEta(), re->stationPhi());
        }
    }
    /// For the NSW, the alignment parameters are stored under the same key as the RE
    return re->identify();
}
StatusCode GeomAlignCondAlg::loadDeltas(const EventContext& ctx,
                                        deltaMap& alignDeltas,
                                        alignTechMap& techTransforms) const {
    if (m_readKeyALines.empty()) {
        ATH_MSG_DEBUG("Loading of the A line parameters deactivated");
        return StatusCode::SUCCESS;
    }
    const ALineContainer* aLineContainer{};
    ATH_CHECK(SG::get(aLineContainer, m_readKeyALines, ctx));
    std::vector<const MuonReadoutElement*> readoutEles = m_detMgr->getAllReadoutElements();
    ATH_MSG_INFO("Load the alignment of "<<readoutEles.size()<<" detector elements");
    for (const MuonReadoutElement* re : readoutEles) {
        const GeoAlignableTransform* alignTrans = re->alignableTransform();
        if (!alignTrans) {
            ATH_MSG_WARNING("The readout element "<<m_idHelperSvc->toStringDetEl(re->identify())
                            <<" has no alignable transform.");
            continue;
        }
        std::shared_ptr<const Amg::Transform3D>& cached = alignDeltas[alignTrans];
        if (cached) {
            ATH_MSG_DEBUG("The alignable transformation for "<<m_idHelperSvc->toStringChamber(re->identify())
                         <<" has been cached before. ");
            techTransforms[re->detectorType()].insert(alignTrans);
            continue;
        }
        /// Construct the identifier to search for the proper Aline transformation 
        const Identifier stationId = alignmentId(re);
        ALineContainer::const_iterator aLineItr = aLineContainer->find(stationId);
        if (aLineItr == aLineContainer->end()) {
            ATH_MSG_VERBOSE("No Alines were stored for "<<m_idHelperSvc->toString(re->identify())
                          <<". Used "<<m_idHelperSvc->toString(stationId)<<" as station Identifier");
            continue;
        }
        /// Store the alignable transformation
        cached = std::make_shared<Amg::Transform3D>(aLineItr->delta());
        techTransforms[re->detectorType()].insert(alignTrans);
    }
    return StatusCode::SUCCESS;
}
StatusCode GeomAlignCondAlg::loadMdtDeformPars(const EventContext& ctx,
                                               ActsTrk::DetectorAlignStore& store) const {
    
    if (!m_applyMdtAsBuilt  && !m_applyBLines) {
        return StatusCode::SUCCESS;
    }
    auto internAlign = std::make_unique<MdtAlignmentStore>(m_idHelperSvc.get());
    const MdtAsBuiltContainer* asBuiltCont{nullptr};
    const BLineContainer* bLines{nullptr};
    
    ATH_CHECK(SG::get(asBuiltCont, m_readMdtAsBuiltKey, ctx));
    ATH_CHECK(SG::get(bLines, m_readKeyBLines, ctx));
    
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    for (auto itr = idHelper.module_begin(); itr != idHelper.module_end(); ++itr) {
       const Identifier& stationId{*itr};
        const BLinePar* bline{nullptr};
        if (bLines) {
            BLineContainer::const_iterator itr = bLines->find(stationId);
            if (itr != bLines->end()) bline = &(*itr);
        }
        const MdtAsBuiltPar* asBuilt{nullptr};
        if (asBuiltCont) {
            MdtAsBuiltContainer::const_iterator itr = asBuiltCont->find(stationId);
            if (itr != asBuiltCont->end()) asBuilt = &(*itr);
        }
        if (asBuilt || bline) {
            internAlign->storeDistortion(stationId, bline, asBuilt);
        }       
    }
    // Down cast the alignment pointer
    store.internalAlignment = std::move(internAlign);
    return StatusCode::SUCCESS;
}
StatusCode GeomAlignCondAlg::loadMmDeformPars(const EventContext& ctx,
                                              ActsTrk::DetectorAlignStore& store) const {
    if (!m_applyMmPassivation && !m_applyNswAsBuilt && !m_applyBLines) {
        return StatusCode::SUCCESS;
    }
    auto internAlign = std::make_unique<MmAlignmentStore>();
    const NswAsBuiltDbData* asBuiltPars{};
    ATH_CHECK(SG::get(internAlign->passivation, m_readNswPassivKey, ctx));
    ATH_CHECK(SG::get(asBuiltPars, m_readNswAsBuiltKey, ctx));
    if (asBuiltPars) {
        internAlign->asBuiltPars = asBuiltPars->microMegaData;
    }
    const BLineContainer* bLines{};
    ATH_CHECK(SG::get(bLines, m_readKeyBLines, ctx));
    if (bLines) {
        for (const MmReadoutElement* re : m_detMgr->getAllMmReadoutElements()){
            const Identifier stationId = alignmentId(re);
            BLineContainer::const_iterator itr = bLines->find(stationId);
            if (itr != bLines->end()) {
                internAlign->cacheBLine(re->identify(), *itr);
            }
        }
    }
    store.internalAlignment = std::move(internAlign);
    return StatusCode::SUCCESS;
}
StatusCode GeomAlignCondAlg::loadStgcDeformPars(const EventContext& ctx,
                                                ActsTrk::DetectorAlignStore& store) const{
    if (!(m_applyNswAsBuilt && !m_readsTgcAsBuiltKey.empty()) && !m_applyBLines) {
        return StatusCode::SUCCESS;
    }
    auto internAlign = std::make_unique<sTgcAlignmentStore>();
    ATH_CHECK(SG::get(internAlign->asBuiltPars, m_readsTgcAsBuiltKey, ctx));
    const BLineContainer* bLines{};
    ATH_CHECK(SG::get(bLines, m_readKeyBLines, ctx));
    if (bLines) {
        for (const sTgcReadoutElement* re : m_detMgr->getAllsTgcReadoutElements()) {
            const Identifier stationId = alignmentId(re);
            BLineContainer::const_iterator itr = bLines->find(stationId);
            if (itr != bLines->end()) {
                internAlign->cacheBLine(re->identify(), *itr);
            }
        }
    }
    store.internalAlignment = std::move(internAlign);
    return StatusCode::SUCCESS;
}

StatusCode GeomAlignCondAlg::declareDependencies(const EventContext& ctx,
                                                 ActsTrk::DetectorType detType,
                                                 SG::WriteCondHandle<ActsTrk::DetectorAlignStore>& writeHandle) const {
    writeHandle.addDependency(IOVInfiniteRange::infiniteTime());
    if (m_applyALines) {
        SG::ReadCondHandle depHandle{m_readKeyALines, ctx};
        ATH_CHECK(depHandle.isValid());
        writeHandle.addDependency(depHandle);
    }
    const bool issTGC = detType == ActsTrk::DetectorType::sTgc;
    const bool isMm   = detType == ActsTrk::DetectorType::Mm;
    const bool isMdt = detType == ActsTrk::DetectorType::Mdt;
    
    if (m_applyBLines&& (issTGC || isMm || isMdt)) {
        SG::ReadCondHandle depHandle{m_readKeyBLines, ctx};
        ATH_CHECK(depHandle.isValid());
        writeHandle.addDependency(depHandle);
    }
    if (m_applyMdtAsBuilt && isMdt) {
        SG::ReadCondHandle depHandle{m_readMdtAsBuiltKey, ctx};
        ATH_CHECK(depHandle.isValid());
        writeHandle.addDependency(depHandle);
    }
    if (m_applyMmPassivation && detType == ActsTrk::DetectorType::Mm) {
        SG::ReadCondHandle depHandle{m_readNswPassivKey, ctx};
        ATH_CHECK(depHandle.isValid());
        writeHandle.addDependency(depHandle);
    }
    if (m_applyNswAsBuilt && isMm) {
        SG::ReadCondHandle depHandle{m_readNswAsBuiltKey, ctx};
        ATH_CHECK(depHandle.isValid());
        writeHandle.addDependency(depHandle);
    }
    if(m_applyNswAsBuilt && issTGC && !m_readsTgcAsBuiltKey.empty()){
        SG::ReadCondHandle depHandle{m_readsTgcAsBuiltKey, ctx};
        ATH_CHECK(depHandle.isValid());
        writeHandle.addDependency(depHandle);
    }
    return StatusCode::SUCCESS;
}

StatusCode GeomAlignCondAlg::execute(const EventContext& ctx) const {
    deltaMap alignDeltas{};
    alignTechMap techTransforms{};
    const unsigned memBeforeAlign = GeoPerfUtils::getMem();
    ATH_CHECK(loadDeltas(ctx, alignDeltas, techTransforms));
    
    /// Create the condition handles
    for (std::size_t det =0 ; det < m_techs.size(); ++det) {
        const SG::WriteCondHandleKey<ActsTrk::DetectorAlignStore>& key = m_writeKeys[det];
        const ActsTrk::DetectorType subDet = m_techs[det];

        SG::WriteCondHandle<ActsTrk::DetectorAlignStore> writeHandle{key, ctx};
        if (writeHandle.isValid()) {
            ATH_MSG_VERBOSE("The alignment constants for "<<ActsTrk::to_string(subDet)
                          <<" are still valid.");
            continue;
        }
        auto writeCdo = std::make_unique<ActsTrk::DetectorAlignStore>(subDet);

        const std::set<const GeoAlignableTransform*>& toStore =  techTransforms[subDet];
        /// Append the alignable transformations to the conditions object
        for (const GeoAlignableTransform* alignable : toStore) {
           const std::shared_ptr<const Amg::Transform3D>& cached = alignDeltas[alignable];
           if (!cached) continue;
           writeCdo->geoModelAlignment->setDelta(alignable, alignDeltas[alignable]);
        }
        writeCdo->geoModelAlignment->lockDelta();
        if (subDet == ActsTrk::DetectorType::Mdt) {
            ATH_CHECK(loadMdtDeformPars(ctx,*writeCdo));
        } else if (subDet == ActsTrk::DetectorType::Mm) {
            ATH_CHECK(loadMmDeformPars(ctx, *writeCdo));
        } else if (subDet == ActsTrk::DetectorType::sTgc) {
            ATH_CHECK(loadStgcDeformPars(ctx, *writeCdo));
        }
        /// Propagate the cache throughout the geometry
        ATH_CHECK(declareDependencies(ctx, subDet, writeHandle));
        /// Cache all transforms at the creation of this conditions object
        if (m_fillAlignStoreCache) {
            unsigned numAligned{0};
            std::ranges::for_each(m_detMgr->getAllReadoutElements(subDet),
                [&](const MuonGMR4::MuonReadoutElement* re) {
                    numAligned += re->storeAlignedTransforms(*writeCdo);
                });
            /// The geoModel constants are no longer needed.
            writeCdo->geoModelAlignment.reset();
            ATH_MSG_DEBUG("Populated the alignment store "<<to_string(subDet)<<" with "<<numAligned<<" transforms");
        } else if (m_fillGeoAlignStore) {
            /// Ensure that the rigid transformations of the detector elements are applied 
            std::ranges::for_each(m_detMgr->getAllReadoutElements(subDet),
                    [&](const MuonReadoutElement* re){
                        const Amg::Transform3D& detTrf{re->getMaterialGeom()->getAbsoluteTransform(writeCdo->geoModelAlignment.get())};
                        ATH_MSG_VERBOSE("Detector element "<<m_idHelperSvc->toStringDetEl(re->identify())<<" is located at "
                                    <<Amg::toString(detTrf));     
                    });
            /// There's no need to cache the delta parameters longer
            writeCdo->geoModelAlignment->getDeltas()->clear();
            writeCdo->geoModelAlignment->lockPosCache();
        } 
        ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    }

    alignDeltas.clear();
    techTransforms.clear();
    /// Whipe the GeoModelCache
    GeoClearAbsPosAction whipeTreeTop{};
    for (unsigned treeTop = 0 ; treeTop < m_detMgr->getNumTreeTops(); ++treeTop) {
        m_detMgr->getTreeTop(treeTop)->exec(&whipeTreeTop);
    }

    const unsigned memAfterAlign  = GeoPerfUtils::getMem();
    ATH_MSG_INFO("Caching of the alignment parameters required "<<(memAfterAlign - memBeforeAlign) / 1024<<" MB of memory");
    return StatusCode::SUCCESS;
}
}
