/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CscCalibData/CscCalibReportPed.h"
#include "TH1I.h"
#include "TH2F.h"

#include <utility>

/* default constructor */
CscCalibReportPed::CscCalibReportPed()
{ }

/* full constructor */
CscCalibReportPed::CscCalibReportPed(std::string label) :  
    CscCalibReportBase::CscCalibReportBase(std::move(label))
{ }

CscCalibReportPed::~CscCalibReportPed()
{
}

void CscCalibReportPed::setPedAmpHists(std::vector<TH1I*>&&  somePedAmpHists)
{
  m_pedAmpHists = std::move(somePedAmpHists);
}
        
void CscCalibReportPed::setSampHists(std::vector< std::vector<TH1I*> >&& someSampHists)
{
  m_sampHists = std::move(someSampHists);
}

void CscCalibReportPed::setBitHists(std::vector<TH1I*>&& someBitHists)
{
  m_bitHists = std::move(someBitHists);
}

void CscCalibReportPed::setBitCorrelation(std::vector<TH2F*>&& somebitCorrelation)
{
  m_bitCorrelation = std::move(somebitCorrelation);
}

const std::vector<TH1I*>& CscCalibReportPed::getPedAmpHists() const
{
    return m_pedAmpHists;
}

const std::vector<std::vector<TH1I*> >& CscCalibReportPed:: getSampHists() const
{
  return m_sampHists;
}

const std::vector<TH1I*>& CscCalibReportPed::getBitHists() const
{
    return m_bitHists;
}

const std::vector<TH2F*>& CscCalibReportPed::getBitCorrelation() const
{
    return m_bitCorrelation;
}

//**setOnlineTHoldTests*///
void CscCalibReportPed::setOnlineTHoldTests(std::vector<int>&& onlineTests){
  m_onlineTHoldTests = std::move(onlineTests);
}

/**setOnlineTholdTests - contains number of times a channel's sample went above online threshold*/
const std::vector<int>& CscCalibReportPed::getOnlineTHoldTests() const{
  return m_onlineTHoldTests;
}
