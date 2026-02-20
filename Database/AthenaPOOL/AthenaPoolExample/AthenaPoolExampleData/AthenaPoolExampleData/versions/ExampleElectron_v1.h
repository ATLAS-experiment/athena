/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// $Id: ExampleElectron.h
#ifndef ATHENAPOOLEXAMPLEDATA_EXAMPLEELECTRON_V1_H
#define ATHENAPOOLEXAMPLEDATA_EXAMPLEELECTRON_V1_H

#include "AthContainers/AuxElement.h"
#include "AthenaKernel/BaseInfo.h"

namespace xAOD {

class ExampleElectron_v1 : public SG::AuxElement {
 public:
  // getter
  double pt() const;
  float charge() const;

  // setter
  void setPt(double p);
  void setCharge(float c);

};  // class

}  // namespace xAOD

// Declare inheritance to SG
SG_BASE(xAOD::ExampleElectron_v1, SG::AuxElement);

#endif  // ATHENAPOOLEXAMPLEDATA_EXAMPLEELECTRON_V1_H
