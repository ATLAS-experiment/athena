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


//the function triggers loading dictionaries for typedefs
void dummyTriggerForTrigPartTypedefs(TrigElectronContainer b,
				     TrigPhotonContainer d,
				     DataVector<TrigElectronContainer> i,
				     DataVector<TrigPhotonContainer> k
				     ){

  //to avoid warnings about unused variables
  TrigElectronContainer bb=b;
  TrigPhotonContainer   dd=d;
  DataVector<TrigElectronContainer> ii = i;
  DataVector<TrigPhotonContainer>   kk = k;
}

template class std::vector<const TrigElectron*>;
template class std::vector<const TrigPhoton*>;

#endif // TRIG_PARTICLE_DICT
