/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAPOOLEXAMPLEALGORITHMS_PASSNONEFILTER_H
#define ATHENAPOOLEXAMPLEALGORITHMS_PASSNONEFILTER_H

/** @file PassNoneFilter.h
 *  @brief class definition for PassNoneFilter
 *  @author Jack Cranshaw (Jack.Cranshaw@cern.ch)
 *  $Id: PassNoneFilter.h,v 1.1 2008-12-10 21:28:11 gemmeren Exp $
 **/

#include "AthenaBaseComps/AthAlgorithm.h"

namespace AthPoolEx {

/** @class PassNoneFilter
 *  @brief Simple Filter algorithm which simply sets pass=false for all events.
 **/
class PassNoneFilter : public AthAlgorithm {
public:
   PassNoneFilter(const std::string& name, ISvcLocator* pSvcLocator);
   virtual ~PassNoneFilter() = default;

   virtual StatusCode initialize() override final;
   virtual StatusCode execute() override final;
   virtual StatusCode finalize() override final;
};

} // end namespace AthPoolEx

#endif
