/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/src/JVec_v1.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief For testing jagged vectors.
 */


#include "DataModelTestDataCommon/versions/JVec_v1.h"
#include "AthContainers/JaggedVec.h"


namespace DMTest {


auto JVec_v1::ivec() const -> intRange_t {
  static const SG::ConstAccessor<ielt_type> acc( "ivec" );
  return acc( *this );
}


void JVec_v1::setIVec( const std::vector<int>& v ) {
  static const SG::Accessor<ielt_type> acc( "ivec" );
  acc( *this ) = v;
}


auto JVec_v1::fvec() const -> floatRange_t {
  static const SG::ConstAccessor<felt_type> acc( "fvec" );
  return acc( *this );
}


void JVec_v1::setFVec( const std::vector<float>& v ) {
  static const SG::Accessor<felt_type> acc( "fvec" );
  acc( *this ) = v;
}


auto JVec_v1::svec() const -> stringRange_t {
  static const SG::ConstAccessor<selt_type> acc( "svec" );
  return acc( *this );
}


void JVec_v1::setSVec( const std::vector<std::string>& v ) {
  static const SG::Accessor<selt_type> acc( "svec" );
  acc( *this ) = v;
}


auto JVec_v1::lvec() const -> linkRange_t {
  static const SG::ConstAccessor<lelt_type> acc( "lvec" );
  return acc( *this );
}


void JVec_v1::setLVec( const std::vector<ElementLink<CVec> >& v ) {
  static const SG::Accessor<lelt_type> acc( "lvec" );
  acc( *this ) = v;
}


} // namespace DMTest

