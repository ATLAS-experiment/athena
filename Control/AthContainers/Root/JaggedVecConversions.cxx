/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/JaggedVecConversions.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2024
 * @brief Conversions for accessing jagged vector variables.
 */


#include "AthContainers/tools/JaggedVecConversions.h"


namespace SG { namespace detail {


/**
 * @brief Resize one jagged vector element.
 * @param elt_index The index of the element to resize.
 * @param n_new The size of the new element.
 *
 * Any added payload elements are default-initialized.
 */
void JaggedVecProxyBase::resize1 (size_t elt_index, index_type n_new)
{
  int n_old = elt (elt_index).size (elt_index);
  adjust1 (elt_index, n_old, static_cast<int>(n_new) - n_old);
}


/**
 * @brief Add or remove payload items from one jagged vector element.
 * @param elt_index The index of the element to resize.
 * @param index The index of the payload item within this jagged vector
 *              element after the insertion/deletion (in other words,
 *              the first element that moves as a result of the change).
 * @param n_add The number of elements to add.  May be negative
 *              to remove elements.
 */
void JaggedVecProxyBase::adjust1 (size_t elt_index, index_type index, int n_add)
{
  // Return right away if there's nothing to do.
  if (n_add == 0) return;

  // Find the IAuxTypeVector for the payload variable.
  IAuxTypeVector* linkedVec;
  if (m_linkedVec.index() == 0) {
    // We already have it.
    linkedVec = std::get<0>(m_linkedVec);
  }
  else {
    // Look it up from the container.
    SG::auxid_t auxid = std::get<1>(m_linkedVec);
    linkedVec = m_container.getStore()->linkedVector (auxid);

    // Remember it to possibly use again.
    m_linkedVec = linkedVec;
  }

  // Indices from the element that we're modifying.
  Elt_t& e = elt(elt_index);
  size_t beg = e.begin(elt_index);
  size_t end = e.end();

  // Shift the payload items.
  if (!linkedVec->shift (beg+index, n_add)) {
    m_container.clearCache (linkedVec->auxid());
  }

  // Adjust the indices in the jagged vector elements.
  // First the element that we're modifying...
  e = JaggedVecEltBase (end + n_add);
  // .. then all the remaining elements.
  std::for_each (m_elts.begin() + elt_index+1, m_elts.end(),
                 JaggedVecEltBase::Shift (n_add));
}


} } // namespace SG::detail
