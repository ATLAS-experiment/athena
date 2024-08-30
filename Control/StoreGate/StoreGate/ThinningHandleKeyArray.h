/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STOREGATE_THINNINGHANDLEKEYARRAY_H
#define STOREGATE_THINNINGHANDLEKEYARRAY_H 1

#include "StoreGate/HandleKeyArray.h"
#include "StoreGate/ThinningHandle.h"
#include "StoreGate/ThinningHandleKey.h"


namespace SG {

  /**
   * @class SG::ThinningHandleKeyArray<T>
   * @brief class to hold an array of ThinningHandleKeys
   *
   * since it inherits from std::vector, all vector operations are
   * permitted.
   *
   * This can be converted to a class if it needs customization
   *
   * Note: makeHandles only works if the type is the same; otherwise have to do individually
   *
   */
  template <class T>
  using ThinningHandleKeyArray = HandleKeyArray<ThinningHandle<T>,ThinningHandleKey<T>, Gaudi::DataHandle::Reader >;
  


} // namespace SG

#endif
