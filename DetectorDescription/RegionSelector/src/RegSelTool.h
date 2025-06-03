/// emacs: this is -*- c++ -*-
///
///   @class RegSelTool RegSelTool.h
/// 
///          This is the Region Selector tool for the ID And muon spectrometer 
///          tables
///     
///   @author Mark Sutton
///
///   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
///

#ifndef REGIONSELECTOR_REGSELTOOL_H
#define REGIONSELECTOR_REGSELTOOL_H

// interface includes
#include "IRegionSelector/IRegSelTool.h"
#include "IRegionSelector/IRoiDescriptor.h"

// spam
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/MsgStream.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include <string>
#include <iostream>
#include <vector>
#include <cstdint>

#include "IRegionSelector/IRegSelLUTCondData.h"

class RegSelModule;
class RegSelSiLUT;
class IInterface;


class RegSelTool : public extends<AthAlgTool, IRegSelTool> {

public:
  using base_class::base_class;

  //! Destructor.
  virtual ~RegSelTool() override;

  //! @method initialize, loads lookup tables for retrieve %Identifier %Hash and ROBID 
  virtual StatusCode initialize() override;

protected:

  //! @method lookup, actually retrieve the lookup table as conditions data

  const IRegSelLUT* lookup( const EventContext& ctx ) const override;
    
private:

  //! Declare properties
  Gaudi::Property<bool> m_dumpTable{ this, "WriteTable", false, "write out maps to files for debugging" };
  Gaudi::Property<bool> m_initialised{ this, "Initialised", false, "flag to determine whether the corresponding subsystem is initilised" };

  SG::ReadCondHandleKey<IRegSelLUTCondData> m_tableKey{ this, "RegSelLUT", "Tool_Not_Initalised", "Region Selector lookup table" };

};

#endif // REGIONSELECTOR_REGSELTOOL_H
