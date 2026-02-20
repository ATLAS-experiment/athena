/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAPOOLEXAMPLEALGORITHMS_READCOND_H
#define ATHENAPOOLEXAMPLEALGORITHMS_READCOND_H

/** @file ReadCond.h
 *  @brief This file contains the class definition for the ReadCond class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 *  $Id: ReadCond.h,v 1.1 2008-12-10 21:28:11 gemmeren Exp $
 **/

#include "AthenaBaseComps/AthAlgorithm.h"

namespace AthPoolEx {

/** @class AthPoolEx::ReadCond
 *  @brief This class provides an example for reading conditions data objects from Pool.
 **/
class ReadCond : public AthAlgorithm {
public:
   ReadCond(const std::string& name, ISvcLocator* pSvcLocator);
   virtual ~ReadCond() = default;

public:
   /// Gaudi Service Interface method implementations:
   virtual StatusCode initialize() override final;
   virtual StatusCode execute() override final;
   virtual StatusCode finalize() override final;
};

} // end AthPoolEx namespace

#endif
