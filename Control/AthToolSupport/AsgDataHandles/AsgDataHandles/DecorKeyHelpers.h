/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */

#ifndef ASG_DATA_HANDLES_DECOR_KEY_HELPERS_H
#define ASG_DATA_HANDLES_DECOR_KEY_HELPERS_H

#ifndef XAOD_STANDALONE
#include <StoreGate/DecorKeyHelpers.h>
#else

#include "AsgDataHandles/VarHandleKey.h"
#include <stdexcept>
#include <string>

namespace SG {

inline std::string contKeyFromKey (const std::string& key)
{
  const auto split = key.rfind ('.');
  if (split == std::string::npos)
    return key;
  return key.substr (0, split);
}

inline std::string decorKeyFromKey (const std::string& key)
{
  const auto split = key.rfind ('.');
  if (split == std::string::npos)
    return "";
  return key.substr (split + 1);
}

inline std::string makeContDecorKey(const std::string& cont, const std::string& decor)
{
  if (cont.empty()) return decor;
  if (decor.empty()) return cont;
  return cont + '.' + decor;
}

inline std::string makeContDecorKey(const VarHandleKey& contKey, const std::string& key)
{
  if (key.empty() || key.ends_with ('+')) {
    return "";
  }
  return makeContDecorKey( contKey.key(), key);
}

}

#endif

#endif
