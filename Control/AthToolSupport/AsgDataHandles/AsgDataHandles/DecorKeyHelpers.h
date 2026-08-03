/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
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


/**
 * @brief Extract the container part of key.
 * @param key The decoration handle key.
 *
 * Given a key of the form CCC.DDD, returns the container part, CCC.
 */
inline std::string contKeyFromKey (const std::string& key)
{
  std::string::size_type ipos = key.find ('.');
  if (ipos == std::string::npos)
    return key;
  return key.substr (0, ipos);
}


/**
 * @brief Extract the decoration part of key.
 * @param key The decoration handle key.
 *
 * Given a key of the form CCC.DDD, returns the decoration part, DDD or
 * deflt if no decoration key is found.
 */
inline std::string decorKeyFromKey (const std::string& key)
{
  std::string::size_type ipos = key.find ('.');
  if (ipos == std::string::npos)
    return "";
  return key.substr (ipos+1, std::string::npos);
}


/**
 * @brief Make a StoreGate key from container and decoration name.
 * @param cont  The container handle key.
 * @param decor The decoration handle key.
 *
 * Given keys of the form "CCC" and "DDD", returns the full key, "CCC.DDD".
 * If cont or decor is empty, returns the non-empty key (or an empty string).
 */
inline std::string makeContDecorKey(const std::string& cont, const std::string& decor)
{
  if (cont.empty()) return decor;
  if (decor.empty()) return cont;
  return cont + '.' + decor;
}


/**
 * @brief Make a StoreGate key from container and decoration.
 * @param contKey  The VarHandleKey of the container holding the decoration.
 * @param key      The decoration name.
 *
 * Construct the StoreGate key from the associated container and the
 * decoration name passed in @c key. If the latter also contains the container
 * name, an exception will be raised.  Returns an empty string if @c key
 * is empty.
 */
inline std::string makeContDecorKey(const VarHandleKey& contKey, const std::string& key)
{
  if (key.empty() || key.ends_with ('+')) {
    return "";
  }
  if (key.find('.') != std::string::npos) {
    throw std::runtime_error (key + " (DecorHandleKey has been declared with a parent container. "
                              "Its value should not contain any container name.)");
  }

  // Note: VHK::fullKey().key() contains the store prefix, key() does not.
  return makeContDecorKey( contKey.key(), key);
}


/**
 * @brief Remove container name from decoration key.
 * @param contKey  The VarHandleKey of the container holding the decoration.
 * @param key      The decoration name to be modified.
 *
 * Given a key of the form SG+CCC.DDD, removes SG+CCC.
 */
inline void removeContFromDecorKey(const VarHandleKey& contKey, std::string& key)
{
  // Remove container name from key
  const std::string toerase = contKey.key() + ".";
  const size_t pos = key.find(toerase);
  if (pos != std::string::npos) {
    key.erase(pos, toerase.size());
  }

}

} // namespace SG

#endif // XAOD_STANDALONE
#endif // ASG_DATA_HANDLES_DECOR_KEY_HELPERS_H
