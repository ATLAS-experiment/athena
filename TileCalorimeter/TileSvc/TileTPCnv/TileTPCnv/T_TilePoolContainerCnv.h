///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// T_TilePoolContainerCnv.h 
// Base class for Tile converters
// Author: Alexander Solodkov <Sanya.Solodkov@cern.ch>
// Date:   June 2009
/////////////////////////////////////////////////////////////////// 
#ifndef TILETPCNV_T_TILEPOOLCONTAINERCNV_H
#define TILETPCNV_T_TILEPOOLCONTAINERCNV_H

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
#include "EventContainers/SelectAllObject.h"
#include "Identifier/IdentifierHash.h"
#include "TileEvent/TileRawChannelCollection.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileEvent/TileRawDataContainer.h"
#include "TileEvent/TileMutableDataContainer.h"
#include "TileIdentifier/TileFragHash.h"

#include <GaudiKernel/IMessageSvc.h>
#include <GaudiKernel/MsgStream.h>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <inttypes.h>

template<class TRANS, class PERS, class CONV>
class T_TilePoolContainerCnv : public T_AthenaPoolTPCnvConstBase<TRANS, PERS> {
public:
  using T_AthenaPoolTPCnvConstBase<TRANS, PERS>::transToPers;
  using T_AthenaPoolTPCnvConstBase<TRANS, PERS>::persToTrans;

  typedef typename PERS::ElemVector pers_ElemVector;
  typedef typename PERS::const_iterator pers_const_iterator;
  typedef typename SelectAllObject<TRANS>::const_iterator trans_const_iterator;
  using Collection = typename TRANS::IDENTIFIABLE;

  T_TilePoolContainerCnv() : m_elementCnv()  {}

  /** Converts vector of PERS::value_type objects to vector of TRANS::value_type objects,
      using converter CONV
      @param persVect [IN] vector of persistent objects
      @param transVect [IN] vector of transient object
      @param log [IN] output message stream
  */
  virtual void persToTrans(const PERS* pers, TRANS* trans, MsgStream &log) const override
  {

    const std::vector<unsigned int>& param = pers->getParam();
    const pers_ElemVector& vec = pers->getVector();

    unsigned int pers_type = (param.size()>0) ? param[0] : 0;
    int hashType = pers_type    & 0xF;
    int type = (pers_type >> 4) & 0xF;
    int unit = (pers_type >> 8) & 0xF;
    uint32_t bsflags = pers_type & 0xFFFFF000;
    if (hashType == 0xF) hashType = TileFragHash::Beam;
    if (type == 0xF) type = TileFragHash::Beam;

    log << MSG::DEBUG << MSG::hex << "pers_type= 0x" << pers_type 
        << " - " <<  bsflags << " " << unit << " " << type << " " << hashType
        << MSG::dec << " Nelements= " << vec.size() << endmsg;

    trans->cleanup(); // remove all collections

    if ( trans->get_hashType() != hashType ) {
      log << MSG::DEBUG << "Pers hash type " << hashType
          << " does not match Trans hash type " << trans->get_hashType()
          << " ==> reinitializing hash " << endmsg;

      trans->initialize(false,(TileFragHash::TYPE)hashType); 
    }

    trans->set_unit((TileRawChannelUnit::UNIT)unit);
    trans->set_type((TileFragHash::TYPE)type);
    trans->set_bsflags(bsflags);


    auto mutableContainer = std::make_unique<TileMutableDataContainer<TRANS>>();
    if (mutableContainer->status().isFailure()) {
      throw std::runtime_error("Failed to initialize Tile mutable Container");
    }

    for( pers_const_iterator it = vec.begin(), iEnd = vec.end(); it != iEnd; ++it) {
      if (mutableContainer->push_back(m_elementCnv.createTransientConst(&(*it), log)).isFailure()) {
        throw std::runtime_error("Failed to add Tile element to Collection");
      }
    }

    std::vector<IdentifierHash> hashes = mutableContainer->GetAllCurrentHashes();

    if constexpr (std::is_same_v<TRANS, TileRawChannelContainer> ) {
      int paramSize = param.size();
      if (paramSize > 1) {
        int firstHashPosition = 1;
        unsigned int goodBCID = 0xDEAD;
        if (((paramSize - 2) % 5) == 0) {
          // Not default BCID is saved
          goodBCID = param[1];
          firstHashPosition = 2;
        }
        if (goodBCID != 0xDEAD) {
          for (const IdentifierHash& hash : hashes) {
            TileRawChannelCollection* rawChannelCollection = mutableContainer->indexFindPtr(hash);
            rawChannelCollection->setFragDSPBCID(goodBCID);
          }
        }
        // 5 elements per module (collection hash, DSP BCID, BCID, global CRC, memory parity)
        int lastHashPosition = paramSize - 5;
        for (int i = firstHashPosition; i <= lastHashPosition; ++i) {
          TileRawChannelCollection* rawChannelCollection =  mutableContainer->indexFindPtr(param[i]);
          if (!rawChannelCollection) {
            i += 4;
            continue;
          }
          rawChannelCollection->setFragDSPBCID(param[++i]);
          rawChannelCollection->setFragBCID(param[++i]);
          rawChannelCollection->setFragGlobalCRC(param[++i]);
          rawChannelCollection->setFragMemoryPar(param[++i]);
        }
      }
    }

    for (const IdentifierHash& hash : hashes) {
      Collection* coll = mutableContainer->indexFindPtr(hash);
      auto newColl = std::make_unique<Collection>(std::move(*coll));
      if (trans->addOrDelete(std::move(newColl), hash).isFailure()) {
        throw std::runtime_error("Failed to add Tile collection to Identifiable Container");
      }
    }

  }
  
  /** Converts vector of TRANS::value_type objects to vector of PERS::value_type objects,
      using converter CONV
      @param transVect [IN] vector of transient object
      @param persVect [IN] vector of persistent objects
      @param log [IN] output message stream
  */
  virtual void transToPers(const TRANS* trans, PERS* pers, MsgStream &log) const override
  {

    pers->clear();
    pers->reserve(1, 12288);

    unsigned int pers_type = ((trans->get_hashType() & 0xF) |
                             ((trans->get_type() & 0xF)<<4) |
                             ((trans->get_unit() & 0xF)<<8) |
                             (trans->get_bsflags() & 0xFFFFF000) ) ; 
    pers->push_back_param(pers_type);

    if constexpr (std::is_same_v<TRANS, TileRawChannelContainer> ) {
      if ((trans->get_type() == TileFragHash::OptFilterDsp) && !trans->empty()) {

        std::unordered_map<unsigned int, int> bcidFreequency;
        std::vector<IdentifierHash> hashes = trans->GetAllCurrentHashes();
        for (const IdentifierHash& hash : hashes) {
          const TileRawChannelCollection* rawChannelCollection = trans->indexFindPtr(hash);
          ++bcidFreequency[rawChannelCollection->getFragDSPBCID()];
        }

        unsigned int goodBCID = (*std::max_element(bcidFreequency.begin(), bcidFreequency.end(),
                                                 [] (const auto& bcidAndFreequency1, const auto& bcidAndFreequency2) {
                                                   return bcidAndFreequency1.second < bcidAndFreequency2.second;
                                                 })).first;

        if (goodBCID != 0xDEAD) { // Do not store default BCID
          pers->push_back_param(goodBCID);
        }

        for (const IdentifierHash& hash : hashes) {
          const TileRawChannelCollection* rawChannelCollection = trans->indexFindPtr(hash);

          int frag = rawChannelCollection->identify();
          bool extendedBarrel = (frag > 0x2ff);
          bool specialModule = (frag == 0x30e || frag == 0x411); // EBA15 or EBC18

          unsigned int existingDMUmask = (~(specialModule ? 0xC301 : (extendedBarrel ? 0xC300 : 0))) & 0xFFFF;

          unsigned int feDMUmask = rawChannelCollection->getFragFEChipMask();
          if (extendedBarrel) { // EBA or EBC
            if (specialModule)  feDMUmask <<= 1; // Shift by one DMU in EBA15 EBC18
            feDMUmask = (feDMUmask & 0xFF) | ((feDMUmask & 0xF00) << 2); // Shift upper half by two DMUs
          }

          // Pack all other errors into fragment memory parity
          unsigned int fragMemoryParity = ((rawChannelCollection->getFragMemoryPar()
                                            | rawChannelCollection->getFragHeaderBit()
                                            | rawChannelCollection->getFragHeaderPar()
                                            | rawChannelCollection->getFragSampleBit()
                                            | rawChannelCollection->getFragSamplePar()
                                            | ((feDMUmask & rawChannelCollection->getFragRODChipMask()) ^ 0xFFFF)) // These masks are reverted
                                           & existingDMUmask);

          unsigned int fragBCID = rawChannelCollection->getFragBCID() & existingDMUmask;
          if (rawChannelCollection->getFragDSPBCID() != goodBCID
              || fragBCID
              || rawChannelCollection->getFragGlobalCRC()
              || fragMemoryParity) {

            pers->push_back_param(hash);
            pers->push_back_param(rawChannelCollection->getFragDSPBCID());
            pers->push_back_param(rawChannelCollection->getFragBCID() & existingDMUmask);
            pers->push_back_param(rawChannelCollection->getFragGlobalCRC());
            pers->push_back_param(fragMemoryParity);
          }
        }
      }
    }

    SelectAllObject<TRANS> selAll(trans); 
    for(trans_const_iterator it = selAll.begin(),
                           iEnd = selAll.end();
        it != iEnd; ++it) {
      m_elementCnv.transToPers((*it), pers->newElem(), log);
    }

    log << MSG::DEBUG << MSG::hex << "pers_type= 0x" << pers_type
        << " - " << (trans->get_bsflags()>>12)
        << " " << trans->get_unit() 
        << " " << trans->get_type() 
        << " " << trans->get_hashType()
        << MSG::dec << "  Nelements= " << pers->getVector().size() << endmsg;
  }

private:
  /// TP converter used for vector elements
  CONV 	m_elementCnv;	
};

#endif
