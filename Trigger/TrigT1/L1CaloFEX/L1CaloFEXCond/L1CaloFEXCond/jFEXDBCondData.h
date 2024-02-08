/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//             Interface for jFEXDBCondData - Tool to read the COOL DB for jFEX
//                              -------------------
//     begin                : 01 08 2023
//     email                : Sergi.Rodriguez@cern.ch
//***************************************************************************

#ifndef jFEXDBCondData_H
#define jFEXDBCondData_H

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
#include <unordered_map>

namespace LVL1 {

class jFEXDBCondData
{

    public:
        /* Main constructor */
        jFEXDBCondData(){};
        /* Main desconstructor */
        ~jFEXDBCondData(){};

        /** Getters **/
        int  get_jJCalibParam(int module, int range) const;
        const std::vector<int> &  get_jJCalibParams(int module) const;
        bool get_doPileUpJet() const;
        bool get_doPileUpMet() const;
        int  get_PUThrLowEm() const;
        int  get_PUThrHighEm() const;
        int  get_PUThrLowHadLar() const;
        int  get_PUThrHighHadLar() const;
        int  get_PUThrLowHadHecOverlap() const;
        int  get_PUThrHighHadHecOverlap() const;
        int  get_PUThrLowHadTrex() const;
        int  get_PUThrHighHadTrex() const;
        int  get_PUThrLowFcal() const;
        int  get_PUThrHighFcal() const;
        std::array<uint16_t,4>  get_NoiseCuts(uint16_t onlineID) const;
        std::array<uint16_t,4>  get_PileUpValues(uint16_t onlineID) const;

        /** Setters **/
        void set_jJCalibParam(const std::vector<std::vector<int>>& params);
        void set_doPileUpJet(bool PileUpCorrectionJet);
        void set_doPileUpMet(bool PileUpCorrectionMET);
        void set_PUThrLowEm(int PileUpThresholdLowEm);
        void set_PUThrHighEm(int PileUpThresholdHighEm);
        void set_PUThrLowHadLar(int PileUpThresholdLowHadLar);
        void set_PUThrHighHadLar(int PileUpThresholdHighHadLar);
        void set_PUThrLowHadHecOverlap(int PileUpThresholdLowHadHecOverlap);
        void set_PUThrHighHadHecOverlap(int PileUpThresholdHighHadHecOverlap);
        void set_PUThrLowHadTrex(int PileUpThresholdLowHadTrex);
        void set_PUThrHighHadTrex(int PileUpThresholdHighHadTrex);
        void set_PUThrLowFcal(int PileUpThresholdLowFcal);
        void set_PUThrHighFcal(int PileUpThresholdHighFcal);
        void set_NoiseCuts(std::unordered_map< uint16_t, std::array<uint16_t,4> > NoiseCuts);
        void set_PileUpValues(std::unordered_map< uint16_t, std::array<uint16_t,4> > PileUpWeight);
        void set_sendDefaults(bool sendDefaults);
        
        
    private:
    
        bool m_sendDefaults = true;
        
        // Values for jJ Calibration
        std::vector<std::vector<int>> m_jJCalibParams =
        {   //for ET based calibration : <20  <30  <40  <50  <65  <80 <110 <150 <inf  GeV (9 bins)
            //for eta based calibration: based on per-FPGA local eta indices (up to 25 bins)
            std::vector<int>(25,0),// jFEX 0  FCal
            std::vector<int>(25,0),// jFEX 1  Central
            std::vector<int>(25,0),// jFEX 2  Central
            std::vector<int>(25,0),// jFEX 3  Central
            std::vector<int>(25,0),// jFEX 4  Central
            std::vector<int>(25,0) // jFEX 5  FCal
        };

        // Apply pileup on met or jet?
        bool m_PileUpCorrectionJet = false;
        bool m_PileUpCorrectionMET = false;
        
        // upper/lower thresholds to calculate pile-up 
        int m_PileUpThresholdLowEm = 0;
        int m_PileUpThresholdHighEm = 0;
        int m_PileUpThresholdLowHadLar = 0;
        int m_PileUpThresholdHighHadLar =  0;
        int m_PileUpThresholdLowHadHecOverlap = 0;
        int m_PileUpThresholdHighHadHecOverlap = 0;
        int m_PileUpThresholdLowHadTrex = 0;
        int m_PileUpThresholdHighHadTrex = 0;
        int m_PileUpThresholdLowFcal = 0;
        int m_PileUpThresholdHighFcal = 0;
        
        // Noise values for EM and HAD 
        // the map contains: key: OnlineID and the array contains {CutJetEM, CutJetHad, CutMetEM, CutMetHad} in that order!
        std::unordered_map< uint16_t, std::array<uint16_t,4> > m_NoiseCuts;
        std::array<uint16_t,4> m_NoiseCuts_default= {0,0,0,0};
        
        // PileUp values for EM and HAD 
        // the map contains: key: OnlineID and the array contains {CutJetEM, CutJetHad, CutMetEM, CutMetHad} in that order!
        std::unordered_map< uint16_t, std::array<uint16_t,4> > m_PileUpWeight;
        std::array<uint16_t,4> m_PileUpWeight_default= {0,0,0,0};
        
        std::array<int,2> DecodeOnlineID(uint16_t onlineID) const;

};



}//end of namespace

CLASS_DEF   ( LVL1::jFEXDBCondData , 264634186 , 1 )
CONDCONT_DEF( LVL1::jFEXDBCondData , 39175606 );

#endif
