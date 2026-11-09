/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARFEBHEADERCONTAINER_H
#define LARFEBHEADERCONTAINER_H

#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "LArRawEvent/LArFebHeader.h"

/**
   @class LArFebHeaderContainer
   @brief Container class for LArFebHeader
   * 
   * Publicly inherits from ATHENA ObjectVector<br>
   * @author Walter Lampl
   
*/
class LArFebHeaderContainer : public DataVector<LArFebHeader> {

 public :
 
  /** @brief Constructor */
  using DataVector<LArFebHeader>::DataVector;
   
/**
  * destructor 
  */
  virtual ~LArFebHeaderContainer() { }
private:    

  /** @link aggregation 
       * @supplierCardinality 1..*
       * @clientRole holds
       * @clientCardinality 1*/
  /*#  LArFebHeader lnkLArFebHeader; */
} ;


CLASS_DEF(LArFebHeaderContainer,1295914285,0)

#endif
