/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***********************************************************************************************
//                   CalibInfo - Class to store the calibration information
//                           ---------------------------------------
//     begin                : 01 11 2023
//     email                : sergi.rodriguez@cern.ch
//***********************************************************************************************

#ifndef PIXCALIB_H
#define PIXCALIB_H

#include "TKey.h"
#include "TString.h"
#include "TFile.h"
#include "TH1F.h"
#include "TF1.h"
#include "TH2F.h"
#include "TDirectoryFile.h"
#include "TGraphErrors.h"
#include "ChargeCalibration/common/PixelMapping.h"
#include "ChargeCalibration/pixel/tools/CalibFrontEndInfo.h"

#include <map>
#include <iostream>
#include <array>
#include <cmath>




class Calib {
    public:
        Calib (int whichPart, bool saveFile, std::string moduleName = "") {
            m_whichPart = whichPart;
            
            if(saveFile){
                m_savefile = saveFile;
                m_wFile = std::make_unique<TFile>(m_layers.at(whichPart)+".HIST.root","RECREATE");
            }
            if( std::strcmp(moduleName.c_str(), "") != 0 ){
                m_runOneMOD = true;
                m_testMOD = moduleName;
            }
        };
        ~ Calib (){
            if(m_savefile){
                m_wFile->Write(0,TObject::kOverwrite);
                m_wFile->Close();
            }
        };
        bool fillThresholds(const pix::PixelMapping &pm, const std::string &inThrFile, std::map<unsigned int , std::vector<std::unique_ptr<CalibFrontEndInfo>> > &map_info);
        bool fillTiming    (const pix::PixelMapping &pm, const std::string &inTimFile, std::map<unsigned int , std::vector<std::unique_ptr<CalibFrontEndInfo>> > &map_info);
        bool totFitting    (const pix::PixelMapping &pm, const std::string &inTimFile, std::map<unsigned int , std::vector<std::unique_ptr<CalibFrontEndInfo>> > &map_info);
        
    private:
        
        bool m_savefile = false;
        std::unique_ptr<TFile> m_wFile; 
        
        bool m_runOneMOD = false;
        TString m_testMOD = "";
        
        static constexpr float m_chi_error = 0.05;
        
        // privated vars 
        int m_whichPart = -1;
        const std::array<TString, 4> m_MODprefixes{"L0", "L1", "L2", "D"};
        const std::array<TString, 4> m_layers{"Blayer", "L1", "L2", "Disk"};
        
        static constexpr int   m_etaBins  = 144;
        static constexpr int   m_phiBins  = 320;
        
        static constexpr int   m_thrnbins = 200;
        static constexpr float m_thrLo    = 0.;
        static constexpr float m_thrHi    = 6000.;
        static constexpr float m_sigLo    = 0.;
        static constexpr float m_sigHi    = 500;  
        
        static constexpr int   m_timnbins = 300;
        static constexpr float m_timLo    = 1000.;
        static constexpr float m_timHi    = 7000.; 
                
        static constexpr int   m_totnbins    = 255;
        static constexpr float m_totLo       = 0.;
        static constexpr float m_totHi       = 255.;
        static constexpr int   m_totsigNBins = 100;
        static constexpr float m_totsigLo    = 0.;
        static constexpr float m_totsigHi    = 1.;        
         
        // injected charges
        static constexpr int m_nFE = 16;  
        static constexpr int m_ncharge = 21;  
        static constexpr std::array<float, m_ncharge> m_chargeArr{   3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000, 12000, 14000, 16000, 18000, 20000, 25000};
        static constexpr std::array<float, m_ncharge> m_chargeErrArr{   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,     0,     0,     0,     0,     0,     0,     0};  
        
        // injection starting point for fit
        static constexpr int m_qthresh = 5;
        
        // privated functions 
        int chipId(int iphi, int ieta);
        int pixelType(int iphi, int ieta, bool isForTOT = false);
        TIter getRodIterator(const TFile & inputFile);
        TIter getModuleIterator( TDirectoryFile* rodDir);
        TH2F* get2DHistogramFromPath( TDirectoryFile* rodDir, const TString & moduleName, const TString & histName, int charge=-1);
        bool moduleInPart(const TString & modName);
        std::vector<float> getParams(const TF1 *f, unsigned int params);
        std::vector<float> getParams_quality(const TF1 *f);
        bool reFit_normalPix(std::vector<float> &params, std::vector<float> &q, std::vector<float> &qerr, std::vector<float> &tot, std::vector<float> &toterr, std::vector<float> &sig, std::vector<float> &sigerr, const unsigned int fe);
        void graphTitles(const std::unique_ptr<TGraphErrors> &graph, const std::string &name, const std::string &Yname);
        
        class  funcTot {
            public:
                // use constructor to customize your function object
                double operator() (double *x, double *par) {
                    double ret = 9.9e10;
                    double num = x[0]+par[1];
                    double denom = x[0]+par[2];
                    if (denom != 0.0) ret = par[0]*(num/denom);
                    return ret;
                }
        };
        
        class  funcDisp {
            public:
                // use constructor to customize your function object
                double operator() (double *x, double *par) {
                    double ret = 9.9e10;
                    ret = par[0]+par[1]*x[0];
                    return ret;
                }
        };
    
};

#endif
