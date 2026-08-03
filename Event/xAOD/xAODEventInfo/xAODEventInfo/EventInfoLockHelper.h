// Dear emacs, this is -*- c++ -*-
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODEVENTINFO_EVENTINFOLOCKHELPER
#define XAODEVENTINFO_EVENTINFOLOCKHELPER

#include <stdint.h> 
#include <functional>
#include <vector>
#include "CxxUtils/checker_macros.h"

//Helper class to avoid locking EventInfo flags (such as ErrorState or detectorFlags) 
//when reading xAOD::EventInfo from a POOL file
//Useful only in the context of AODFixes involving these flags. 


class EventInfoLockHelper {
public:
    EventInfoLockHelper()=default;
    typedef std::function<bool(uint32_t, uint32_t,uint64_t)> unlockFunc_t;

     /**
      * @brief Add a function defining if the flags are to be locked. Should be called from initialize() of the AODFix algorithm  
      * @param fn A std::function taking a run-number, a lumiblock number and an event number as paramenter. Should return true if the lock should be avoided
      *                 
      * This (static) method should be called from initialize() of the AODFix algorithm 
      */
    static void addUnlockFunc(unlockFunc_t fn) ATLAS_NOT_THREAD_SAFE;
   
   
    /** 
      * @brief Check if the lock should happen or not 
      * @param runNbr RunNumber of the current event
      * @param lbNbr LumiBlock number of the current event
      * @param evtNbr EventNumber of the current event
      *                 
      * To be called in EventAuxInfo_vN::toTransient() 
      */
    bool evalUnlockFunc(uint32_t runNbr, uint32_t lbNrb, uint64_t evtNbr) const;

private:
    static std::vector<unlockFunc_t> s_unlockFuncs ATLAS_THREAD_SAFE;


};






#endif