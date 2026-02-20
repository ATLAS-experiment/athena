/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef TRIGBPHYSICSEVENT_TRIGBPHYSICSEVENTDICT
#define TRIGBPHYSICSEVENT_TRIGBPHYSICSEVENTDICT

#include "TrigBphysicsEvent/TrigJpsi.h"
#include "TrigBphysicsEvent/TrigJpsiContainer.h"
#include "AthContainers/DataVector.h"
//coverity[pass_by_value]
void dummyTriggerForTrigPartTypedefs(TrigJpsiContainer& a, DataVector<TrigJpsi> b){
	TrigJpsiContainer aa = a;
	DataVector<TrigJpsi> bb = std::move(b);
}

template class std::vector<TrigJpsi*>;

#endif // TRIGBPHYSICSEVENT_TRIGBPHYSICSEVENTDICT 
