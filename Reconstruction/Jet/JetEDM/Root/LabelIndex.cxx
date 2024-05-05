/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// LabelIndex.h

#include "JetEDM/LabelIndex.h"
#include <map>
#include <utility>

using jet::LabelIndex;

using LIMap = std::map<std::string, LabelIndex *>;
using Index = LabelIndex::Index;
using Label = LabelIndex::Label;

//**********************************************************************

LabelIndex::LabelIndex(Label nam) : m_name(std::move(nam)) { }

//**********************************************************************

Index LabelIndex::addLabel(const Label& lab) {
  Index idx = index(lab);
  if ( idx ) return idx;
  m_labs.push_back(lab);
  xAOD::JetConstitScale conscale = xAOD::CalibratedJetConstituent;
  if ( lab == "EMTopo" ) conscale = xAOD::UncalibratedJetConstituent;
  m_constitScales.push_back(conscale);
  return m_labs.size();
}

//**********************************************************************

const Label& LabelIndex::name() const {
  return m_name;
}

//**********************************************************************

Label LabelIndex::label(Index idx) const {
  if ( idx < 1 ) return "";
  if ( idx-1 > m_labs.size() ) return "";
  return m_labs[idx - 1];
}

//**********************************************************************

xAOD::JetConstitScale LabelIndex::constitScale(Index idx) const {
  if ( idx < 1 ) return xAOD::CalibratedJetConstituent;
  if ( idx-1 > m_constitScales.size() ) return xAOD::UncalibratedJetConstituent;
  return m_constitScales[idx - 1];
}

//**********************************************************************

Index LabelIndex::index(const Label& lab) const {
  for ( Index jdx=0; jdx<m_labs.size(); ++jdx ) {
    if ( m_labs[jdx] == lab ) return jdx + 1;
  }
  return 0;
}

//**********************************************************************

LabelIndex::LabelIndex(const LabelIndex&) : m_name("") { }

//**********************************************************************

LabelIndex& LabelIndex::operator=(const Label&) {
  return *this;
}

//**********************************************************************

Index LabelIndex::size() const { return m_labs.size() ; }
