/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETEVENTATHENAPOOL_INDETSIMDATACOLLECTION_P4_H
#define INDETEVENTATHENAPOOL_INDETSIMDATACOLLECTION_P4_H

#include <vector>
#include <utility>
#include "InDetEventAthenaPool/InDetSimData_p3.h"
#include "Identifier/Identifier.h"

class InDetSimDataCollection_p4
{


public:

  InDetSimDataCollection_p4()
  { } ;

  // container cnv does conversion
  friend class InDetSimDataCollectionCnv_p4;

private:
  std::vector<std::pair<Identifier::value_type, InDetSimData_p3> > m_simdata;
};

#endif // INDETEVENTATHENAPOOL_INDETSIMDATA_P4_H
