/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file InDetPlotBase.cxx
 * @author shaun roe
 **/

#include "InDetPlotBase.h"
#include "AthenaBaseComps/AthCheckMacros.h"

#include "TEfficiency.h"

#include <cmath>

namespace {
  bool
  validArguments(const float arg) {
    return not (std::isnan(arg));
  }

  bool
  validArguments(const float arg, const float arg2) {
    return not (std::isnan(arg) or std::isnan(arg2));
  }

  bool
  validArguments(const float arg, const float arg2, const float arg3) {
    return not (std::isnan(arg) or std::isnan(arg2) or std::isnan(arg3));
  }
}


InDetPlotBase::InDetPlotBase(InDetPlotBase* pParent, const std::string& dirName) :
  PlotBase(pParent, dirName),
  AthMessaging("InDetPlotBase"),
  m_histoDefSvc("HistogramDefinitionSvc", "InDetPlotBase")
{
}

void
InDetPlotBase::book(TH1*& pHisto, const SingleHistogramDefinition& hd) {
  if (hd.isValid()) {
    pHisto = Book1D(hd.name, hd.allTitles, hd.nBinsX, hd.xAxis.first, hd.xAxis.second, false);
  }
  }
void
InDetPlotBase::book(TProfile*& pHisto, const SingleHistogramDefinition& hd) {
  if (hd.isValid()) {
    pHisto = BookTProfile(hd.name, hd.allTitles, hd.nBinsX, hd.xAxis.first, hd.xAxis.second, hd.yAxis.first,
                          hd.yAxis.second, false);
  }
  }
void
InDetPlotBase::book(TProfile2D*& pHisto, const SingleHistogramDefinition& hd) {
  if (hd.isValid()) {
    pHisto = BookTProfile2D(hd.name, hd.allTitles, hd.nBinsX, hd.xAxis.first, hd.xAxis.second, hd.nBinsY, hd.yAxis.first,
                          hd.yAxis.second, false);
  }
  }
void
InDetPlotBase::book(TH2*& pHisto, const SingleHistogramDefinition& hd) {
  if (hd.isValid()) {
    pHisto = Book2D(hd.name, hd.allTitles, hd.nBinsX, hd.xAxis.first, hd.xAxis.second, hd.nBinsY, hd.yAxis.first,
                    hd.yAxis.second, false);
  }
  }
/**/
void
InDetPlotBase::book(TEfficiency*& pHisto, const SingleHistogramDefinition& hd) {
  if (hd.isValid()) {
    if(hd.nBinsY==0) {
      pHisto = BookTEfficiency(hd.name, hd.allTitles, hd.nBinsX, hd.xAxis.first, hd.xAxis.second, false);
    } else {
      pHisto = BookTEfficiency(hd.name, hd.allTitles, hd.nBinsX, hd.xAxis.first, hd.xAxis.second, hd.nBinsY, hd.yAxis.first, hd.yAxis.second, false);
    }
  }
  }

void
InDetPlotBase::fillHisto(TProfile* pTprofile, const float bin, const float weight, const float weight2) {
  if (pTprofile and validArguments(bin, weight)) {
    pTprofile->Fill(bin, weight,weight2);
  }
}

void
InDetPlotBase::fillHisto(TProfile2D* pTprofile, const float xval, const float yval, const float weight, const float weight2) {
  if (pTprofile and validArguments(xval,yval, weight) and validArguments(weight2)) {
    pTprofile->Fill(xval,yval, weight,weight2);
  }
}

//
void
InDetPlotBase::fillHisto(TH1* pTh1, const float value) {
  if (pTh1 and validArguments(value)) {
    pTh1->Fill(value);
  }
}

void
InDetPlotBase::fillHisto(TH1* pTh1, const float value, const float weight) {
  if (pTh1 and validArguments(value)) {
    pTh1->Fill(value, weight);
  }
}

//
void
InDetPlotBase::fillHisto(TH2* pTh2, const float xval, const float yval) {
  if (pTh2 and validArguments(xval, yval)) {
    pTh2->Fill(xval, yval);
  }
}

//
void
InDetPlotBase::fillHisto(TH2* pTh2, const float xval, const float yval, const float weight) {
  if (pTh2 and validArguments(xval, yval)) {
    pTh2->Fill(xval, yval, weight);
  }
}

void
InDetPlotBase::fillHisto(TH3* pTh3, const float xval, const float yval, const float zval) {
  if (pTh3 and validArguments(xval, yval, zval)) {
    pTh3->Fill(xval, yval, zval);
  }
}

void
InDetPlotBase::fillHisto(TEfficiency* pTeff,  const float value, const bool accepted, float weight) {
  if (pTeff and validArguments(value)) {
    if(weight==1.) pTeff->Fill(accepted, value); // To get proper error estimate when possible
    else pTeff->FillWeighted(accepted, weight, value);
  }
}

void
InDetPlotBase::fillHisto(TEfficiency* eff2d, const float xvalue, const float yvalue, const bool accepted, const float weight) {
  if (eff2d and validArguments(xvalue, yvalue)) {
    if(weight==1.) eff2d->Fill(accepted, xvalue, yvalue);
    else eff2d->FillWeighted(accepted, weight, xvalue, yvalue);
  }
}

/**/
SingleHistogramDefinition
InDetPlotBase::retrieveDefinition(std::string_view histoIdentifier, std::string_view folder, std::string_view nameOverride) {

  ATH_CHECK( m_histoDefSvc.retrieve(), {} );

  bool folderDefault = (folder.empty() or folder == "default");
  SingleHistogramDefinition s = m_histoDefSvc->definition(histoIdentifier, folder);
  // "default" and empty string should be equivalent
  if (folderDefault and s.empty()) {
    const std::string otherDefault = (folder.empty()) ? ("default") : "";
    s = m_histoDefSvc->definition(histoIdentifier, otherDefault);
  }
  if (s.empty()) {
    ATH_MSG_WARNING("Histogram definition is empty for identifier " << histoIdentifier);
  }
  if (!nameOverride.empty()){
    s.name = nameOverride; 
  }
  return s;
}
