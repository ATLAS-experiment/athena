/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    JsonPlotsDefReadTool.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>
 **/

/// Local include(s)
#include "InDetTrackPerfMon/JsonPlotsDefReadTool.h"
#include "InDetTrackPerfMon/SinglePlotDefinition.h"

/// Json parsing library
#include <nlohmann/json.hpp>


///--------------------------
///------- Initialize -------
///--------------------------
StatusCode IDTPM::JsonPlotsDefReadTool::initialize()
{
  ATH_MSG_DEBUG( "Initializing " << name() );
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
  using strVec_t = std::vector< std::string >;

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
        getInt( plotDef.at( "xAxis_nBins" ).get_ref< cstr_t >() ) : 0;

    /// nBinsY
    unsigned int nBinsY = plotDef.contains( "yAxis_nBins" ) ?
        getInt( plotDef.at( "yAxis_nBins" ).get_ref< cstr_t >() ) : 0;

    /// nBinsZ
    unsigned int nBinsZ = plotDef.contains( "zAxis_nBins" ) ?
        getInt( plotDef.at( "zAxis_nBins" ).get_ref< cstr_t >() ) : 0;

    /// xAxis limits
    float xLow = plotDef.contains( "xAxis_low" ) ?
        getFloat( plotDef.at( "xAxis_low" ).get_ref< cstr_t >() ) : 0.;
    float xHigh = plotDef.contains( "xAxis_high" ) ?
        getFloat( plotDef.at( "xAxis_high" ).get_ref< cstr_t >() ) : 0.;

    /// yAxis limits
    float yLow = plotDef.contains( "yAxis_low" ) ?
        getFloat( plotDef.at( "yAxis_low" ).get_ref< cstr_t >() ) : 0.;
    float yHigh = plotDef.contains( "yAxis_high" ) ?
        getFloat( plotDef.at( "yAxis_high" ).get_ref< cstr_t >() ) : 0.;

    /// zAxis limits
    float zLow = plotDef.contains( "zAxis_low" ) ?
        getFloat( plotDef.at( "zAxis_low" ).get_ref< cstr_t >() ) : 0.;
    float zHigh = plotDef.contains( "zAxis_high" ) ?
        getFloat( plotDef.at( "zAxis_high" ).get_ref< cstr_t >() ) : 0.;

    /// xAxis doLogLin
    bool xDoLogLinBins(false);
    if( plotDef.contains( "xAxis_doLogLinBins" ) ) {
      std::string xDoLogLinBinsStr = plotDef.at( "xAxis_doLogLinBins" ).get_ref< cstr_t >();
      if( xDoLogLinBinsStr == "true" ) xDoLogLinBins = true;
      else if( xDoLogLinBinsStr == "false" ) xDoLogLinBins = false;
      else ATH_MSG_WARNING( "xAxis_doLogLinBins ! valid" );
    }

    /// yAxis doLogLin
    bool yDoLogLinBins(false);
    if( plotDef.contains( "yAxis_doLogLinBins" ) ) {
      std::string yDoLogLinBinsStr = plotDef.at( "yAxis_doLogLinBins" ).get_ref< cstr_t >();
      if( yDoLogLinBinsStr == "true" ) yDoLogLinBins = true;
      else if( yDoLogLinBinsStr == "false" ) yDoLogLinBins = false;
      else ATH_MSG_WARNING( "yAxis_doLogLinBins ! valid" );
    }

    /// zAxis doLogLin
    bool zDoLogLinBins(false);
    if( plotDef.contains( "zAxis_doLogLinBins" ) ) {
      std::string zDoLogLinBinsStr = plotDef.at( "zAxis_doLogLinBins" ).get_ref< cstr_t >();
      if( zDoLogLinBinsStr == "true" ) zDoLogLinBins = true;
      else if( zDoLogLinBinsStr == "false" ) zDoLogLinBins = false;
      else ATH_MSG_WARNING( "zAxis_doLogLinBins ! valid" );
    }

    /// xAxis bins (variable size)
    strVec_t xBinsStrVec;
    if( plotDef.contains( "xAxis_bins" ) ) xBinsStrVec = plotDef.at( "xAxis_bins" ).get< strVec_t >();
    std::vector< float > xBinsVec;
    for( cstr_t thisBin : xBinsStrVec ) xBinsVec.push_back( getFloat( thisBin ) );
    if( ! xBinsVec.empty() ) {
      /// overwriting binning
      xLow = xBinsVec.front();  xHigh = xBinsVec.back();  nBinsX = xBinsVec.size() - 1;
    }

    /// yAxis bins (variable size)
    strVec_t yBinsStrVec;
    if( plotDef.contains( "yAxis_bins" ) ) yBinsStrVec = plotDef.at( "yAxis_bins" ).get< strVec_t >();
    std::vector< float > yBinsVec;
    for( cstr_t thisBin : yBinsStrVec ) yBinsVec.push_back( getFloat( thisBin ) );
    if( ! yBinsVec.empty() ) {
      /// overwriting binning
      yLow = yBinsVec.front();  yHigh = yBinsVec.back();  nBinsY = yBinsVec.size() - 1;
    }

    /// zAxis bins (variable size)
    strVec_t zBinsStrVec;
    if( plotDef.contains( "zAxis_bins" ) ) zBinsStrVec = plotDef.at( "zAxis_bins" ).get< strVec_t >();
    std::vector< float > zBinsVec;
    for( cstr_t thisBin : zBinsStrVec ) zBinsVec.push_back( getFloat( thisBin ) );
    if( ! zBinsVec.empty() ) {
      /// overwriting binning
      zLow = zBinsVec.front();  zHigh = zBinsVec.back();  nBinsZ = zBinsVec.size() - 1;
    }

    /// xTitle
    cstr_t xTitle = plotDef.contains( "xAxis_title" ) ?
        plotDef.at( "xAxis_title" ).get_ref< cstr_t >() : "";

    /// yTitle
    cstr_t yTitle = plotDef.contains( "yAxis_title" ) ?
        plotDef.at( "yAxis_title" ).get_ref< cstr_t >() : "";

    /// zTitle
    cstr_t zTitle = plotDef.contains( "zAxis_title" ) ?
        plotDef.at( "zAxis_title" ).get_ref< cstr_t >() : "";

    /// xAxis bin labels
    strVec_t xBinLabelsVec;
    if( plotDef.contains( "xAxis_labels" ) ) xBinLabelsVec = plotDef.at( "xAxis_labels" ).get< strVec_t >();
    if( ! xBinLabelsVec.empty() ) {
      /// overwiting binning
      nBinsX = xBinLabelsVec.size(); xLow = 0; xHigh = nBinsX;
    }

    /// yAxis bin labels
    strVec_t yBinLabelsVec;
    if( plotDef.contains( "yAxis_labels" ) ) yBinLabelsVec = plotDef.at( "yAxis_labels" ).get< strVec_t >();
    if( ! yBinLabelsVec.empty() ) {
      /// overwiting binning
      nBinsY = yBinLabelsVec.size(); yLow = 0; yHigh = nBinsY;
    }

    /// zAxis bin labels
    strVec_t zBinLabelsVec;
    if( plotDef.contains( "zAxis_labels" ) ) zBinLabelsVec = plotDef.at( "zAxis_labels" ).get< strVec_t >();
    if( ! zBinLabelsVec.empty() ) {
      /// overwiting binning
      nBinsZ = zBinLabelsVec.size(); zLow = 0; zHigh = nBinsZ;
    }

    /// Adding new SinglePlotDefinition to vector
    plotDefVec.emplace_back(
        name, type, title,
        xTitle, nBinsX, xLow, xHigh, xDoLogLinBins, xBinsVec, xBinLabelsVec,
        yTitle, nBinsY, yLow, yHigh, yDoLogLinBins, yBinsVec, yBinLabelsVec,
        zTitle, nBinsZ, zLow, zHigh, zDoLogLinBins, zBinsVec, zBinLabelsVec,
        folder );

    /// Check if plot definition is valid. Removing
    if( ! plotDefVec.back().isValid() ) {
      ATH_MSG_ERROR( "Removing invalid plot :" <<
                     "\n\t- string: " << plotDefStr <<
                     "\n\t- digest: " << plotDefVec.back().plotDigest() );
      plotDefVec.pop_back(); // removing from vector
    }

  } // close m_plotDefs loop

  return plotDefVec; 
}


///-------------------------
///--- Utility functions ---
///-------------------------
float IDTPM::JsonPlotsDefReadTool::getFloat(
    const std::string& s, float defaultNum ) const
{
  try {
    float f = std::stof(s);
    return f;
  } catch(...) {
    return defaultNum;
  }
}

unsigned int IDTPM::JsonPlotsDefReadTool::getInt(
    const std::string& s, unsigned int defaultNum ) const
{
  return ( static_cast< unsigned int >( getFloat( s, defaultNum ) ) );
}
