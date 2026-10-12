// -*- C++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


/**

Description: Needed to provide a persistent TrigElectron class in 
             TrigElectronContainer (which is derived from DataVector)
  
Authour: R.Goncalo - Thu Sep 20 18:01:20 BT 2005

*/

#ifndef TRIG_PARTICLE_DICT
#define TRIG_PARTICLE_DICT

#include "TrigParticle/TrigElectron.h"
#include "TrigParticle/TrigElectronContainer.h"


#include "TrigParticle/TrigPhoton.h"
#include "TrigParticle/TrigPhotonContainer.h"

#include "TrigParticle/TrigTau.h"
#include "TrigParticle/TrigTauContainer.h"

#include "TrigParticle/TrigL2Bphys.h"
#include "TrigParticle/TrigL2BphysContainer.h"

#include "TrigParticle/TrigEFBphys.h"
#include "TrigParticle/TrigEFBphysContainer.h"


//the function triggers loading dictionaries for typedefs
void dummyTriggerForTrigPartTypedefs(TrigElectronContainer b,
				     TrigL2BphysContainer c,
				     TrigPhotonContainer d,
				     TrigTauContainer e,
				     TrigEFBphysContainer g,
				     DataVector<TrigElectronContainer> i,
				     DataVector<TrigL2BphysContainer> j,
				     DataVector<TrigPhotonContainer> k,
				     DataVector<TrigTauContainer> l,
				     DataVector<TrigEFBphysContainer> n
				     ){

  //to avoid warnings about unused variables
  TrigElectronContainer bb=b;
  TrigL2BphysContainer  cc=c; 
  TrigPhotonContainer   dd=d;
  TrigTauContainer      ee=e;   
  TrigEFBphysContainer  gg=g;
  DataVector<TrigElectronContainer> ii = i;
  DataVector<TrigL2BphysContainer>  jj = j;
  DataVector<TrigPhotonContainer>   kk = k;
  DataVector<TrigTauContainer>      ll = l;
  DataVector<TrigEFBphysContainer>  nn = n;
}

template class std::vector<const TrigElectron*>;
template class std::vector<const TrigPhoton*>;
template class std::vector<const TrigTau*>;

#endif // TRIG_PARTICLE_DICT
