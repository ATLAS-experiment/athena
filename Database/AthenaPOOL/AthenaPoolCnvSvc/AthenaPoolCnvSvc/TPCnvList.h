// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file AthenaPoolCnvSvc/TPCnvList.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2016
 * @brief Helper for calling TP converters from an Athena converter.
 */


#ifndef ATHENAPOOLCNVSVC_TPCNVLIST_H
#define ATHENAPOOLCNVSVC_TPCNVLIST_H


#include "AthenaPoolCnvSvc/TPCnvElt.h"
#include <tuple>


namespace AthenaPoolCnvSvc {


/**
 * @brief Helper for calling TP converters from an Athena converter.
 */
template <class CNV, class TRANS, class ... TPCNVS>
class TPCnvList
{
public:
  using list_t = std::tuple<TPCnvElt<CNV, TPCNVS>...>;


  /**
   * @brief Read the persistent object and convert it to transient.
   * @param parent The top-level pool converter object.
   * @param key The SG key of the object being read.
   * @param msg MsgStream for error reporting.
   *
   * Returns a newly-allocated object.
   * If the type of the persistent object on the file does not match the
   * the type of any of our TP converters, return nullptr.
   * Other errors are reported by raising exceptions.
   */
  std::unique_ptr<TRANS>
  createTransient (CNV& parent, const Token* token, const std::string& key, MsgStream& msg);

  
  /**
   * @brief Read the persistent object and convert it to transient.
   * @param parent The top-level pool converter object.
   * @param trans The transient object to modify.
   * @param key The SG key of the object being read.
   * @param msg MsgStream for error reporting.
   *
   * Overwrites the provided transient object.
   * If the type of the persistent object on the file does not match the
   * the type of any of our TP converters, return false.
   * Other errors are reported by raising exceptions.
   */
  bool persToTrans (CNV& parent, TRANS* trans, const Token* token, const std::string& key, MsgStream& msg);
  

private:
  /// List of TP converter instances, wrapped by @c TPCnvElt.
  list_t m_list{};
};


} // namespace AthenaPoolCnvSvc


#include "AthenaPoolCnvSvc/TPCnvList.icc"


#endif // not ATHENAPOOLCNVSVC_TPCNVLIST_H


