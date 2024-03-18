/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***********************************************************************************************
//                   CalibFrontEndInfo - Class to store the calibration information
//                           ---------------------------------------
//     begin                : 01 11 2023
//     email                : sergi.rodriguez@cern.ch
//***********************************************************************************************

#include "CalibFrontEndInfo.h"

std::stringstream CalibFrontEndInfo::printDBformat()    const {
    
    std::stringstream mytext;
    
    if(m_FEid == 0) mytext << m_MODid_str << " " << m_MODid << "\n";
    
    mytext  << "I" << m_FEid
            << " " << m_NormalTheshold << " " << m_NormalRms << " " << m_NormalNoise << " " << m_NormalIntime 
            << " " << m_LongTheshold   << " " << m_LongRms   << " " << m_LongNoise   << " " << m_LongIntime 
            << " " << m_GangedTheshold << " " << m_GangedRms << " " << m_GangedNoise << " " << m_GangedIntime
            
            << " " << m_NormalFitParams.at(0) << " " << m_NormalFitParams.at(1) << " " << m_NormalFitParams.at(2)
            << " " << m_LongFitParams.at(0)   << " " << m_LongFitParams.at(1)   << " " << m_LongFitParams.at(2)  
            << " " << m_SigFitParams.at(0)    << " " << m_SigFitParams.at(1) ;          
    
    return mytext;
    
};

void CalibFrontEndInfo::printBeautyformat()    const {

    if(m_FEid == 0) printf("ROD: %s -> MOD: %s\n",m_RODid_str.c_str(),m_MODid_str.c_str());
    printf("I%02d | %4d %3d %3d %4d | %4d %3d %3d %4d | %4d %3d %3d %4d | %11.4e %11.4e %11.4e (%6.1f/%6.1f) | %11.4e %11.4e %11.4e  (%6.1f/%6.1f) | %11.4e %11.4e  (%6.1f/%6.1f) ---> %d\n",

           m_FEid,

           m_NormalTheshold, m_NormalRms, m_NormalNoise, m_NormalIntime,
           m_LongTheshold  , m_LongRms  , m_LongNoise  , m_LongIntime  ,
           m_GangedTheshold, m_GangedRms, m_GangedNoise, m_GangedIntime,

           m_NormalFitParams.at(0), m_NormalFitParams.at(1), m_NormalFitParams.at(2) , m_NormalFitParamsQuality.at(0) , m_NormalFitParamsQuality.at(1),
           m_LongFitParams.at(0)  , m_LongFitParams.at(1)  , m_LongFitParams.at(2)   , m_LongFitParamsQuality.at(0)   , m_LongFitParamsQuality.at(1)  ,
           
           m_SigFitParams.at(0)   , m_SigFitParams.at(1)   , m_SigFitParamsQuality.at(0) , m_SigFitParamsQuality.at(1),
           m_times_fitted
          );
    return;
};


void CalibFrontEndInfo::printVals()    const {

    printf("******************************************************************************************************\n");
    printf("%-6s: %s - %-9s:%5d / %s - %-12s:%3d\n", "ROD ID", m_RODid_str.c_str(), "Module ID", m_MODid,m_MODid_str.c_str(), "Front End ID", m_FEid);

    printf("-----------------------------------------------------------------------\n");
    printf("|  %-9s  |  %9s  |  %9s  |  %9s  |  %9s  |\n","", "Threshold", "   RMS", " Noise", " Intime"      );
    printf("-----------------------------------------------------------------------\n");
    printf("|  %-9s  |  %9d  |  %9d  |  %9d  |  %9d  |\n","Normal", m_NormalTheshold, m_NormalRms, m_NormalNoise, m_NormalIntime);
    printf("|  %-9s  |  %9d  |  %9d  |  %9d  |  %9d  |\n","Long"  , m_LongTheshold  , m_LongRms  , m_LongNoise  , m_LongIntime  );
    printf("|  %-9s  |  %9d  |  %9d  |  %9d  |  %9d  |\n","Ganged", m_GangedTheshold, m_GangedRms, m_GangedNoise, m_GangedIntime);
    printf("-----------------------------------------------------------------------\n\n");

    if(m_NormalFitParams.size() == 3 ) {
        printf("%-40s: %11.2f / %11.2f / %11.2f\n","Fitting parameters for normal pixel",m_NormalFitParams.at(0),m_NormalFitParams.at(1),m_NormalFitParams.at(2));
    }
    else {
        printf("%-40s: %11s / %11s / %11s\n","Fitting parameters for normal pixel","not set","not set","not set");
    }
    if(m_LongFitParams.size() == 3 ) {
        printf("%-40s: %11.2f / %11.2f / %11.2f\n","Fitting parameters for long pixel",m_LongFitParams.at(0),m_LongFitParams.at(1),m_LongFitParams.at(2));
    }
    else {
        printf("%-40s: %11s / %11s / %11s\n","Fitting parameters for long pixel","not set","not set","not set");
    }
    if(m_SigFitParams.size() == 2 ) {
        printf("%-40s: %11.7f / %.5e\n\n","Fitting parameters for sig (linear fit)",m_SigFitParams.at(0),m_SigFitParams.at(1));
    }
    else {
        printf("%-40s: %11s / %11s\n\n","Fitting parameters for sig (linear fit)","not set","not set");
    }
    return;
};

//Prints the information stored in case of error - needs more implementation.
void CalibFrontEndInfo::printMODerr()    const {

    bool error = false;

    int vals[12] = {m_NormalTheshold, m_NormalRms, m_NormalNoise, m_NormalIntime,
                    m_LongTheshold  , m_LongRms  , m_LongNoise  , m_LongIntime  ,
                    m_GangedTheshold, m_GangedRms, m_GangedNoise, m_GangedIntime,
                   };

    for(const auto &val : vals) {
        if(val <= 0) error = true;
    }

    if(error) printVals();

    return;
};
        
        

