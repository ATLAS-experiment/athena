// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file ActsGeometry/ActsElementVector.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2022
 * @brief Helper to hold elements for deletion.
 */


#ifndef ACTSGEOMETRY_ACTSELEMENTVECTOR_H
#define ACTSGEOMETRY_ACTSELEMENTVECTOR_H


#include "ActsGeometry/ActsDetectorElement.h"
#include "CxxUtils/checker_macros.h"
#include <mutex>


/**
 * @brief Helper to hold elements for deletion.
 *
 * This just holds a vector of shared_ptr's to AtlasDetectorElement,
 * in order to maintain a reference count on them until it's time
 * for them to be deleted.
 */
class ActsElementVector
{
public:
  // The only thing we can do is add another element to the list.
  // Declared const so that the thread-safety checker won't complain ---
  // we lock the vector internally.
  void push_back (std::shared_ptr<const ActsDetectorElement> p) const
  {
    std::scoped_lock lock (m_mutex);
    m_vec.push_back (p);
  }

  // Allow access if we have a mutable reference
  std::vector<std::shared_ptr<const ActsDetectorElement>>& vector() {
    return m_vec;
  }

private:
  mutable std::vector<std::shared_ptr<const ActsDetectorElement>> m_vec ATLAS_THREAD_SAFE;
  mutable std::mutex m_mutex;
};


#endif // not ACTSGEOMETRY_ACTSELEMENTVECTOR_H
