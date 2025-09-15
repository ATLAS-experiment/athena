/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthViews/View.h"

using namespace SG;

View::View(const std::string& name, const int index, const bool AllowFallThrough, std::string const& storeName) {

  std::string fullName = name;
  if ( index == -1 ) {
    m_index = 0;
  }
  else {
    m_index = index;
    fullName += '_';
    fullName += std::to_string( index );
  }

  m_implementation = new SimpleView( fullName, AllowFallThrough, storeName );
}

View::~View () {
  delete m_implementation;
}
