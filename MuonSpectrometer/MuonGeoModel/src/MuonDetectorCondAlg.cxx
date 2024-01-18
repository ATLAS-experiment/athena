/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModel/MuonDetectorCondAlg.h"

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/ClassID_traits.h"
#include "AthenaKernel/CondCont.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonDetDescrUtils/BuildNSWReadoutGeometry.h"
#include "MuonGeoModel/MuonDetectorFactory001.h"
#include "MuonGeoModel/MuonDetectorTool.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "GeoModelKernel/GeoVolumeCursor.h"
#include <fstream>

MuonDetectorCondAlg::MuonDetectorCondAlg(const std::string &name, ISvcLocator *pSvcLocator) : 
    AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MuonDetectorCondAlg::initialize() {
    ATH_MSG_DEBUG("Initializing ...");
    // Read Handles
    ATH_CHECK(m_iGeoModelTool.retrieve());

    ATH_CHECK(m_readALineKey.initialize(m_applyALines));
    ATH_CHECK(m_readBLineKey.initialize(m_applyBLines));
    ATH_CHECK(m_readILineKey.initialize(m_applyILines));
    ATH_CHECK(m_readMdtAsBuiltKey.initialize(m_applyMdtAsBuilt));
    ATH_CHECK(m_readNswAsBuiltKey.initialize(m_applyNswAsBuilt));
    ATH_CHECK(m_condMmPassivKey.initialize(m_applyMmPassivation));
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_writeDetectorManagerKey.initialize());
    ATH_MSG_INFO("Initialize successful -- "<<m_applyALines<<", "<<m_applyBLines<<","
                                            <<m_applyILines<<","<<m_applyMdtAsBuilt<<","
                                            <<m_applyNswAsBuilt<<","<<m_applyMmPassivation);
    return StatusCode::SUCCESS;
}

StatusCode MuonDetectorCondAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());
    
    // =======================
    // Conditions handle
    // =======================
    SG::WriteCondHandle<MuonGM::MuonDetectorManager> writeHandle{m_writeDetectorManagerKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    writeHandle.addDependency(IOVInfiniteRange::infiniteRunLB());

    // =======================
    // Create the MuonDetectorManager by calling the MuonDetectorFactory001
    // =======================
    MuonGM::MuonDetectorFactory001 theFactory(detStore().operator->());
    MuonGM::MuonDetectorManager *mgr{nullptr};
    if (m_iGeoModelTool->createFactory(mgr).isFailure()) {
        ATH_MSG_FATAL("unable to create MuonDetectorFactory001 ");
        return StatusCode::FAILURE;
    }
    std::unique_ptr<MuonGM::MuonDetectorManager> MuonMgrData(mgr);
   
    // =======================
    // Add NSW to the MuonDetectorManager by calling BuildReadoutGeometry from MuonAGDDToolHelper
    // =======================
    
    if (MuonMgrData->mmIdHelper() && MuonMgrData->stgcIdHelper()) {
        BuildNSWReadoutGeometry theBuilder{};
        bool success=false;
        if(m_applyMmPassivation){           
            SG::ReadCondHandle<NswPassivationDbData> readMmPass{m_condMmPassivKey, ctx};
            if(!readMmPass.isValid()){
              ATH_MSG_ERROR("Cannot find conditions data container for MM passivation!");
              return StatusCode::FAILURE;
            }
            writeHandle.addDependency(readMmPass);
            success = theBuilder.BuildReadoutGeometry(MuonMgrData.get(), readMmPass.cptr());
        }
        else {
            success = theBuilder.BuildReadoutGeometry(MuonMgrData.get(), nullptr);
        }
        if(!success){
            ATH_MSG_FATAL("unable to add NSW ReadoutGeometry in the MuonDetectorManager in conditions store");
            return StatusCode::FAILURE;
        }
    }

    // =======================
    // Update CSC Internal Alignment if requested
    // =======================

    if (!m_readILineKey.empty()) {
        SG::ReadCondHandle<ALineContainer> readILinesHandle{m_readILineKey, ctx};
        if (!readILinesHandle.isValid()){
            ATH_MSG_FATAL("Failed to retrieve the CSC I-line container "<<readILinesHandle.fullKey());
            return StatusCode::FAILURE;
        }        
        writeHandle.addDependency(readILinesHandle);
        ATH_CHECK(MuonMgrData->updateCSCInternalAlignmentMap(**readILinesHandle));
    }

    // =======================
    // Update MdtAsBuiltMapContainer if requested BEFORE updating ALINES and BLINES
    // =======================
    if (!m_readMdtAsBuiltKey.empty()) {
        SG::ReadCondHandle<MdtAsBuiltContainer> readMdtAsBuiltHandle{m_readMdtAsBuiltKey, ctx};
        if (!readMdtAsBuiltHandle.isValid()) {
            ATH_MSG_FATAL("Failed to load Mdt as-built container "<<m_readMdtAsBuiltKey.fullKey());
            return StatusCode::FAILURE;
        }
        writeHandle.addDependency(readMdtAsBuiltHandle);
        ATH_CHECK(MuonMgrData->updateMdtAsBuiltParams(**readMdtAsBuiltHandle));
    }

    // =======================
    // Set NSW as-built geometry if requested
    // =======================
    if (!m_readNswAsBuiltKey.empty()) {
        SG::ReadCondHandle<NswAsBuiltDbData> readNswAsBuilt{m_readNswAsBuiltKey, ctx};
        if(!readNswAsBuilt.isValid()) {
            ATH_MSG_ERROR("Cannot find conditions data container for NSW as-built!");
            return StatusCode::FAILURE;
        }
        writeHandle.addDependency(readNswAsBuilt);
        MuonMgrData->setNswAsBuilt(*readNswAsBuilt); 
    }

    // =======================
    // Update Alignment, ALINES
    // =======================
    if (m_applyALines) {
        SG::ReadCondHandle<ALineContainer> readALinesHandle{m_readALineKey, ctx};
        if (!readALinesHandle.isValid()) {
            ATH_MSG_FATAL("Failed to load ALine container "<<m_readALineKey.fullKey());
            return StatusCode::FAILURE;
        }
        writeHandle.addDependency(readALinesHandle);
        ATH_CHECK(MuonMgrData->updateAlignment(**readALinesHandle));     
    } else ATH_MSG_INFO("Do not apply the A Lines of the alignment");
 
    // =======================
    // Update Deformations, BLINES
    // =======================
    if (m_applyBLines) {
        SG::ReadCondHandle<BLineContainer> readBLinesHandle{m_readBLineKey, ctx};
        if (!readBLinesHandle.isValid()) {
            ATH_MSG_FATAL("Failed to load B line container "<<m_readBLineKey.fullKey());
            return StatusCode::FAILURE;
        }
        writeHandle.addDependency(readBLinesHandle);
        ATH_CHECK (MuonMgrData->updateDeformations(**readBLinesHandle));  
    } else ATH_MSG_INFO("Do not apply the B Lines of the alignment");
    
    // !!!!!!!! UPDATE ANYTHING ELSE ???????
    ATH_CHECK(copyInertMaterial(*MuonMgrData));
    ATH_CHECK(writeHandle.record(std::move(MuonMgrData)));
    ATH_MSG_INFO("recorded new " << writeHandle.key() << " with range " << writeHandle.getRange() << " into Conditions Store");

    return StatusCode::SUCCESS;
}
StatusCode MuonDetectorCondAlg::copyInertMaterial(MuonGM::MuonDetectorManager& detMgr) const {
    const MuonGM::MuonDetectorManager *MuonDetMgrDS{nullptr};
    ATH_CHECK(detStore()->retrieve(MuonDetMgrDS));

    // In the new geomodel, there is only one GeoModel tree and it is pre-built. In that
    // case these pointers are the same and one should not copy to self. 
    if (MuonDetMgrDS->getTreeTop(0)==detMgr.getTreeTop(0)) return StatusCode::SUCCESS;
    // ---------------------------------------------------------------------------------

    PVLink condMgrWorld{detMgr.getTreeTop(0)};
       
    GeoVolumeCursor detStoreCursor{MuonDetMgrDS->getTreeTop(0)};
    while (!detStoreCursor.atEnd()) {
        PVConstLink worldNode(detStoreCursor.getVolume());
        const Amg::Transform3D transform{detStoreCursor.getTransform()};
        const GeoLogVol* logVol = worldNode->getLogVol();
        const std::string_view vname = logVol->getName();
        detStoreCursor.next();
        if (vname.find("Station") != std::string::npos) continue;
        /// All operations are atomic. So it's safe to cast constness away
        GeoVPhysVol* physVol ATLAS_THREAD_SAFE = const_cast<GeoVPhysVol*>(worldNode.operator->()) ;
        const GeoVPhysVol& pvConstLink = *worldNode;
        ATH_MSG_DEBUG("Volume in the static world "<<vname<<" "<<typeid(pvConstLink).name()
                        <<"children: "<<worldNode->getNChildNodes()
                        <<" getDefX(): "<<Amg::toString(worldNode->getDefX())
                        <<" getX(): "<<Amg::toString(worldNode->getX())
                        <<" cursor: "<<Amg::toString(transform));        
        condMgrWorld->add(new GeoTransform(transform));
        condMgrWorld->add(physVol);
    }
    return StatusCode::SUCCESS;
}


