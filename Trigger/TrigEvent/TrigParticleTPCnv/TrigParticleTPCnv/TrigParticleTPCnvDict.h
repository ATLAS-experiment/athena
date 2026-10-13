/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGPARTICLETPCNV_TRIGPARTICLETPCNVDICT_H
#define TRIGPARTICLETPCNV_TRIGPARTICLETPCNVDICT_H

/* TrigElectron T/P classes: persistent version 1*/
#include "TrigParticleTPCnv/TrigElectronContainer_tlp1.h"
#include "TrigParticleTPCnv/TrigElectronContainer_p1.h"
#include "TrigParticleTPCnv/TrigElectron_p1.h"
/* TrigElectron T/P classes: persistent version 2*/
#include "TrigParticleTPCnv/TrigElectronContainer_tlp2.h"
#include "TrigParticleTPCnv/TrigElectronContainer_p2.h"
#include "TrigParticleTPCnv/TrigElectron_p2.h"
/* TrigElectron T/P classes: persistent version 3*/
#include "TrigParticleTPCnv/TrigElectronContainer_p3.h"
#include "TrigParticleTPCnv/TrigElectron_p3.h"

//TrigPhoton p1
#include "TrigParticleTPCnv/TrigPhotonContainer_tlp1.h"
#include "TrigParticleTPCnv/TrigPhotonContainer_p1.h"
#include "TrigParticleTPCnv/TrigPhoton_p1.h"

//TrigPhoton p2
#include "TrigParticleTPCnv/TrigPhotonContainer_tlp2.h"
#include "TrigParticleTPCnv/TrigPhotonContainer_p2.h"
#include "TrigParticleTPCnv/TrigPhoton_p2.h"

//TrigPhoton p3
#include "TrigParticleTPCnv/TrigPhotonContainer_p3.h"
#include "TrigParticleTPCnv/TrigPhoton_p3.h"

//dummy instances of all objects using std::vector<TPObjRef> for the dictionary
struct dummy_TrigParticleTPCnvDict
{

  std::vector<TrigElectron_p1>                m_dummyTrigElectron_p1;
  std::vector<TrigElectron_p2>                m_dummyTrigElectron_p2;
  std::vector<TrigPhoton_p1> 		      m_dummyTrigPhoton_p1;
  std::vector<TrigPhoton_p2> 		      m_dummyTrigPhoton_p2;
  std::vector<TrigPhoton_p3> 		      m_dummyTrigPhoton_p3;

};

#endif
