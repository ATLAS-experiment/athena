/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigAnalysisTest_EVENTPTRDEF_H
#define TrigAnalysisTest_EVENTPTRDEF_H
/* This is a copy of TrigDecisionTool/EventPtrDef.h
 * should be removed and taken from the original once the
 * tag is propagated
 */

namespace asg {
   class SgEvent;
}
class StoreGateSvc;

#ifdef XAOD_STANDALONE
    typedef asg::SgEvent* EventPtr_t;
#elif !defined(XAOD_STANDALONE)
      typedef StoreGateSvc*  EventPtr_t;
#else
#   error "Wrong environment configuration detected!"
#endif

#endif
