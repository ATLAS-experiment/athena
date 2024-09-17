/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CSCCALIBDATA_CSCCALIBREPORTBASE_H
#define CSCCALIBDATA_CSCCALIBREPORTBASE_H

/**************************************************************************
Package: MuonSpectrometer/MuonCalib/CscCalib/CscCalibData
Name: CscCalibReportBase.h
Author: Caleb Parnell-Lampen
Date & Place: July 4, 2008, University of Arizona

Base class to hold info for a class. Mostly an interface class for which 
different types of reports can be derived from.
****************************************************************************/
#include <string>

class CscCalibReportBase
{

    public:

        /* default constructor */
        CscCalibReportBase();

        /* full constructor */
        CscCalibReportBase(std::string theReportName);

        /* destructor */
        virtual ~CscCalibReportBase();
        
        /* set the calibration constants */
        void setLabel  (const std::string & label);

        /* get report name*/
        const std::string& getLabel() const;
       
    private:
        /* Simple label for defining the type of report*/
        std::string m_label;
};

#endif
