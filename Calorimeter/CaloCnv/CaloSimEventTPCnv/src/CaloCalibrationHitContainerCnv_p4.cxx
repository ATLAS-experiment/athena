/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloSimEvent/CaloCalibrationHit.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "Identifier/Identifier.h"

#include "Identifier/IdentifierHash.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloDM_ID.h"
#include "CaloIdentifier/CaloIdManager.h"

#include "AthenaPoolCnvSvc/Compressor.h"

// CaloCalibrationHitContainerCnv_p4, used for T/P separation
// author Ilija Vukotic

#include "CaloSimEventTPCnv/CaloCalibrationHitContainerCnv_p4.h"
#include "map"

void CaloCalibrationHitContainerCnv_p4::transToPers(const CaloCalibrationHitContainer* transCont, CaloCalibrationHitContainer_p4* persCont, MsgStream &log)
{

  //    static int ev=0;
  size_t size = transCont->size();
  if (log.level() <= MSG::DEBUG) log << MSG::DEBUG  << " ***  Writing CaloCalibrationHitContainer_p4 of size:"<<size<<endmsg;

  persCont->m_channelHash.reserve(size);
  std::vector<float> tempE;
  tempE.reserve(size*4);
  std::vector<unsigned int> tempPID; tempPID.reserve(size);

  CaloCalibrationHitContainer::const_iterator it  = transCont->begin();

  std::multimap <unsigned int, unsigned int> map_hashPositions;// first hash ; second its position in container

  for (unsigned int w=0;w<size;++w) {

    unsigned int id = (*it)->cellID().get_identifier32().get_compact();
    map_hashPositions.insert(std::pair<unsigned int, int>(id, w));
    ++it;
  }

  std::multimap<unsigned int, unsigned int>::const_iterator iter;
  unsigned int old=0;
  for (iter=map_hashPositions.begin(); iter != map_hashPositions.end(); ++iter) {
    unsigned int pHash=(iter->first)-old; // to store as a difference
    old=iter->first;
    unsigned int pos=iter->second;
    persCont->m_channelHash.push_back(pHash);
    const CaloCalibrationHit& hit = *transCont->At(pos);
    tempE.push_back( static_cast<float>(hit.energyEM()) );
    tempE.push_back( static_cast<float>(hit.energyNonEM()) );
    tempE.push_back( static_cast<float>(hit.energyInvisible()) );
    tempE.push_back( static_cast<float>(hit.energyEscaped()) );
    tempPID.push_back( static_cast<unsigned int>(hit.particleUID()) );
  }

  Compressor A; A.setNrBits(18);
  A.reduce(tempE,persCont->m_energy); // packs energy
  persCont->m_name = transCont->Name(); //stores name
  persCont->m_particleUID = std::move(tempPID);

}



void CaloCalibrationHitContainerCnv_p4::persToTrans(const CaloCalibrationHitContainer_p4* persCont, CaloCalibrationHitContainer* transCont, MsgStream &log)
{
  size_t cells=persCont->m_channelHash.size();
  if (log.level() <= MSG::DEBUG) log << MSG::DEBUG  << " ***  Reading CaloCalibrationHitContainer of size: "<<cells<<endmsg;
  transCont->clear();
  transCont->reserve(cells);
  transCont->setName(persCont->name() );

  Compressor A;
  std::vector<float> tempE;
  tempE.reserve(cells*4);
  A.expandToFloat(persCont->m_energy,tempE);

  unsigned int sum=0;
  for (unsigned int i=0;i<cells;++i) {
    sum+= persCont->m_channelHash[i];

    transCont->push_back
      (new CaloCalibrationHit(static_cast<Identifier>(sum),
                              tempE[i*4],
                              tempE[i*4+1],
                              tempE[i*4+2],
                              tempE[i*4+3],
                              HepMC::INVALID_PARTICLE_ID,
                              static_cast<int>(persCont->m_particleUID[i])));
  }
}
