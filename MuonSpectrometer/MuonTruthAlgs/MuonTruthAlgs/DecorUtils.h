/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHALGS_DECORUTILS_H
#define MUONTRUTHALGS_DECORUTILS_H

#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "StoreGate/WriteDecorHandle.h"

namespace Muon{
    template <typename ContType, typename DataType>
    using DecorHandlePtr_wt = std::unique_ptr<SG::WriteDecorHandle<ContType, DataType>>;
    template <typename ContType, typename DataType>
    using DecorHandlePtrVec_t = std::vector<DecorHandlePtr_wt<ContType, DataType>>;

 
    /** @brief Returns a unique_ptr with an initialized WriteDecorHandle  */
    template <typename DataType, typename ContType>
        DecorHandlePtr_wt<ContType, DataType> 
            makeHandle(const EventContext& ctx,
                       const SG::WriteDecorHandleKey<ContType>& key,
                       const DataType defVal = {}) {
            if (key.empty()) {
                return nullptr;
            }
            auto decorHandle = std::make_unique<SG::WriteDecorHandle<ContType, DataType>>(key, ctx);
            for (const auto* obj : (**decorHandle)){
                (*decorHandle)(*obj) = defVal;
            }
            return decorHandle;
    }
    
    template <typename DataType, typename ContType>
        DecorHandlePtrVec_t<ContType, DataType>
                makeHandles(const EventContext& ctx,
                            const SG::WriteDecorHandleKeyArray<ContType>& keys,
                            const DataType defVal = {}) {
            DecorHandlePtrVec_t<ContType, DataType> handles{};
            for (const SG::WriteDecorHandleKey<ContType>& key : keys) {
                handles.emplace_back(makeHandle(ctx, key, defVal));
            }
            return handles;
        }
}
#endif