#include "xAODEventInfo/EventInfoLockHelper.h"


    std::vector<EventInfoLockHelper::unlockFunc_t> EventInfoLockHelper::s_unlockFuncs;


    void EventInfoLockHelper::addUnlockFunc(unlockFunc_t fn) {
        s_unlockFuncs.push_back(fn);
    }


    bool EventInfoLockHelper::evalUnlockFunc(uint32_t runNbr, uint32_t lbNrb, uint64_t evtNbr) const {
        for (auto& fn : s_unlockFuncs) {
            if (fn(runNbr,lbNrb,evtNbr)) return true;
        }
        return false;
    }