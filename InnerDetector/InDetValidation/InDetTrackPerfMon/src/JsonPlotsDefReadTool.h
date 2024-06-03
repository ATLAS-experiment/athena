/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_JSONPLOTSDEFREADTOOL_T
#define INDETTRACKPERFMON_JSONPLOTSDEFREADTOOL_T

/**
 * @file   JsonPlotsDefReadTool.h
 * @author Marco Aparo <marco.aparo@cern.ch>
 * @date   26 April 2023
 * @brief  Tool to read/parse plots definitions from Json input format
 */

/// Athena includes
#include "AsgTools/AsgTool.h"

/// Local includes
#include "IPlotsDefReadTool.h"


namespace IDTPM {

  class JsonPlotsDefReadTool :
      public virtual IPlotsDefReadTool,
      public asg::AsgTool {

  public:

    ASG_TOOL_CLASS( JsonPlotsDefReadTool, IPlotsDefReadTool );
   
    /// Constructor 
    JsonPlotsDefReadTool( const std::string& name ) :
        asg::AsgTool( name ) { }

    /// Destructor
    virtual ~JsonPlotsDefReadTool() = default;

    /// Initialize
    virtual StatusCode initialize() override;

    /// Parse input pltos defnitions and returns
    /// vector of SinglePlotDefinition
    virtual std::vector< SinglePlotDefinition > getPlotsDefinitions() const override;

  private:

    /// Tool properties
    StringArrayProperty m_plotsDefs{ this, "PlotsDefs", {}, "Vector of plots definition strings" }; 
    
  }; // class JsonPlotsDefReadTool

} // namespace IDTPM

#endif // > !INDETTRACKPERFMON_JSONPLOTSDEFREADTOOL_T
