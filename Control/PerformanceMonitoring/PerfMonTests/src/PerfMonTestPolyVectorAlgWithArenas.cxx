/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "AthContainers/DataVector.h"
#include "AthAllocators/DataPool.h"    /* NEW */

#include "Hit.h"
#include "PerfMonTestPolyVectorAlgWithArenas.h"
/* #define DEBUG_ME 1 */

using namespace PerfMonTest;

typedef DataVector<IHit> HitPtrContainer;


StatusCode PolyVectorAlgWithArenas::execute()
{  
  DataPool<DHit> dhitPool;
  DataPool<FHit> fhitPool;
  ATH_MSG_DEBUG("Executing ");
  HitPtrContainer vptr(SG::VIEW_ELEMENTS);  //<<< NEW
  vptr.reserve(m_2bReserved.value());
  int vcap(vptr.capacity());
  int vold;
#ifdef DEBUG_ME
  IHit* p1(0);
  IHit* p2(0);
#endif
  IHit* p3(0);
  std::cout << "initial capacity " << vcap << std::endl;
  int size(m_vectorSize.value());
  for(int i(0); i<size; ++i) {
    vold=vcap;
#ifdef DEBUG_ME
    p1=p2;
    p2=p3;
#endif
    p3=(i % m_mixture.value()) ? 
      (IHit*) new(fhitPool.nextElementPtr()) FHit(i, i, i): 
      (IHit*) new(dhitPool.nextElementPtr()) DHit(i, i, i);   //<<< NEW
    vptr.push_back(p3);
    vcap=vptr.capacity();
    if (m_mapIt.value()) m_mixMap[i]=p3;
    if (vold != vcap) std::cout << "iteration " << i << " new capacity " << vcap <<std::endl;
#ifdef DEBUG_ME
    if (((int)p3-(int)p2) != ((int)p2-(int)p1)) std::cout << "iteration " << i << " new chunk @" << hex << p3 << " previous was @" << p2 << dec << std::endl;
#ifdef REALLY_DEBUG_ME
    std::cout << "iteration " << i << " P3 @" << hex << p3 << " p2 @" << p2 << " p1 @" << p1 << dec << ' ' << (int)p3-(int)p2 << ' ' <<(int)p2-(int)p1 <<std::endl;
#endif
#endif
  }

  return StatusCode::SUCCESS;
}
