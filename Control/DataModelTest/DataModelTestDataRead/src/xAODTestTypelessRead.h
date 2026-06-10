// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file src/xAODTestTypelessRead.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2014
 * @brief Algorithm to test reading xAOD objects with auxiliary data,
 *        without compile-time typing of aux data.
 */

#ifndef DATAMODELTESTDATAREAD_XAODTESTTYPELESSREAD_H
#define DATAMODELTESTDATAREAD_XAODTESTTYPELESSREAD_H


#include "AthenaBaseComps/AthAlgorithm.h"


namespace DMTest {


/**
 * @brief Algorithm for reading test xAOD data.
 */
class xAODTestTypelessRead
  : public AthAlgorithm
{
public:
  using AthAlgorithm::AthAlgorithm;


  /**
   * @brief Algorithm event processing.
   */
  virtual StatusCode execute(const EventContext& ctx); 


private:
  template <class OBJ, class AUX>
  StatusCode testit (const std::string& key);

  template <class OBJ>
  StatusCode testit_view (const std::string& key);

  /// Event counter.
  int m_count = 0;

  /// Prefix for names written to SG.  Null for no write.
  StringProperty m_writePrefix
    { this, "WritePrefix", "cview", "" };

  StringProperty m_cviewKey
    { this, "CViewKey", "cview", "" };
  StringProperty m_pvecKey
    { this, "PVecKey", "pvec", "" };
  StringProperty m_hviewKey
    { this, "HViewKey", "hview", "" };
};


} // namespace DMTest


#endif // not DATAMODELTESTDATAREAD_XAODTESTTYPELESSREAD_H
