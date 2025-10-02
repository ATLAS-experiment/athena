/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-

/**
 * @file
 *
 * Overlaying of Identifiable Containers.  Functions common to the
 * single-hit-per-readout-channel-in-an-event case and the
 * possible-multiple-hits-on-a-channel case are declared in this file.
 *
 * @author Tadej Novak
 * @author Christos Anastopoulos DataPool strategy
 * @author Andrei Gaponenko <agaponenko@lbl.gov>, 2009
 */

#ifndef IDC_OVERLAYCOMMON_H
#define IDC_OVERLAYCOMMON_H

#include <memory>

#include <Identifier/IdentifierHash.h>
#include "AthAllocators/DataPool.h"

namespace Overlay
{

/*
 * We do not have implementations for these.
 * A concrete algorithm inheriting from
 * IDC_OverlayBase will have to specialize
 * one of the two based on if uses
 * DataPool or not
 */
template <class Collection>
std::unique_ptr<Collection> copyCollection(const IdentifierHash &hashId,
                                           const Collection *collection);

template <typename Collection, typename Type>
std::unique_ptr<Collection> copyCollection(const IdentifierHash &hashId,
                                           const Collection *collection,
                                           DataPool<Type>& dataItems);
/*
 * We have implementations for these.
 * A concrete algorithm inheriting from
 * IDC_OverlayBase could potentially specialize
 * one of the two based on if uses
 * DataPool or not
 */
template <typename Collection, typename Alg>
void mergeCollections(Collection *bkgCollection,
                      Collection *signalCollection,
                      Collection *outputCollection,
                      const Alg *algorithm);

template <typename Type, typename Collection, typename Alg>
void mergeCollections(Collection *bkgCollection,
                      Collection *signalCollection,
                      Collection *outputCollection,
                      const Alg *algorithm,
                      DataPool<Type>& dataItems);

template <typename Type, typename Collection, typename Alg>
void mergeSortedCollections(Collection *bkgCollection,
                            Collection *signalCollection,
                            Collection *outputCollection,
                            const Alg *algorithm,
                            DataPool<Type>& dataItems);

/*
 * We have a dummy implementation of this.
 * Algorithms have to specialize this.
 */
template<class Datum, class Alg>
void mergeChannelData(Datum &baseDatum,
                      const Datum &additionalDatum,
                      const Alg *algorithm);

/*
 * Algorithms have to specialize this if used.
 */
template <typename Collection>
void sortCollection(Collection *collection);

} // namespace Overlay

#include "IDC_OverlayBase/IDC_OverlayCommon.icc"

#endif
