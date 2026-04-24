/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CscCalibEvent/CscCalibDataContainer.h"
#include "EventContainers/SelectAllObject.h"
#include <iostream>

CscCalibDataContainer::CscCalibDataContainer(int maxhash) 
	: IdentifiableContainer<CscCalibDataCollection>(maxhash) {
}

/** return the class ID */
CLID CscCalibDataContainer::classID() {
  return ClassID_traits<CscCalibDataContainer>::ID();       
}


/** Return the total number of CalibDatas in the container */
CscCalibDataContainer::size_type CscCalibDataContainer::calibData_size() const {
  CscCalibDataContainer::size_type count = 0;
  CscCalibDataContainer::const_iterator it = begin();
  CscCalibDataContainer::const_iterator iend = end();
  for (; it != iend; ++it ) {
    count += (*it)->size();
  }
  return count;
}


/** Output stream - for printing */ 
std::ostream& operator<<(std::ostream& lhs, const CscCalibDataContainer& rhs) {
  lhs << "CscDataCollectionContainer has " << rhs.calibData_size() << " CalibData:" << std::endl;
  typedef SelectAllObject<CscCalibDataContainer> SELECTOR; 
  SELECTOR sel(&rhs);
  SELECTOR::const_iterator it = sel.begin(); 
  SELECTOR::const_iterator it_end = sel.end(); 
  bool first = true;
  for ( ; it!=it_end;++it) { 
    if ( first ) {
       first = false;
    } else {
      lhs << std::endl;
    }
    lhs << "  " << *it;
  }
  return lhs;
}

