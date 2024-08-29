/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/src/PLinks_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2023
 * @brief For testing packed links.
 */


#include "DataModelTestDataCommon/versions/PLinks_v1.h"
#include "AthContainers/PackedLink.h"


namespace DMTest {


/// Get the single link.
ElementLink<CVec> PLinks_v1::plink() const {
  static const SG::ConstAccessor<plink_type> acc( "plink" );
  return acc( *this );
}


/// Set the single link.
void PLinks_v1::setPLink( const ElementLink<CVec>& value ) {
  static const SG::Accessor<plink_type> acc( "plink" );
  acc( *this ) = value;
}


/// Get element @c i of the vector of links.
ElementLink<CVec> PLinks_v1::vlink (size_t i) const {
  static const SG::ConstAccessor<vlinks_type> acc( "vlinks" );
  return acc( *this ).at (i);
}


/// Set element @c i of the vector of links.
void PLinks_v1::setVLink (size_t i, const ElementLink<CVec>& l)
{
  static const SG::Accessor<vlinks_type> acc( "vlinks" );
  acc( *this ).at(i) = l;
}


/// Get the vector of links, as a const range.
auto PLinks_v1::vlinks() const -> const_el_range
{
  static const SG::ConstAccessor<vlinks_type> acc( "vlinks" );
  return acc( *this );
}


/// Get the vector of links, as a non-const range.
auto PLinks_v1::vlinks() -> el_range
{
  static const SG::Accessor<vlinks_type> acc( "vlinks" );
  return acc( *this );
}


/// Set the vector of links.
void PLinks_v1::setVLinks (const std::vector<ElementLink<CVec> >& v)
{
  static const SG::Accessor<vlinks_type> acc( "vlinks" );
  acc( *this ) = v;
}


} // namespace DMTest

