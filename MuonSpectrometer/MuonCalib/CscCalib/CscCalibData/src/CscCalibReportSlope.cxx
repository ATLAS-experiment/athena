/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CscCalibData/CscCalibReportSlope.h"
#include "GaudiKernel/MsgStream.h"
#include "AthenaKernel/getMessageSvc.h"

#include "TGraphErrors.h"
#include "TH1I.h"
#include "TProfile.h"
#include <utility>

/* default constructor */
CscCalibReportSlope::CscCalibReportSlope()
{ }

/* full constructor */
CscCalibReportSlope::CscCalibReportSlope(std::string label) :  
  CscCalibReportBase::CscCalibReportBase(std::move(label))
{ }

CscCalibReportSlope::~CscCalibReportSlope()
{
}


void CscCalibReportSlope::setBitHists(std::vector<TH1I*>&& someBitHists)
{
    m_bitHists = std::move(someBitHists);
}

const std::vector<TH1I*>& CscCalibReportSlope::getBitHists() const
{
    return m_bitHists;
}

void CscCalibReportSlope::setCalGraphs(std::vector<TGraphErrors*>&& someCalGraphs)
{
  m_calGraphs = std::move(someCalGraphs);
}

const std::vector<TGraphErrors*>& CscCalibReportSlope::getCalGraphs() const
{
  return m_calGraphs;
}

void CscCalibReportSlope::setAmpProfs( std::map<int,TProfile *>&& someAmpProfs )
{
  m_ampProfs = std::move(someAmpProfs);
}

void CscCalibReportSlope::setFitResults( std::vector<float>&& someFitResults){
  m_fitResults = std::move(someFitResults);
}

const std::map<int,TProfile*>& CscCalibReportSlope::getAmpProfs() const
{
  return m_ampProfs;
}

void CscCalibReportSlope::setPulsedChambers( std::set<int>&& somePulsedChambers)
{
  m_pulsedChambers = std::move(somePulsedChambers);
}
    

const std::set<int>& CscCalibReportSlope::getPulsedChambers() const
{
  return m_pulsedChambers;
}

const std::vector<float>& CscCalibReportSlope::getFitResults() const{
  return m_fitResults;
}

