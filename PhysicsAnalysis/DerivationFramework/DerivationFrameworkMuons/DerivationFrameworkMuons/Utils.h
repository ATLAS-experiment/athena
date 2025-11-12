/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef DERIVATIONFRAMEWORK_MUONUTILS_H
#define DERIVATIONFRAMEWORK_MUONUTILS_H

#include "StoreGate/WriteDecorHandle.h"
class EventContext;

namespace DerivationFramework{
    /*   Small helper function to create a WriteDecorHandle  which then sets the value of the first element
     *   automatically to a default value
     */
    template <class dType, class ContType> SG::WriteDecorHandle<ContType, dType> makeHandle(const EventContext& ctx, 
                                                                                            const SG::WriteDecorHandleKey<ContType>& key,
                                                                                            const dType &defValue = dType{}) {
        SG::WriteDecorHandle<ContType, dType> handle{key, ctx};
        if (handle->size()) handle(*handle->at(0)) = defValue;
        return handle;
    }
}
#endif