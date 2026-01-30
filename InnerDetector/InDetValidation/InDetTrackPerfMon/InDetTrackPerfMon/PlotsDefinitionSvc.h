/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_PLOTSDEFINITIONSVC_H
#define INDETTRACKPERFMON_PLOTSDEFINITIONSVC_H

/**
 * @file    PlotsDefinitionSvc.h
 * @brief   AthService to hold (and propagate) the definition
 *          of the monitoring plots in this package
 *          (based on HistogramDefinitionSvc.h of the IDPVM package)
 * @author  Marco Aparo <marco.aparo@cern.ch>, Shaun Roe <shaun.roe@cern.ch>
 * @date    19 June 2023
**/

/// Athena include(s)
#include "AsgServices/AsgService.h"

/// Gaudi include(s)
#include "GaudiKernel/ToolHandle.h"

/// Local include(s)
#include "InDetTrackPerfMon/IPlotsDefinitionSvc.h"
#include "IPlotsDefReadTool.h"
#include "InDetTrackPerfMon/SinglePlotDefinition.h"


class PlotsDefinitionSvc :
    public extends<asg::AsgService, IPlotsDefinitionSvc> {

public:

  /// Constructor
  PlotsDefinitionSvc( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor
  virtual ~PlotsDefinitionSvc() = default;

  /// initialize
  virtual StatusCode initialize() override;

  /// finalize
  virtual StatusCode finalize() override;

  /// Get the plot definition
  virtual const IDTPM::SinglePlotDefinition& definition(
      const std::string& identifier ) const override;

  /// Update the map with a new entry
  StatusCode update( const IDTPM::SinglePlotDefinition& def );
 
private:

  plotsDefMap_t m_plotsDefMap;

  IDTPM::SinglePlotDefinition m_nullDef;

  ToolHandle< IDTPM::IPlotsDefReadTool > m_plotsDefReadTool {
      this, "PlotsDefReadTool", "IDTPM::InDetTrackPerfMon/IPlotsDefReadTool", "Tool to read plots definitions from parsed list of strings" };

  std::string m_anaTag;
  
}; // class PlotsDefinitionSvc

#endif // > !INDETTRACKPERFMON_PLOTSDEFINITIONSVC_H
