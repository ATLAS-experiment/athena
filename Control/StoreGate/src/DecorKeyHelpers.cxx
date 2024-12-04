/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file StoreGate/src/DecorKeyHelpers.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2017
 * @brief Some common helper functions used by decoration handles.
 */


#include "StoreGate/DecorKeyHelpers.h"
#include "StoreGate/exceptions.h"
#include "StoreGate/VarHandleKey.h"


namespace SG {


/**
 * @brief Extract the container part of key.
 * @param key The decoration handle key.
 *
 * Given a key of the form CCC.DDD, returns the container part, CCC.
 */
std::string contKeyFromKey (const std::string& key)
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
 * Given a key of the form CCC.DDD, returns the decoration part, DDD.
 */
std::string decorKeyFromKey (const std::string& key)
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
std::string makeContDecorKey(const std::string& cont, const std::string& decor)
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
std::string makeContDecorKey(const VarHandleKey& contKey, const std::string& key)
{
  if (key.empty() || key.ends_with ('+')) {
    return "";
  }
  if (key.find('.') != std::string::npos) {
    throw SG::ExcBadHandleKey(key + " (DecorHandleKey has been declared with a parent container. "
                              "Its value should not contain any container name.)");
  }

  // Note: VHK::fullKey().key() contains the store prefix, key() does not.
  return makeContDecorKey( contKey.fullKey().key(), key);
}


/**
 * @brief Remove container name from decoration key.
 * @param contKey  The VarHandleKey of the container holding the decoration.
 * @param key      The decoration name to be modified.
 *
 * Given a key of the form SG+CCC.DDD, removes SG+CCC.
 */
void removeContFromDecorKey(const VarHandleKey& contKey, std::string& key)
{
  // Remove container name from key
  const std::string toerase = contKey.fullKey().key() + ".";
  const size_t pos = key.find(toerase);
  if (pos != std::string::npos) {
    key.erase(pos, toerase.size());
  }

}

} // namespace SG
