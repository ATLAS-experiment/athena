/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_SINGLEPLOTDEFINITION_H
#define INDETTRACKPERFMON_SINGLEPLOTDEFINITION_H

/**
 * @file    SinglePlotDefinition.h
 * @brief   Class to store (internally) each plot definition
 *          in this package (originally based on the
 *          SingleHistogramDefinition class of IDPVM)
 * @author  Marco Aparo <marco.aparo@cern.ch>, Shaun Roe <shaun.roe@cern.ch>
 * @date    21 March 2024
**/

/// STL include(s)
#include <string>
#include <vector>
#include <utility> // std::pair


namespace IDTPM {

  class SinglePlotDefinition {

  public:

    /// typedef for axes limits: (lower bound, upper bound)
    typedef std::pair< float, float >  axesLimits_t;

    /// Parametrised Constructor
    SinglePlotDefinition(
      const std::string& name = "", const std::string& type = "", const std::string& title = "",
      const std::string& xTitle = "",
      unsigned int nBinsX = 0, float xLow = 0., float xHigh = 0.,
      bool doLogLinBinsX = false, const std::vector<float>& xBinsVec = {},
      const std::vector< std::string >& xBinLabelsVec = {},
      const std::string& yTitle = "",
      unsigned int nBinsY = 0, float yLow = 0., float yHigh = 0.,
      bool doLogLinBinsY = false, const std::vector<float>& yBinsVec = {},
      const std::vector< std::string >& yBinLabelsVec = {},
      const std::string& zTitle = "",
      unsigned int nBinsZ = 0, float zLow = 0., float zHigh = 0.,
      bool doLogLinBinsZ = false, const std::vector<float>& zBinsVec = {},
      const std::vector< std::string >& zBinLabelsVec = {},
      const std::string& folder = "" );

    /// Default destructor
    ~SinglePlotDefinition() = default;

    /// = operator
    SinglePlotDefinition& operator=( const SinglePlotDefinition& ) = default;

    /// ------------------------
    /// ---- getter methods ----
    /// ------------------------
    const std::string& name() const { return m_name; }
    const std::string& type() const { return m_type; }
    const std::string& title() const { return m_title; }
    const std::string& xTitle() const { return m_xTitle; }
    const std::string& yTitle() const { return m_yTitle; }
    const std::string& zTitle() const { return m_zTitle; }
    unsigned int nBinsX() const { return m_nBinsX; }
    unsigned int nBinsY() const { return m_nBinsY; }
    unsigned int nBinsZ() const { return m_nBinsZ; }
    float xLow() const {  return m_xAxis.first; }
    float xHigh() const { return m_xAxis.second; }
    float yLow() const {  return m_yAxis.first; }
    float yHigh() const { return m_yAxis.second; }
    float zLow() const {  return m_zAxis.first; }
    float zHigh() const { return m_zAxis.second; }
    bool doLogLinBinsX() const { return m_doLogLinBinsX; }
    bool doLogLinBinsY() const { return m_doLogLinBinsY; }
    bool doLogLinBinsZ() const { return m_doLogLinBinsZ; }
    const std::vector<float>& xBinsVec() const { return m_xBinsVec; }
    const std::vector<float>& yBinsVec() const { return m_yBinsVec; }
    const std::vector<float>& zBinsVec() const { return m_zBinsVec; }
    bool doVarBinsX() const { return ( ! m_xBinsVec.empty() ); }
    bool doVarBinsY() const { return ( ! m_yBinsVec.empty() ); }
    bool doVarBinsZ() const { return ( ! m_zBinsVec.empty() ); }
    const std::vector< std::string >& xBinLabelsVec() const { return m_xBinLabelsVec; }
    const std::vector< std::string >& yBinLabelsVec() const { return m_yBinLabelsVec; }
    const std::vector< std::string >& zBinLabelsVec() const { return m_zBinLabelsVec; }
    const std::string& folder() const { return m_folder; }
    bool isEmpty() const { return m_empty; }
    const std::string& identifier() const { return m_identifier; }
    const std::string& plotDigest() const { return m_plotDigest; }
    const std::string& titleDigest() const { return m_titleDigest; }

    bool isValid() const;

    /// ------------------------
    /// ---- setter methods ----
    /// ------------------------
    /// These need and are followed by the recomputing of the digest strings
    void name( std::string_view name_s ) { m_name = name_s; digest(); }
    void type( std::string_view type_s ) { m_type = type_s; digest(); }
    void title( std::string_view title_s ) { m_title = title_s; digest(); }
    void xTitle( std::string_view xTitle_s ) { m_xTitle = xTitle_s; digest(); }
    void yTitle( std::string_view yTitle_s ) { m_yTitle = yTitle_s; digest(); }
    void zTitle( std::string_view zTitle_s ) { m_zTitle = zTitle_s; digest(); }
    void nBinsX( float nBinsX_f ) { m_nBinsX = nBinsX_f; digest(); }
    void nBinsY( float nBinsY_f ) { m_nBinsY = nBinsY_f; digest(); }
    void nBinsZ( float nBinsZ_f ) { m_nBinsZ = nBinsZ_f; digest(); }
    void xLimits( float xLow, float xHigh ) { m_xAxis = std::make_pair( xLow, xHigh ); digest(); }
    void yLimits( float yLow, float yHigh ) { m_yAxis = std::make_pair( yLow, yHigh ); digest(); } 
    void zLimits( float zLow, float zHigh ) { m_zAxis = std::make_pair( zLow, zHigh ); digest(); }
    void doLogLinBinsX( bool doLogLinBinsX_b ) { m_doLogLinBinsX = doLogLinBinsX_b; }
    void doLogLinBinsY( bool doLogLinBinsY_b ) { m_doLogLinBinsY = doLogLinBinsY_b; }
    void doLogLinBinsZ( bool doLogLinBinsZ_b ) { m_doLogLinBinsZ = doLogLinBinsZ_b; }
    void setxBinsVec( const std::vector<float>& vec ) { if( ! vec.empty() ) {
      m_xBinsVec = vec; m_nBinsX = vec.size() - 1;
      m_xAxis = std::make_pair( vec.front(), vec.back() ); digest(); } }
    void setyBinsVec( const std::vector<float>& vec ) { if( ! vec.empty() ) {
      m_yBinsVec = vec; m_nBinsY = vec.size() - 1;
      m_yAxis = std::make_pair( vec.front(), vec.back() ); digest(); } }
    void setzBinsVec( const std::vector<float>& vec ) { if( ! vec.empty() ) {
      m_zBinsVec = vec; m_nBinsZ = vec.size() - 1;
      m_zAxis = std::make_pair( vec.front(), vec.back() ); digest(); } }
    void setxBinLabelsVec( const std::vector< std::string >& vec ) { if( ! vec.empty() ) {
      m_xTitle = "";
      m_xBinLabelsVec = vec;  m_nBinsX = vec.size();
      m_xAxis = std::make_pair( 0, m_nBinsX ); digest(); } }
    void setyBinLabelsVec( const std::vector< std::string >& vec ) { if( ! vec.empty() ) {
      m_yTitle = "";
      m_yBinLabelsVec = vec;  m_nBinsY = vec.size();
      m_yAxis = std::make_pair( 0, m_nBinsY ); digest(); } }
    void setzBinLabelsVec( const std::vector< std::string >& vec ) { if( ! vec.empty() ) {
      m_zTitle = "";
      m_zBinLabelsVec = vec;  m_nBinsZ = vec.size();
      m_zAxis = std::make_pair( 0, m_nBinsZ ); digest(); } }
    void folder( std::string_view folder_s ) { m_folder = folder_s; digest(); } 
    void setEmpty( bool empty = true ) { m_empty = empty; }

    /// recompute m_identifier
    void redoIdDigest();

    /// recompute m_plotDigest
    void redoPlotDigest();

    /// recompute m_titleDigest
    void redoTitleDigest();

    /// recompute m_is*D
    void redoTypeDigest();

    /// recompute m_identifier, m_plotDigest, m_titleDigest, m_is*D
    void digest() {
      redoIdDigest();
      redoPlotDigest();
      redoTitleDigest();
      redoTypeDigest();
    }

  private:

    /// main members
    std::string m_name;
    std::string m_type;
    std::string m_title, m_xTitle, m_yTitle, m_zTitle;
    unsigned int m_nBinsX, m_nBinsY, m_nBinsZ;
    axesLimits_t m_xAxis, m_yAxis, m_zAxis;
    bool m_doLogLinBinsX, m_doLogLinBinsY, m_doLogLinBinsZ;
    std::vector<float> m_xBinsVec, m_yBinsVec, m_zBinsVec;
    std::vector< std::string > m_xBinLabelsVec, m_yBinLabelsVec, m_zBinLabelsVec;
    std::string m_folder;

    /// status member
    bool m_empty;

    /// derived members, i.e. dependant on main members
    std::string m_identifier;
    std::string m_plotDigest;
    std::string m_titleDigest;
    bool m_is1D, m_is2D, m_is3D;

  }; // class SinglePlotDefinition

} // namespace IDTPM

#endif // > !INDETTRACKPERFMON_SINGLEPLOTDEFINITION_H
