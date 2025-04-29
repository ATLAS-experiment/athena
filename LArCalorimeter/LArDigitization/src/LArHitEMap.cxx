/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArDigitization/LArHitEMap.h"
#include <cstdlib>
#include <iostream>

#include "CaloDetDescr/CaloDetDescrElement.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"


//For the buildWindow function
#include "GeneratorObjects/McEventCollection.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "TruthUtils/HepMCHelpers.h"


//bool LArHitEMap::Initialize(std::vector<bool>& flags, bool windows, bool digit)
LArHitEMap::LArHitEMap(const LArOnOffIdMapping* cabling, const CaloCell_ID* cellid, const CaloDetDescrManager* cddMgr, bool digit) :
  m_cabling(cabling),
  m_calocell_id(cellid),
  m_cddMgr(cddMgr) {

  //the last cell of the FCAL is the hash-max for LAr (ignore the Tile part) 
  IdentifierHash fcalCellMin, fcalCellMax;
  cellid->calo_cell_hash_range(CaloCell_ID::LARFCAL,fcalCellMin,fcalCellMax);

  //fill energy map up to fcal-hashmax(= lar-hashmax) 
  m_emap.resize(fcalCellMax);
  if (digit) m_digmap.resize(fcalCellMax,nullptr);
}


// add energy using the calo-cell hash
bool LArHitEMap::AddEnergy(const IdentifierHash index, const float energy, const float time) {
  if(index >= m_emap.size()) return(false);
  m_emap[index].AddHit(energy,time);
  return true; 
}

// add energy using identifier
bool LArHitEMap::AddEnergy(const Identifier cellid, const float energy, const float time) {
  IdentifierHash idHash=m_calocell_id->calo_cell_hash(cellid);
  return AddEnergy(idHash,energy,time);
}

bool LArHitEMap::AddDigit(const LArDigit* digit) {
  const HWIdentifier ch_id = digit->channelID();
  if (m_cabling->isOnlineConnected(ch_id)) {
    Identifier cellid=m_cabling->cnvToIdentifier(ch_id);
    IdentifierHash h=m_calocell_id->calo_cell_hash(cellid);
   
    if (h>=m_digmap.size()) return false ;
    m_digmap[h]=digit;
    return true;
  }
  else 
    return false;
}

int LArHitEMap::GetNbCells(void) const
{
  return m_emap.size()  ;
}

bool LArHitEMap::BuildWindows(const McEventCollection* mcCollptr,
                              float deta,float dphi, float ptmin)
{
// get list of particles
    std::vector<double> phiPart;
    std::vector<double> etaPart;

    etaPart.clear();
    phiPart.clear();

    if (!mcCollptr) {
      return false;
    }

    McEventCollection::const_iterator itr;
//    std::cout << " start loop over particles " << std::endl;
    for (itr = mcCollptr->begin(); itr!=mcCollptr->end(); ++itr) {
      for (const auto& part: *(*itr))
      {
         //works only for photons(22) and electrons(11) primary particle (+pi0 in case not decayed by generator)
         // with pt>5 GeV
         // pickup "stable" particle from generator excluding G4 secondaries
         if(   ( MC::isPhoton(part) || MC::isElectron(part) || part->pdg_id()==111) && part->momentum().perp()> ptmin
             && MC::isStable(part) && !HepMC::is_simulation_particle(part) ) 
         {
          etaPart.push_back(part->momentum().pseudoRapidity());
          phiPart.push_back(part->momentum().phi());
         }
      }
    }


    if ( etaPart.empty()) return true;

    for (unsigned int i=0; i < m_emap.size(); i++) 
    {
     LArHitList& theLArHitList = m_emap[i];
     const CaloDetDescrElement* calodde = m_cddMgr->get_element(IdentifierHash(i));
     double eta=calodde->eta();
     double phi=calodde->phi();
     for(unsigned int iPart=0;iPart<etaPart.size();++iPart)
       {
	 double deltaPhi=fmod(phiPart[iPart]-phi+3.0*M_PI,2.0*M_PI)-M_PI;
	 double deltaEta=etaPart[iPart]-eta;
	 if( std::fabs(deltaPhi)<dphi/2. &&
	     std::fabs(deltaEta)<deta/2. )
	   {
	     theLArHitList.setInWindows(); 
	     break;
	   }
       }  // loop over particles
    }    // loop over cells
  return true;
}
