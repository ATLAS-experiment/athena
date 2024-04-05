/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsMuonAlignCondAlg.h"

#include <StoreGate/ReadCondHandle.h>
#include <GeoModelKernel/GeoPerfUtils.h>
#include <GeoModelKernel/GeoClearAbsPosAction.h>
#include <AthenaKernel/IOVInfiniteRange.h>


using namespace MuonGMR4;

#define CREATE_READHANDLE(CONT_TYPE, KEY)                  \
     SG::ReadCondHandle<CONT_TYPE> readHandle{KEY, ctx};   \
     if (!readHandle.isValid()) {                          \
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__              \
                    <<" Failed to load "<<KEY.fullKey());  \
        return StatusCode::FAILURE;                        \
     }

ActsMuonAlignCondAlg::ActsMuonAlignCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthReentrantAlgorithm{name, pSvcLocator}{}

StatusCode ActsMuonAlignCondAlg::initialize() {
    
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
        return std::find(m_techs.begin(),m_techs.end(), d) != m_techs.end();
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
    ATH_CHECK(m_writeKeys.initialize());
    return StatusCode::SUCCESS;
}

Identifier ActsMuonAlignCondAlg::alignmentId(const MuonGMR4::MuonReadoutElement* re) const {
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
StatusCode ActsMuonAlignCondAlg::loadDeltas(const EventContext& ctx,
                                            deltaMap& alignDeltas,
                                            alignTechMap& techTransforms) const {
    if (m_readKeyALines.empty()) {
        ATH_MSG_DEBUG("Loading of the A line parameters deactivated");
        return StatusCode::SUCCESS;
    }
    
    CREATE_READHANDLE(ALineContainer, m_readKeyALines);
    const ALineContainer* aLineContainer{*readHandle};
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
StatusCode ActsMuonAlignCondAlg::loadMdtDeformPars(const EventContext& ctx,
                                                   ActsTrk::DetectorAlignStore& store) const {
    
    if (!m_applyMdtAsBuilt  && !m_applyBLines) {
        return StatusCode::SUCCESS;
    }
    std::unique_ptr<MdtAlignmentStore> internAlign = std::make_unique<MdtAlignmentStore>();
    const MdtAsBuiltContainer* asBuiltCont{nullptr};
    const BLineContainer* bLines{nullptr};
    if (m_applyMdtAsBuilt) {
        CREATE_READHANDLE(MdtAsBuiltContainer, m_readMdtAsBuiltKey);
        asBuiltCont = readHandle.cptr();
    }
    if (m_applyBLines) {
        CREATE_READHANDLE(BLineContainer, m_readKeyBLines);
        bLines = readHandle.cptr();        
    }

    std::vector<const MdtReadoutElement*> reEles = m_detMgr->getAllMdtReadoutElements();
    for (const MdtReadoutElement* re : reEles) {
        const Identifier stationId = alignmentId(re);
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
        if (asBuilt || bline) internAlign->storeDistortion(re->identify(), bline, asBuilt);
    }
    // Down cast the alignment pointer
    store.internalAlignment = std::move(internAlign);
    return StatusCode::SUCCESS;
}
StatusCode ActsMuonAlignCondAlg::loadMmDeformPars(const EventContext& ctx,
                                                  ActsTrk::DetectorAlignStore& store) const {
    if (!m_applyMmPassivation && !m_applyNswAsBuilt && !m_applyBLines) {
        return StatusCode::SUCCESS;
    }
    std::unique_ptr<MmAlignmentStore> internAlign = std::make_unique<MmAlignmentStore>();
    if (m_applyMmPassivation) {
        CREATE_READHANDLE(NswPassivationDbData, m_readNswPassivKey);
        internAlign->passivation = readHandle.cptr();
    }
    if (m_applyNswAsBuilt) {
        CREATE_READHANDLE(NswAsBuiltDbData, m_readNswAsBuiltKey);
        internAlign->asBuiltPars = readHandle->microMegaData;
    }
    if (m_applyBLines) {
        CREATE_READHANDLE(BLineContainer, m_readKeyBLines);
        /// Replace it by the vector of MmReadout elements once these are implemented
        std::vector<const MuonReadoutElement*> reEles = m_detMgr->getAllReadoutElements();
        for (const MuonReadoutElement* re : reEles){
            if (re->detectorType() != ActsTrk::DetectorType::Mm) continue;
            const Identifier stationId = alignmentId(re);
            BLineContainer::const_iterator itr = readHandle->find(stationId);
            if (itr != readHandle->end()) internAlign->cacheBLine(re->identify(), *itr);
        }
    }
    store.internalAlignment = std::move(internAlign);
    return StatusCode::SUCCESS;
}
StatusCode ActsMuonAlignCondAlg::loadStgcDeformPars(const EventContext& ctx,
                                                    ActsTrk::DetectorAlignStore& store) const{
    
    if (!m_applyNswAsBuilt && !m_applyBLines) {
        return StatusCode::SUCCESS;
    }
    std::unique_ptr<sTgcAlignmentStore> internalAlign = std::make_unique<sTgcAlignmentStore>();
    if (m_applyNswAsBuilt) {
        CREATE_READHANDLE(NswAsBuiltDbData, m_readNswAsBuiltKey);
        internalAlign->asBuiltPars = readHandle->sTgcData;
    }
    if (m_applyBLines) {
        CREATE_READHANDLE(BLineContainer, m_readKeyBLines);
        /// Replace it by the vector of MmReadout elements once these are implemented
        std::vector<const MuonReadoutElement*> reEles = m_detMgr->getAllReadoutElements();
        for (const MuonReadoutElement* re : reEles){
            if (re->detectorType() != ActsTrk::DetectorType::sTgc) continue;
            const Identifier stationId = alignmentId(re);
            BLineContainer::const_iterator itr = readHandle->find(stationId);
            if (itr != readHandle->end()) internalAlign->cacheBLine(re->identify(), *itr);
        }
    }
    store.internalAlignment = std::move(internalAlign);
    return StatusCode::SUCCESS;
}

StatusCode ActsMuonAlignCondAlg::declareDependencies(const EventContext& ctx,
                                                     ActsTrk::DetectorType detType,
                                                     SG::WriteCondHandle<ActsTrk::DetectorAlignStore>& writeHandle) const {
    writeHandle.addDependency(IOVInfiniteRange::infiniteTime());
    if (m_applyALines) {
        CREATE_READHANDLE(ALineContainer, m_readKeyALines);
        writeHandle.addDependency(readHandle);
    }
    const bool isNsw = detType == ActsTrk::DetectorType::sTgc ||
                       detType == ActsTrk::DetectorType::Mm;
    const bool isMdt = detType == ActsTrk::DetectorType::Mdt;
    if (m_applyBLines&& (isNsw || isMdt)) {
        CREATE_READHANDLE(BLineContainer, m_readKeyBLines);
        writeHandle.addDependency(readHandle);
    }
    if (m_applyMdtAsBuilt && isMdt) {
        CREATE_READHANDLE(MdtAsBuiltContainer, m_readMdtAsBuiltKey);
        writeHandle.addDependency(readHandle);
    }
    if (m_applyMmPassivation && detType == ActsTrk::DetectorType::Mm) {
        CREATE_READHANDLE(NswPassivationDbData, m_readNswPassivKey);
        writeHandle.addDependency(readHandle);
    }
    if (m_applyNswAsBuilt && isNsw) {
        CREATE_READHANDLE(NswAsBuiltDbData, m_readNswAsBuiltKey);
        writeHandle.addDependency(readHandle);
    }
    return StatusCode::SUCCESS;
}

StatusCode ActsMuonAlignCondAlg::execute(const EventContext& ctx) const {
    deltaMap alignDeltas{};
    alignTechMap techTransforms{};
    unsigned int memBeforeAlign = GeoPerfUtils::getMem();
    ATH_CHECK(loadDeltas(ctx, alignDeltas, techTransforms));
    
    std::vector<const MuonReadoutElement*> readoutEles = m_detMgr->getAllReadoutElements();
    /// Create the condition handles
    unsigned int numAligned{0};
    for (size_t det =0 ; det < m_techs.size(); ++det) {
        const SG::WriteCondHandleKey<ActsTrk::DetectorAlignStore>& key = m_writeKeys[det];
        const ActsTrk::DetectorType subDet = m_techs[det];

        SG::WriteCondHandle<ActsTrk::DetectorAlignStore> writeHandle{key, ctx};
        if (writeHandle.isValid()) {
            ATH_MSG_FATAL("The alignment constants for "<<ActsTrk::to_string(subDet)
                          <<" is still valid. That should not happen at this stage");
            return StatusCode::FAILURE;
        }
        std::unique_ptr<ActsTrk::DetectorAlignStore> writeCdo = std::make_unique<ActsTrk::DetectorAlignStore>(subDet);

        const std::set<const GeoAlignableTransform*>& toStore =  techTransforms[subDet];
        /// Append the alignable transformations to the conditions object
        for (const GeoAlignableTransform* alignable : toStore) {
           const std::shared_ptr<const Amg::Transform3D>& cached = alignDeltas[alignable];
           if (!cached) continue;
           writeCdo->geoModelAlignment->setDelta(alignable, alignDeltas[alignable]);
        }
        if (subDet == ActsTrk::DetectorType::Mdt) {
            ATH_CHECK(loadMdtDeformPars(ctx,*writeCdo));
        } else if (subDet == ActsTrk::DetectorType::Mm) {
            ATH_CHECK(loadMmDeformPars(ctx, *writeCdo));
        } else if (subDet == ActsTrk::DetectorType::sTgc) {
            ATH_CHECK(loadStgcDeformPars(ctx, *writeCdo));
        }
        /// Propagate the cache throughout the geometry
        ATH_CHECK(declareDependencies(ctx, subDet, writeHandle));
        if (m_fillAlignStoreCache) {
            for (const MuonReadoutElement* re : readoutEles){
                numAligned+= re->storeAlignedTransforms(*writeCdo);
            }
            /// The geoModel constants are no longer needed.
            writeCdo->geoModelAlignment.reset();
        }
        ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    }
    /// Check that all readout elements were properly aligned
    if (m_fillAlignStoreCache && numAligned != readoutEles.size()){
        ATH_MSG_FATAL("Only "<<numAligned<<" out of "<<readoutEles.size()<<" were picked up by the alignment cutalg");
        return StatusCode::FAILURE;
    }
    alignDeltas.clear();
    techTransforms.clear();
    /// Whipe the GeoModelCache
    GeoClearAbsPosAction whipeTreeTop{};
    for (unsigned int treeTop = 0 ; treeTop < m_detMgr->getNumTreeTops(); ++treeTop) {
        m_detMgr->getTreeTop(treeTop)->exec(&whipeTreeTop);
    }

    unsigned int memAfterAlign  = GeoPerfUtils::getMem();
    ATH_MSG_INFO("Caching of the alignment parameters required "<<(memAfterAlign - memBeforeAlign) / 1024<<" MB of memory");
    return StatusCode::SUCCESS;
}

#undef CREATE_READHANDLE