/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    JsonPlotsDefReadTool.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>
 **/

/// Local include(s)
#include "JsonPlotsDefReadTool.h"
#include "SinglePlotDefinition.h"

/// Json parsing library
#include <nlohmann/json.hpp>


///--------------------------
///------- Initialize -------
///--------------------------
StatusCode IDTPM::JsonPlotsDefReadTool::initialize()
{
  ATH_MSG_INFO( "Initializing " << name() );
  ATH_CHECK( asg::AsgTool::initialize() );
  return StatusCode::SUCCESS;
}


///---------------------------
///--- getPlotsDefinitions ---
///---------------------------
std::vector< IDTPM::SinglePlotDefinition > 
IDTPM::JsonPlotsDefReadTool::getPlotsDefinitions() const
{
  using json = nlohmann::json;
  using cstr_t = const std::string&;

  std::vector< SinglePlotDefinition > plotDefVec;

  /// Looping m_plotDefs for each histogram defnition string
  for( cstr_t plotDefStr : m_plotsDefs ) {

    ATH_MSG_DEBUG( "Reading plot definition : " << plotDefStr );

    /// parsing json format
    const json& plotDef = json::parse( plotDefStr );

    /// plot name
    cstr_t name = plotDef.contains( "name" ) ?
        plotDef.at( "name" ).get_ref< cstr_t >() : "";

    /// plot type
    cstr_t type = plotDef.contains( "type" ) ?
        plotDef.at( "type" ).get_ref< cstr_t >() : "";

    /// plot folder
    cstr_t folder = plotDef.contains( "folder" ) ?
        plotDef.at( "folder" ).get_ref< cstr_t >() : "";

    /// plot title
    cstr_t title = plotDef.contains( "title" ) ?
        plotDef.at( "title" ).get_ref< cstr_t >() : "";

    /// nBinsX
    unsigned int nBinsX = plotDef.contains( "xAxis_nBins" ) ?
        ( unsigned int )std::stoi(
            plotDef.at( "xAxis_nBins" ).get_ref< cstr_t >() ) : 0;

    /// nBinsY
    unsigned int nBinsY = plotDef.contains( "yAxis_nBins" ) ?
        ( unsigned int )std::stoi(
            plotDef.at( "yAxis_nBins" ).get_ref< cstr_t >() ) : 0;

    /// nBinsZ
    unsigned int nBinsZ = plotDef.contains( "zAxis_nBins" ) ?
        ( unsigned int )std::stoi(
            plotDef.at( "zAxis_nBins" ).get_ref< cstr_t >() ) : 0;

    /// xAxis limits
    float xLow = plotDef.contains( "xAxis_low" ) ?
        std::stof( plotDef.at( "xAxis_low" ).get_ref< cstr_t >() ) : 0.;
    float xHigh = plotDef.contains( "xAxis_high" ) ?
        std::stof( plotDef.at( "xAxis_high" ).get_ref< cstr_t >() ) : 0.;

    /// yAxis limits
    float yLow = plotDef.contains( "yAxis_low" ) ?
        std::stof( plotDef.at( "yAxis_low" ).get_ref< cstr_t >() ) : 0.;
    float yHigh = plotDef.contains( "yAxis_high" ) ?
        std::stof( plotDef.at( "yAxis_high" ).get_ref< cstr_t >() ) : 0.;

    /// zAxis limits
    float zLow = plotDef.contains( "zAxis_low" ) ?
        std::stof( plotDef.at( "zAxis_low" ).get_ref< cstr_t >() ) : 0.;
    float zHigh = plotDef.contains( "zAxis_high" ) ?
        std::stof( plotDef.at( "zAxis_high" ).get_ref< cstr_t >() ) : 0.;

    /// xTitle
    cstr_t xTitle = plotDef.contains( "xAxis_title" ) ?
        plotDef.at( "xAxis_title" ).get_ref< cstr_t >() : "";

    /// yTitle
    cstr_t yTitle = plotDef.contains( "yAxis_title" ) ?
        plotDef.at( "yAxis_title" ).get_ref< cstr_t >() : "";

    /// zTitle
    cstr_t zTitle = plotDef.contains( "zAxis_title" ) ?
        plotDef.at( "zAxis_title" ).get_ref< cstr_t >() : "";

    /// Adding new SinglePlotDefinition to vector
    plotDefVec.emplace_back(
        name, type, title,
        xTitle, nBinsX, xLow, xHigh,
        yTitle, nBinsY, yLow, yHigh,
        zTitle, nBinsZ, zLow, zHigh,
        folder );

    /// Check if plot definition is valid. Removing
    if( not plotDefVec.back().isValid() ) {
      ATH_MSG_WARNING( "Tried to add invalid plot : " <<
                       plotDefVec.back().plotDigest() );
      plotDefVec.pop_back(); // removing from vector
    }

  } // close m_plotDefs loop

  return plotDefVec; 
}
