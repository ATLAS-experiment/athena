// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/versions/JVec_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief For testing jagged vectors.
 */


#ifndef DATAMODELTESTDATACOMMON_JVEC_V1_H
#define DATAMODELTESTDATACOMMON_JVEC_V1_H


#include "DataModelTestDataCommon/CVec.h"
#include "AthContainers/AuxElement.h"
#include "xAODCore/JaggedVec.h"
#include "AthLinks/ElementLink.h"
#include "AthenaKernel/BaseInfo.h"
#include <string>


namespace DMTest {


/**
 * @brief For testing jagged vectors.
 *
 * A JVec object holds an jagged vectors to several different types:
 * int, float, std::string, and ElementLink<CVec>.
 */
class JVec_v1
  : public SG::AuxElement
{
private:
  using ielt_type = SG::JaggedVecElt<int>;
  using felt_type = SG::JaggedVecElt<float>;
  using selt_type = SG::JaggedVecElt<std::string>;
  using lelt_type = SG::JaggedVecElt<ElementLink<CVec> >;


public:
  using intRange_t    = SG::ConstAccessor<ielt_type>::element_type;
  using floatRange_t  = SG::ConstAccessor<felt_type>::element_type;
  using stringRange_t = SG::ConstAccessor<selt_type>::element_type;
  using linkRange_t   = SG::ConstAccessor<lelt_type>::element_type;

  // Getters / setters.

  intRange_t ivec() const;
  void setIVec (std::span<const int> v);
  floatRange_t fvec() const;
  void setFVec (std::span<const float> v);
  stringRange_t svec() const;
  void setSVec (std::span<const std::string> v);
  linkRange_t lvec() const;
  void setLVec (std::span<const ElementLink<CVec> > v);
};


} // namespace DMTest


SG_BASE (DMTest::JVec_v1, SG::AuxElement);


#endif // not DATAMODELTESTDATACOMMON_JVEC_V1_H
