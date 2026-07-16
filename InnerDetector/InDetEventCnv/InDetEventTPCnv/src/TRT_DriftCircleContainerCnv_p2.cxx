/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetPrepRawData/TRT_DriftCircle.h"
#include "InDetEventTPCnv/InDetPrepRawData/TRT_DriftCircle_p2.h"
#include "InDetEventTPCnv/TRT_DriftCircleContainer_p2.h"
#include "InDetEventTPCnv/InDetPrepRawData/InDetPRD_Collection_p2.h"
#include "InDetPrepRawData/TRT_DriftCircleContainer.h"

#include "Identifier/Identifier.h"
#include "InDetIdentifier/TRT_ID.h"
#include "InDetEventTPCnv/InDetPrepRawData/TRT_DriftCircleCnv_p2.h"
#include "InDetEventTPCnv/TRT_DriftCircleContainerCnv_p2.h"
#include "AthAllocators/DataPool.h"

// Gaudi
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/MsgStream.h"

// Athena
#include "StoreGate/StoreGateSvc.h"
#include "AthenaKernel/errorcheck.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/ReadCondHandle.h"

// #define IDJUMP 0x400


void TRT_DriftCircleContainerCnv_p2::transToPers(const InDet::TRT_DriftCircleContainer* transCont, InDet::TRT_DriftCircleContainer_p2* persCont, MsgStream &log) 
{

    // The transient model has a container holding collections and the
    // collections hold channels.
    //
    // The persistent model flattens this so that the persistent
    // container has two vectors:
    //   1) all collections, and
    //   2) all PRD
    //
    // The persistent collections, then only maintain indexes into the
    // container's vector of all channels. 
    //
    // So here we loop over all collection and add their channels
    // to the container's vector, saving the indexes in the
    // collection. 

    using TRANS = InDet::TRT_DriftCircleContainer;
        
    // this is the id of the latest collection read in
    // Thisstarts from the base of the TRT identifiers
    unsigned int idLast(0);

    // 
    TRT_DriftCircleCnv_p2  chanCnv;
    TRANS::const_iterator it_Coll     = transCont->begin();
    TRANS::const_iterator it_CollEnd  = transCont->end();
    unsigned int collIndex;
    unsigned int chanBegin = 0;
    unsigned int chanEnd = 0;

    persCont->m_collections.resize(transCont->numberOfCollections());

    // to avoid the inside-loop resize
    int totSize = 0; 
    //for ( ; it_Coll != it_CollEnd;  it_Coll++)  {
    for ( it_Coll=transCont->begin(); it_Coll != it_CollEnd;  ++it_Coll)  {
      const InDet::TRT_DriftCircleCollection& collection = (**it_Coll);
      totSize+=collection.size();
    }
    persCont->m_rawdata.resize(totSize);
    persCont->m_prdDeltaId.resize(totSize);

    for (collIndex = 0, it_Coll=transCont->begin(); it_Coll != it_CollEnd; ++collIndex, ++it_Coll)  {
        // Add in new collection
        const InDet::TRT_DriftCircleCollection& collection = (**it_Coll);
        chanBegin  = chanEnd;
        chanEnd   += collection.size();
        InDet::InDetPRD_Collection_p2& pcollection = persCont->m_collections[collIndex];
	unsigned int deltaId = (collection.identifyHash()-idLast);
       
        pcollection.m_hashId = deltaId;
	idLast = collection.identifyHash();
        pcollection.m_size = collection.size();
        // Add in channels
        
        for (unsigned int i = 0; i < collection.size(); ++i) {
            InDet::TRT_DriftCircle_p2* pchan = &(persCont->m_rawdata[i + chanBegin]);
            const InDet::TRT_DriftCircle* chan = static_cast<const InDet::TRT_DriftCircle*>(collection[i]);
            chanCnv.transToPers(chan, pchan, log);
	    persCont->m_prdDeltaId[i+chanBegin]=chan->identify().get_identifier32().get_compact()-collection.identify().get_identifier32().get_compact();
	}
    }
}

void  TRT_DriftCircleContainerCnv_p2::persToTrans(const InDet::TRT_DriftCircleContainer_p2* persCont, InDet::TRT_DriftCircleContainer* transCont, MsgStream &log) 
{

    // The transient model has a container holding collections and the
    // collections hold channels.
    //
    // The persistent model flattens this so that the persistent
    // container has two vectors:
    //   1) all collections, and
    //   2) all channels
    //
    // The persistent collections, then only maintain indexes into the
    // container's vector of all channels. 
    //
    // So here we loop over all collection and extract their channels
    // from the vector.

    const InDetDD::TRT_DetElementCollection* elements(nullptr);
    if (m_useDetectorElement) {
      SG::ReadCondHandle<InDetDD::TRT_DetElementContainer> trtDetEleHandle(m_trtDetEleContKey);
      elements = (*trtDetEleHandle)->getElements();
      if (not trtDetEleHandle.isValid() or elements==nullptr) {
        log << MSG::FATAL << m_trtDetEleContKey.fullKey() << " is not available." << endmsg;
        return;
      }
    }

    InDet::TRT_DriftCircleCollection* coll = nullptr;

    TRT_DriftCircleCnv_p2  chanCnv;
    unsigned int collBegin(0);
    // this is the id of the latest collection read in
    // This starts from the base of the TRT identifiers
    unsigned int idLast(0);
    //    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << " Reading " << persCont->m_collections.size() << "Collections" << endmsg;
    for (unsigned int icoll = 0; icoll < persCont->m_collections.size(); ++icoll) {

        // Create trans collection - in NOT owner of TRT_DriftCircle (SG::VIEW_ELEMENTS)
	// IDet collection don't have the Ownership policy c'tor
        const InDet::InDetPRD_Collection_p2& pcoll = persCont->m_collections[icoll];        
	//        idLast+= pcoll.m_idDelta*IDJUMP;
        // Identifier collID= Identifier(idLast);
	idLast += pcoll.m_hashId;
        IdentifierHash collIDHash=IdentifierHash((unsigned int) idLast);
        Identifier collID = m_trtId->layer_id(collIDHash);
        coll = new InDet::TRT_DriftCircleCollection(collIDHash);
        coll->setIdentifier(collID);
        unsigned int nchans           = pcoll.m_size;
        coll->resize(nchans);
        const InDetDD::TRT_BaseElement* detEl = (elements==nullptr ? nullptr : elements->getDetectorElement(collIDHash));
        // Fill with channels:
        // This is used to read the vector of errMat
        // values and lenght of the value are specified in separate vectors
	//	if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Reading collection with " << nchans << "Channels " << endmsg;
        for (unsigned int ichan = 0; ichan < nchans; ++ ichan) {
            const InDet::TRT_DriftCircle_p2* pchan = &(persCont->m_rawdata[ichan + collBegin]);
            Identifier clusId=Identifier(collID.get_identifier32().get_compact()+persCont->m_prdDeltaId[ichan + collBegin]);

            std::vector<Identifier> rdoList(1);
            rdoList[0]=clusId;

            InDet::TRT_DriftCircle* chan = new InDet::TRT_DriftCircle
              (chanCnv.createTRT_DriftCircle (pchan,
                                              clusId,
                                              std::move(rdoList),
                                              detEl,
                                              log));

            // DC bugfix: set hash Id of the drift circle
	    chan->setHashAndIndex(collIDHash,ichan);
            (*coll)[ichan] = chan;
        }
        collBegin += pcoll.m_size;

        // register the PRD collection in IDC with hash - faster addCollection
        StatusCode sc = transCont->addCollection(coll, collIDHash);
        if (sc.isFailure()) {
            throw std::runtime_error("Failed to add collection to ID Container");
        }
	//	if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "AthenaPoolTPCnvIDCont::persToTrans, collection, hash_id/coll id = " << (int) collIDHash << " / " << collID << ", added to Identifiable container." << endmsg;
    }

    //     if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << " ***  Reading InDet::TRT_DriftCircleContainer" << endmsg;
}



//================================================================
InDet::TRT_DriftCircleContainer* TRT_DriftCircleContainerCnv_p2::createTransient(const InDet::TRT_DriftCircleContainer_p2* persObj, MsgStream& log) {
  //  if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "TRT_DriftCircleContainerCnv_p2::createTransient called " << endmsg;
    if(!m_isInitialized) {
     if (this->initialize(log) != StatusCode::SUCCESS) {
      log << MSG::FATAL << "Could not initialize TRT_DriftCircleContainerCnv_p2 " << endmsg;
      return nullptr; // if m_trtId not initialized return null pointer instead of dereferencing it later
     }
    }
    std::unique_ptr<InDet::TRT_DriftCircleContainer> trans(std::make_unique<InDet::TRT_DriftCircleContainer>(m_trtId->straw_layer_hash_max()));
    persToTrans(persObj, trans.get(), log);
    return(trans.release());
}


StatusCode TRT_DriftCircleContainerCnv_p2::initialize(MsgStream&) {


   // Do not initialize again:
   m_isInitialized=true;

   // Get Storegate, ID helpers, and so on
   SmartIF<StoreGateSvc> detStore{Gaudi::svcLocator()->service("DetectorStore")};
   CHECK( detStore.isValid() );

   // Get the TRT helper from the detector store
   CHECK( detStore->retrieve(m_trtId, "TRT_ID") );

   CHECK(m_trtDetEleContKey.initialize(m_useDetectorElement));

   return StatusCode::SUCCESS;
}

// Methods for test/TRT_DriftCircleContainerCnv_p2_test.cxx                                                                      
void TRT_DriftCircleContainerCnv_p2::setIdHelper(const TRT_ID* trt_id) {
  m_trtId = trt_id;
}

void TRT_DriftCircleContainerCnv_p2::setUseDetectorElement(const bool useDetectorElement) {
  m_useDetectorElement = useDetectorElement;
}

