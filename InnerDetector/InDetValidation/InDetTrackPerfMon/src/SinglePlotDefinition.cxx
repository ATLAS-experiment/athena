/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    SinglePlotDefinition.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>, Shaun Roe <shaun.roe@cern.ch>
 * @date    21 March 2024
**/

/// Local include(s)
#include "SinglePlotDefinition.h"

/// STL include(s)
#include <limits> // std::numeric_limits
#include <cmath> // std::std::isnan
#include <sstream>


/// -----------------------
/// ----- Constructor -----
/// -----------------------
IDTPM::SinglePlotDefinition::SinglePlotDefinition(
    const std::string& name, const std::string& type, const std::string& title,
    const std::string& xTitle,
    unsigned int nBinsX, float xLow, float xHigh,
    bool doLogLinBinsX, const std::vector<float>& xBinsVec,
    const std::vector< std::string >& xBinLabelsVec,
    const std::string& yTitle,
    unsigned int nBinsY, float yLow, float yHigh,
    bool doLogLinBinsY, const std::vector<float>& yBinsVec,
    const std::vector< std::string >& yBinLabelsVec,
    const std::string& zTitle,
    unsigned int nBinsZ, float zLow, float zHigh,
    bool doLogLinBinsZ, const std::vector<float>& zBinsVec,
    const std::vector< std::string >& zBinLabelsVec,
    const std::string& folder ) :
        m_name( name ), m_type( type ), m_title( title ),
        m_xTitle( xTitle ), m_yTitle( yTitle ), m_zTitle( zTitle ),
        m_nBinsX( nBinsX ), m_nBinsY( nBinsY ), m_nBinsZ( nBinsZ ),
        m_doLogLinBinsX( doLogLinBinsX ), m_doLogLinBinsY( doLogLinBinsY ), m_doLogLinBinsZ( doLogLinBinsZ ),
        m_folder( folder ), m_is1D( false ), m_is2D( false ), m_is3D( false )
{
  /// initialise title = name if title is empty
  if( m_title.empty() ) m_title = m_name;

  /// initialising axes ranges
  m_xAxis = ( nBinsX != 0 ) ? std::make_pair( xLow, xHigh ) :
            std::make_pair( std::numeric_limits<float>::quiet_NaN(),
                            std::numeric_limits<float>::quiet_NaN() );

  m_yAxis = ( nBinsY != 0 ) ? std::make_pair( yLow, yHigh ) :
            std::make_pair( std::numeric_limits<float>::quiet_NaN(),
                            std::numeric_limits<float>::quiet_NaN() );

  m_zAxis = ( nBinsZ != 0 ) ? std::make_pair( zLow, zHigh ) :
            std::make_pair( std::numeric_limits<float>::quiet_NaN(),
                            std::numeric_limits<float>::quiet_NaN() );

  /// Recomputing limits and sizes (for variable bin sizes)
  setxBinsVec( xBinsVec );
  setyBinsVec( yBinsVec );
  setzBinsVec( zBinsVec );

  /// Setting bin labels
  setxBinLabelsVec( xBinLabelsVec );
  setyBinLabelsVec( yBinLabelsVec );
  setzBinLabelsVec( zBinLabelsVec );

  /// Sanity check
  m_empty = not isValid();

  // Re-compute digest strings
  if( not m_empty ) digest();
}


/// ---------------------
/// ------ isValid ------
/// ---------------------
/// Is the histogram definition valid
bool IDTPM::SinglePlotDefinition::isValid() const
{
  /// non-valid plot if name or type are undefined
  if( m_name.empty() or m_type.empty() ) return false;

  /// check if type is valid
  bool sane =
      ( m_type.find( "TH1" ) != std::string::npos ) or
      ( m_type.find( "TH2" ) != std::string::npos ) or
      ( m_type.find( "TH3" ) != std::string::npos ) or
      ( m_type.find( "TProfile" ) != std::string::npos ) or // 1D and 2D
      ( m_type.find( "TEfficiency" ) != std::string::npos ); // 1D and 2D

  /// check if bin numbers are valid
  const bool sensibleXBins = ( m_nBinsX != 0 ) and ( not std::isnan( m_nBinsX ) );
  const bool sensibleYBins = ( m_nBinsY != 0 ) and ( not std::isnan( m_nBinsY ) );
  const bool sensibleZBins = ( m_nBinsZ != 0 ) and ( not std::isnan( m_nBinsZ ) );

  /// check validity of axes limits
  /// NB: if yaxis is left undefined, the limits should be NaN,
  /// but (NaN != NaN) is always true, so an extra check is needed
  const bool sensibleXLimits = ( not std::isnan( m_xAxis.first ) ) and
                               ( not std::isnan( m_xAxis.second ) ) and
                               ( m_xAxis.first != m_xAxis.second );

  const bool sensibleYLimits = ( not std::isnan( m_yAxis.first ) ) and
                               ( not std::isnan( m_yAxis.second ) ) and
                               ( m_yAxis.first != m_yAxis.second );

  const bool sensibleZLimits = ( not std::isnan( m_zAxis.first ) ) and
                               ( not std::isnan( m_zAxis.second ) ) and
                               ( m_zAxis.first != m_zAxis.second );

  /// sanity check for 1D, 2D and 3D plots
  sane = sane and sensibleXBins and sensibleXLimits;

  /// sanity check for 2D and 3D plots
  if( m_is2D ) {
    sane = sane and sensibleYBins and sensibleYLimits;

    /// sanity check for 3D plots
    if( m_is3D ) {
      sane = sane and sensibleZBins and sensibleZLimits;
    }
  }

  return sane;
}


/// --------------------
/// --- redoIdDigest ---
/// --------------------
/// recompute plot identifier, i.e.: "folder/name"
/// or "name" if folder is empty
void IDTPM::SinglePlotDefinition::redoIdDigest()
{
  if( m_folder.empty() ){
    m_identifier = m_name;
  } else {
    /// reduce: "/folder" -> "folder"
    if( m_folder[0] == '/' ) {
      m_folder.erase( m_folder.begin() );
    }
    /// add a slash: "folder" -> "folder/"
    if( m_folder.back() != '/' ) m_folder += "/";
    m_identifier = m_folder + m_name;
  }
}


/// ----------------------
/// --- redoPlotDigest ---
/// ----------------------
/// recompute single-line representation of the plot definition
void IDTPM::SinglePlotDefinition::redoPlotDigest()
{
  std::stringstream ss;
  ss << m_name << " - " << m_type << " - " << m_title
     << " - " << m_xTitle << " : (" << m_nBinsX << ", " << m_xAxis.first << ", " << m_xAxis.second << " )"
     << " - " << m_yTitle << " : (" << m_nBinsY << ", " << m_yAxis.first << ", " << m_yAxis.second << " )"
     << " - " << m_zTitle << " : (" << m_nBinsZ << ", " << m_zAxis.first << ", " << m_zAxis.second << " )";

  m_plotDigest = ss.str(); 
}


/// -----------------------
/// --- redoTitleDigest ---
/// -----------------------
/// recompute single-line representation
/// of the titles (titles separated by the ';' delimiter)
void IDTPM::SinglePlotDefinition::redoTitleDigest()
{
  std::stringstream ss;
  ss << m_title << ";" << m_xTitle << ";" << m_yTitle << ";" << m_zTitle;

  m_titleDigest = ss.str();
}


/// ----------------------
/// --- redoTypeDigest ---
/// ----------------------
/// recompute m_is1D, m_is2D and m_is3D
void IDTPM::SinglePlotDefinition::redoTypeDigest()
{
  m_is1D = ( m_type.find("TH1") != std::string::npos ) or
           ( m_type == "TProfile" ) or ( m_type == "TEfficiency" );

  m_is2D = ( m_type.find("TH2") != std::string::npos ) or
           ( m_type.find("2D") != std::string::npos );

  m_is3D = ( m_type.find("TH3") != std::string::npos ) or
           ( m_type.find("3D") != std::string::npos );
}
