/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// MuonDigitContainer.cxx

#include "MuonDigitContainer/TgcDigitContainer.h"
#include "EventContainers/SelectAllObject.h"
#include <map>
#include <cassert>
#include <iostream>

using std::ostream;
using std::endl;

//**********************************************************************
// Local definitions
//**********************************************************************

//**********************************************************************
// Member functions.
//**********************************************************************

// Default constructor.

TgcDigitContainer::TgcDigitContainer(unsigned int hash_max)
: IdentifiableContainer<TgcDigitCollection>(hash_max) 
{

}

//**********************************************************************

// Destructor.

TgcDigitContainer::~TgcDigitContainer() = default;

//**********************************************************************

// return the class ID

CLID TgcDigitContainer::classID()
{
  return ClassID_traits<TgcDigitContainer>::ID();       
}

const CLID& TgcDigitContainer::clID() const
{
  static const CLID cid = classID();
  return cid;
}

//**********************************************************************

// Return the total number of digits in the container.

TgcDigitContainer::size_type TgcDigitContainer::digit_size() const {
  TgcDigitContainer::size_type count = 0;
  TgcDigitContainer::const_iterator it = begin();
  TgcDigitContainer::const_iterator iend = end();

  for (; it != iend; ++it ) {
    count += (*it)->size();
  }
  return count;
}


//**********************************************************************
// Free functions.
//**********************************************************************

// Output stream.

ostream& operator<<(ostream& lhs, const TgcDigitContainer& rhs) {
  lhs << "TgcDigitContainer has " << rhs.digit_size() << " digits:" << endl;
  typedef SelectAllObject<TgcDigitContainer> SELECTOR; 
  SELECTOR sel(&rhs);
  SELECTOR::const_iterator it = sel.begin(); 
  SELECTOR::const_iterator it_end = sel.end(); 

  bool first = true;
  for ( ; it!=it_end;++it)
  { 
      if ( first ) {
        first = false;
      } else {
        lhs << endl;
      }
      lhs << "  " << *it;
  }
  return lhs;
}

//**********************************************************************
