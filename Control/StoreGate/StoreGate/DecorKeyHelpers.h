// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file StoreGate/DecorKeyHelpers.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2017
 * @brief Some common helper functions used by decoration handles.
 */


#ifndef STOREGATE_DECORKEYHELPERS_H
#define STOREGATE_DECORKEYHELPERS_H


#include <string>


namespace SG {

class VarHandleKey;

/**
 * @brief Extract the container part of key.
 * @param key The decoration handle key.
 *
 * Given a key of the form CCC.DDD, returns the container part, CCC.
 */
std::string contKeyFromKey (const std::string& key);


/**
 * @brief Extract the decoration part of key.
 * @param key The decoration handle key.
 *
 * Given a key of the form CCC.DDD, returns the decoration part, DDD or
 * deflt if no decoration key is found.
 */
std::string decorKeyFromKey (const std::string& key, const std::string& deflt = "");


/**
 * @brief Make a StoreGate key from container and decoration name.
 * @param cont  The container handle key.
 * @param decor The decoration handle key.
 *
 * Given keys of the form "CCC" and "DDD", returns the full key, "CCC.DDD".
 * If cont or decor is empty, returns the non-empty key (or an empty string).
 */
std::string makeContDecorKey(const std::string& cont, const std::string& decor);


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
std::string makeContDecorKey(const VarHandleKey& contKey, const std::string& sgkey);


/**
 * @brief Remove container name from decoration key.
 * @param contKey  The VarHandleKey of the container holding the decoration.
 * @param key      The decoration name to be modified.
 *
 * Given a key of the form SG+CCC.DDD, removes SG+CCC.
 */
void removeContFromDecorKey(const VarHandleKey& contKey, std::string& key);

} // namespace SG


#endif // not STOREGATE_DECORKEYHELPERS_H
