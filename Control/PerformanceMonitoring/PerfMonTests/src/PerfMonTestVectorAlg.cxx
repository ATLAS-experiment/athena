/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <map>
#include <vector>
#include "AthenaBaseComps/AthMsgStreamMacros.h"

#include "Hit.h"
#include "PerfMonTestVectorAlg.h"

/* #define DEBUG_ME 1 */

using namespace PerfMonTest;
using namespace::std;

typedef vector<DHit> HitContainer;


StatusCode VectorAlg::execute()
{  
  ATH_MSG_DEBUG("Executing ");
  HitContainer vhit;
  vhit.reserve(m_2bReserved.value());
  int vcap(vhit.capacity());
  int vold;
#ifdef DEBUG_ME
  IHit* p1(0);
  IHit* p2(0);
#endif
  IHit* p3(0);
  cout << "initial capacity " << vcap << endl;
  int size(m_vectorSize.value());
  for(int i(0); i<size; ++i) {
    vold=vcap;
#ifdef DEBUG_ME
    p1=p2;
    p2=p3;
#endif
    vhit.push_back(DHit(i,i,i));
    vcap=vhit.capacity();
    p3=&vhit[i];
    if (m_mapIt.value()) m_mixMap[i]=p3;
    if (vold != vcap) cout << "iteration " << i << " new capacity " << vcap <<endl;
#ifdef DEBUG_ME
    if (((int)p3-(int)p2) != ((int)p2-(int)p1)) cout << "iteration " << i << " new chunk @" << hex << p3 << " previous was @" << p2 << dec << endl;
#ifdef REALLY_DEBUG_ME
    cout << "iteration " << i << " P3 @" << hex << p3 << " p2 @" << p2 << " p1 @" << p1 << dec << ' ' << (int)p3-(int)p2 << ' ' <<(int)p2-(int)p1 <<endl;
#endif
#endif
  }

  return StatusCode::SUCCESS;
}
