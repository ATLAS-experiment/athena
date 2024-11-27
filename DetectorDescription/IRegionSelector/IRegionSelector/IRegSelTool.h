// emacs: this is -*- c++ -*-
//
//   IRegSelTool.h        
// 
//    Interface for the new local RegionSelector tool
// 
//
// Copyright (C) 2012-2019 CERN for the benefit of the ATLAS collaboration


#ifndef IREGIONSELECTOR_IREGSELTOOL_H
#define IREGIONSELECTOR_IREGSELTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

#include "Identifier/IdentifierHash.h"
#include <vector>
#include <stdint.h>

#include "IRegionSelector/IRegSelLUT.h"
#include "GaudiKernel/IAlgTool.h"



/**
 * @class IRegSelTool
 * @brief 
 **/

class IRegSelTool : virtual public IAlgTool, virtual public IRegSelLUT {

public: 

  /// InterfaceID
  DeclareInterfaceID( IRegSelTool, 1, 0 ); 

  //! @method lookup, actually retrieve the lookup table as conditions data
  /// did this ever work ?? It is the wrong class for the calorimeter tables 
  virtual const IRegSelLUT* lookup() const = 0;                                                                                                                              

  virtual const IRegSelLUT* lookup( const EventContext& ctx ) const = 0;

};



#endif //  IREGIONSELECTOR_IREGSELTOOL_H
