/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CSCCALIBDATA_CSCCALIBREPORTPED_H
#define CSCCALIBDATA_CSCCALIBREPORTPED_H

/**************************************************************************
Package: MuonSpectrometer/MuonCalib/CscCalib/CscCalibData
Name: CscCalibReportPed.h
Author: Caleb Parnell-Lampen
Date & Place: July 4, 2008, University of Arizona

Base class to hold info for a class. Mostly an interface class for which 
different types of reports can be derived from. The reports are meant to 
include details about the calibration process, as opposed to the 
CscCalibResults which contain just the simple results of the calibration.
 ****************************************************************************/
#include "CscCalibData/CscCalibReportBase.h"

#include <vector>
#include <string>

class TH1I;
class TH2F;

class CscCalibReportPed : public CscCalibReportBase
{
  private:
    //Pedestal amplitude histograms
    std::vector<TH1I*> m_pedAmpHists;
    std::vector< std::vector<TH1I*> > m_sampHists;
    std::vector<TH1I*> m_bitHists;
    std::vector<TH2F*> m_bitCorrelation;
    std::vector<int>   m_onlineTHoldTests;

  public:

    /* default constructor */
    CscCalibReportPed();
    
    /* delete copy constructor */
    CscCalibReportPed(const CscCalibReportPed &) = delete;
    
    /* delete assignment  */
    CscCalibReportPed& operator =(const CscCalibReportPed &) = delete;

    /* full constructor */
    CscCalibReportPed(std::string label);


    ~CscCalibReportPed();

    /** Set the pedAmpHist vector.  
      @PARAM somePedAmpHists - vector of pedestal histograms.
     */
    void setPedAmpHists(std::vector<TH1I*>&& somePedAmpHists);

    /** Set the sample histogram vector.  
      @PARAM someSampHists - vector of pedestal histograms.
     */

    void setSampHists(std::vector<std::vector<TH1I*> >&& someSampHists);

    /** Set the pedAmpHist vector.  
      @PARAM somePedAmpHists - vector of pedestal histograms.
     */
    void setBitHists(std::vector<TH1I*>&& somePedAmpHists);

    void setBitCorrelation(std::vector<TH2F*>&& somebitCorrelation);




    /**Retrieve pedestal amplitude histogram vector*/
    const std::vector<TH1I*>& getPedAmpHists() const;

    /**Retrieve pedestal  sample amplitude histogram vector*/
    const std::vector<std::vector<TH1I*> >& getSampHists() const;


    /**Retrieve bit histogram vector*/
    const std::vector<TH1I*>& getBitHists() const;

    const std::vector<TH2F*>& getBitCorrelation() const;

    //**setOnlineTHoldTests*///
    void setOnlineTHoldTests(std::vector<int>&& onlineTests);

    /**setOnlineTholdTests - contains number of times a channel's sample went above online threshold*/
    const std::vector<int>& getOnlineTHoldTests() const;

};

#endif
