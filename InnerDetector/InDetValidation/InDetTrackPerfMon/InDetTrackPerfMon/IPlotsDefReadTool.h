/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_IPLOTSDEFREADTOOL_H
#define INDETTRACKPERFMON_IPLOTSDEFREADTOOL_H

/**
 * @file    IPlotsDefReadTool.h
 * @brief   Interface for plots definition reading
 *          tools in this package
 * @author  Marco Aparo <marco.aparo@cern.ch>
 * @date    22 June 2023
**/

/// Athena include(s)
#include "AsgTools/IAsgTool.h"

/// STD includes
#include <vector>


namespace IDTPM {

  /// Forward-declaring SinglePlotDefinition
  class SinglePlotDefinition;


  class IPlotsDefReadTool : virtual public asg::IAsgTool {

  public:

    ASG_TOOL_INTERFACE( IDTPM::IPlotsDefReadTool )

    /// Parse input pltos defnitions && returns
    /// vector of SinglePlotDefinition
    virtual std::vector< SinglePlotDefinition > getPlotsDefinitions() const = 0;

  }; // class IPlotsDefReadTool

} // namespace IDTPM

#endif // > !INDETTRACKPERFMON_IPLOTSDEFREADTOOL_H
