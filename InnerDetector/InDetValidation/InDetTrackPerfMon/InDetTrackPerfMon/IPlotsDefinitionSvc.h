/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_IPLOTSDEFINITIONSVC_H
#define INDETTRACKPERFMON_IPLOTSDEFINITIONSVC_H

/**
 * @file    PlotsDefinitionSvc.h
 * @brief   Service interface to hold (and propagate) the definition
 *          of the monitoring plots in this package
 *          (based on IHistogramDefinitionSvc.h of the IDPVM package)
 * @author  Marco Aparo <marco.aparo@cern.ch>, Shaun Roe <shaun.roe@cern.ch>
 * @date    19 June 2023
**/

/// Athena include(s).
#include <AsgServices/IAsgService.h>

/// STL include(s)
#include <string>
#include <unordered_map>


namespace IDTPM {
  class SinglePlotDefinition;
}


class IPlotsDefinitionSvc :
  virtual public asg::IAsgService {

public:

  /// typedef for map definition
  typedef std::unordered_map< std::string, IDTPM::SinglePlotDefinition > plotsDefMap_t;

  /// Creates the InterfaceID && interfaceID() method
  DeclareInterfaceID( IPlotsDefinitionSvc, 1, 0 );

  /// Destructor
  virtual ~IPlotsDefinitionSvc() = default;

  /// Get the plot definition
  virtual const IDTPM::SinglePlotDefinition& definition(
      const std::string& identifier ) const = 0;

}; // class IPlotsDefinitionSvc

#endif // > !INDETTRACKPERFMON_IPLOTSDEFINITIONSVC_H
