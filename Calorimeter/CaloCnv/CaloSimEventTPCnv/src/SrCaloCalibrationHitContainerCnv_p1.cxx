/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloSimEventTPCnv/SrCaloCalibrationHitContainerCnv_p1.h"

#include "AthenaPoolCnvSvc/Compressor.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloDM_ID.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloSimEvent/CaloCalibrationHit.h"
#include "CaloSimEvent/SrCaloCalibrationHitContainer.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "map"

void SrCaloCalibrationHitContainerCnv_p1::transToPers(
    const SrCaloCalibrationHitContainer* transCont,
    SrCaloCalibrationHitContainer_p1* persCont, MsgStream& log) {

  size_t size = transCont->size();
  if (log.level() <= MSG::DEBUG)
    log << MSG::DEBUG
        << " ***  Writing SrCaloCalibrationHitContainer_p1 of size: " << size
        << endmsg;

  persCont->m_channelHash.reserve(size);
  std::vector<float> tempE;
  tempE.reserve(size * 4);
  std::vector<unsigned int> tempPID;
  tempPID.reserve(size);
  SrCaloCalibrationHitContainer::const_iterator it = transCont->begin();

  std::multimap<unsigned long long, unsigned int>
      map_hashPositions;  // first hash ; second its position in container

  for (unsigned int w = 0; w < size; ++w) {
    unsigned long long id = (*it)->cellID().get_compact();
    map_hashPositions.insert(std::pair<unsigned long long, int>(id, w));
    ++it;
  }

  std::multimap<unsigned long long, unsigned int>::const_iterator iter;
  unsigned long long old = 0;

  for (iter = map_hashPositions.begin(); iter != map_hashPositions.end();
       ++iter) {
    unsigned long long pHash = (iter->first) - old;
    old = iter->first;
    unsigned int pos = iter->second;
    persCont->m_channelHash.push_back(pHash);
    const CaloCalibrationHit& hit = *transCont->At(pos);
    tempE.push_back(static_cast<float>(hit.energyEM()));
    tempE.push_back(static_cast<float>(hit.energyNonEM()));
    tempE.push_back(static_cast<float>(hit.energyInvisible()));
    tempE.push_back(static_cast<float>(hit.energyEscaped()));
    tempPID.push_back(static_cast<unsigned int>(hit.particleID()));
  }
  // For future development: use Compressor class to reduce size of energy
  // storage ?
  // Compressor A; A.setNrBits(18);
  // A.reduce(tempE,persCont->m_energy); // packs energy
  persCont->m_energy = tempE;            // Store directly without compression
  persCont->m_name = transCont->Name();  // stores name
  persCont->m_particleID = tempPID;
}

void SrCaloCalibrationHitContainerCnv_p1::persToTrans(
    const SrCaloCalibrationHitContainer_p1* persCont,
    SrCaloCalibrationHitContainer* transCont, MsgStream& log) {
  size_t cells = persCont->m_channelHash.size();
  if (log.level() <= MSG::DEBUG)
    log << MSG::DEBUG
        << " ***  Reading SrCaloCalibrationHitContainer of size: " << cells
        << endmsg;
  transCont->clear();
  transCont->reserve(cells);
  transCont->setName(persCont->name());

  // For future development: use Compressor class to reconstruct energy values
  // Compressor A;
  // std::vector<float> tempE;
  const std::vector<float>& tempE = persCont->m_energy;  // Direct reference
  // tempE.reserve(cells*4);
  // A.expandToFloat(persCont->m_energy,tempE);

  unsigned long long sum = 0;
  for (unsigned int i = 0; i < cells; ++i) {
    sum += persCont->m_channelHash[i];
    transCont->push_back(new CaloCalibrationHit(
        static_cast<Identifier>(sum), tempE[i * 4], tempE[i * 4 + 1],
        tempE[i * 4 + 2], tempE[i * 4 + 3],
        static_cast<int>(persCont->m_particleID[i]),
        HepMC::INVALID_PARTICLE_ID));
  }
}
