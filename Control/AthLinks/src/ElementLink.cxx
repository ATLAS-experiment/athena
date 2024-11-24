/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthLinks/src/ElementLink.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2024
 * @brief a persistable pointer to an element of a STORABLE (data object)
 */


#include "AthLinks/ElementLink.h"
#include "AthenaKernel/IProxyDict.h"


namespace SG_detail {


/**
 * @brief See if an EL is being remapped.
 * @param sgkey_in Original hashed key of the EL.
 * @param index_in Original index of the EL.
 * @param sgkey_out[out] New hashed key for the EL.
 * @param index_out[out] New index for the EL.
 * @return True if there is a remapping; false otherwise.
 *
 * This version is for the case where the EL index is a @c size_t.
 * For other index types, the the templated version below is used
 * (which doesn't allow remapping indices).
 */
bool checkForRemap (IProxyDict* sg,
                    SG::sgkey_t sgkey_in,
                    size_t index_in,
                    SG::sgkey_t& sgkey_out,
                    size_t& index_out)
{
  return sg->tryELRemap (sgkey_in, index_in, sgkey_out, index_out);
}


} // namespace SG_detail
