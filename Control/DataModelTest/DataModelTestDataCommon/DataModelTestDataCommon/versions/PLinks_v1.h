// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/versions/PLinks_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2023
 * @brief For testing packed links.
 */


#ifndef DATAMODELTESTDATACOMMON_PLINKS_V1_H
#define DATAMODELTESTDATACOMMON_PLINKS_V1_H


#include "DataModelTestDataCommon/CVec.h"
#include "AthContainers/AuxElement.h"
#include "xAODCore/PackedLink.h"
#include "AthenaKernel/BaseInfo.h"


namespace DMTest {


/**
 * @brief For testing packed links.
 *
 * A PLinks object holds an ElementLink to CVec objects, stored as
 * packed links, as well as a vector of links.
 */
class PLinks_v1
  : public SG::AuxElement
{
private:
  using plink_type = SG::PackedLink<CVec>;
  using vlinks_type = std::vector<SG::PackedLink<CVec> >;


public:
  // Get/set the single link.
  ElementLink<CVec> plink() const;
  void setPLink (const ElementLink<CVec>& l);

  // Get/set one element of the vector of links.
  ElementLink<CVec> vlink (size_t i) const;
  void setVLink (size_t i, const ElementLink<CVec>& l);

  // Get the vector of links, as a const range.
  using const_el_range = SG::ConstAccessor<vlinks_type>::const_elt_span;
  const_el_range vlinks() const;

  // Get the vector of links, as a non-const range.
  using el_range = SG::Accessor<vlinks_type>::elt_span;
  el_range vlinks();

  // Set the vector of links.
  void setVLinks (const std::vector<ElementLink<CVec> >& v);
};


} // namespace DMTest


SG_BASE (DMTest::PLinks_v1, SG::AuxElement);


#endif // not DATAMODELTESTDATACOMMON_PLINKS_V1_H
