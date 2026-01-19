/*
 Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//             Interface for jFEXBDTool - Tool to read the COOL DB for jFEX
//                              -------------------
//     begin                : 01 08 2023
//     email                : Sergi.Rodriguez@cern.ch
//***************************************************************************
#include "L1CaloFEXCond/jFEXDBCondData.h"

namespace LVL1 {

    
int jFEXDBCondData::get_jJCalibParam(int module, int range) const {
    return m_jJCalibParams[module][range];
}

const std::vector<int>& jFEXDBCondData::get_jJCalibParams(int module) const {
    return m_jJCalibParams[module];
}

bool jFEXDBCondData::get_doPileUpJet() const {
    return m_PileUpCorrectionJet;
}

bool jFEXDBCondData::get_doPileUpMet() const {
    return m_PileUpCorrectionMET;
}

int jFEXDBCondData::get_PUThrLowEm() const {
    return m_PileUpThresholdLowEm;
}

int jFEXDBCondData::get_PUThrHighEm() const {
    return m_PileUpThresholdHighEm;
}

int jFEXDBCondData::get_PUThrLowHadLar() const {
    return m_PileUpThresholdLowHadLar;
}

int jFEXDBCondData::get_PUThrHighHadLar() const {
    return m_PileUpThresholdHighHadLar;
}

int jFEXDBCondData::get_PUThrLowHadHecOverlap() const {
    return m_PileUpThresholdLowHadHecOverlap;
}

int jFEXDBCondData::get_PUThrHighHadHecOverlap() const {
    return m_PileUpThresholdHighHadHecOverlap;
}

int jFEXDBCondData::get_PUThrLowHadTrex() const {
    return m_PileUpThresholdLowHadTrex;
}

int jFEXDBCondData::get_PUThrHighHadTrex() const {
    return m_PileUpThresholdHighHadTrex;
}

int jFEXDBCondData::get_PUThrLowFcal() const {
    return m_PileUpThresholdLowFcal;
}

int jFEXDBCondData::get_PUThrHighFcal() const {
    return m_PileUpThresholdHighFcal;
}   

std::array<uint16_t,4> jFEXDBCondData::get_NoiseCuts(uint16_t onlineID) const {

    if(m_sendDefaults){
        //Sending default values!
        auto [gEta, gPhi] = DecodeOnlineID(onlineID);
        gEta = gEta < 0 ? std::abs(gEta+1) : gEta;
        if(gEta < 15){ // This is LATOME and Tile towers ( |eta| < 1.5 )
            //{CutJetEM, CutJetHad, CutMetEM, CutMetHad}
            auto p = m_NoiseCuts.find(0x00f0);
            if (p != m_NoiseCuts.end()) return p->second;
            return m_NoiseCuts_default;
        }
        else if (gEta < 32){  // This is LATOME  EMB/EMEC and HEC towers (1.5 < |eta| < 3.1)
            auto p = m_NoiseCuts.find(0x0f00);
            if (p != m_NoiseCuts.end()) return p->second;
            return m_NoiseCuts_default;
        }
        else{ // This is FCAL towers (|eta| > 3.1)
            auto p = m_NoiseCuts.find(0xf000);
            if (p != m_NoiseCuts.end()) return p->second;
            return m_NoiseCuts_default;
        }             
    } else {
        auto itr = m_NoiseCuts.find(onlineID);
        if(itr == m_NoiseCuts.end()) {
            return m_NoiseCuts_default;
        }

        return itr->second;
    }
}  

std::array<uint16_t,4> jFEXDBCondData::get_PileUpValues(uint16_t onlineID) const {
    
    if(m_sendDefaults){
        auto p = m_PileUpWeight.find(0x0000);
        if (p!= m_PileUpWeight.end()) return p->second;
        return m_PileUpWeight_default;
    }
    else{
        auto itr = m_PileUpWeight.find(onlineID);

        if(itr == m_PileUpWeight.end()) {
            return m_PileUpWeight_default;
        }

        return itr->second;         
    }
} 

void jFEXDBCondData::set_jJCalibParam(const std::vector<std::vector<int>>& params){
    m_jJCalibParams = params;
}

void jFEXDBCondData::set_doPileUpJet(bool PileUpCorrectionJet){
    m_PileUpCorrectionJet = PileUpCorrectionJet;
}

void jFEXDBCondData::set_doPileUpMet(bool PileUpCorrectionMET){
    m_PileUpCorrectionMET = PileUpCorrectionMET;
}

void jFEXDBCondData::set_PUThrLowEm(int PileUpThresholdLowEm){
    m_PileUpThresholdLowEm = PileUpThresholdLowEm;
}

void jFEXDBCondData::set_PUThrHighEm(int PileUpThresholdHighEm){
    m_PileUpThresholdHighEm = PileUpThresholdHighEm;
}

void jFEXDBCondData::set_PUThrLowHadLar(int PileUpThresholdLowHadLar){
    m_PileUpThresholdLowHadLar = PileUpThresholdLowHadLar;
}

void jFEXDBCondData::set_PUThrHighHadLar(int PileUpThresholdHighHadLar){
    m_PileUpThresholdHighHadLar = PileUpThresholdHighHadLar;
}

void jFEXDBCondData::set_PUThrLowHadHecOverlap(int PileUpThresholdLowHadHecOverlap){
    m_PileUpThresholdLowHadHecOverlap = PileUpThresholdLowHadHecOverlap;
}

void jFEXDBCondData::set_PUThrHighHadHecOverlap(int PileUpThresholdHighHadHecOverlap){
    m_PileUpThresholdHighHadHecOverlap = PileUpThresholdHighHadHecOverlap;
}

void jFEXDBCondData::set_PUThrLowHadTrex(int PileUpThresholdLowHadTrex){
    m_PileUpThresholdLowHadTrex = PileUpThresholdLowHadTrex;
}

void jFEXDBCondData::set_PUThrHighHadTrex(int PileUpThresholdHighHadTrex){
    m_PileUpThresholdHighHadTrex = PileUpThresholdHighHadTrex;
}

void jFEXDBCondData::set_PUThrLowFcal(int PileUpThresholdLowFcal){
    m_PileUpThresholdLowFcal = PileUpThresholdLowFcal;
}

void jFEXDBCondData::set_PUThrHighFcal(int PileUpThresholdHighFcal){
    m_PileUpThresholdHighFcal = PileUpThresholdHighFcal;
}

void jFEXDBCondData::set_NoiseCuts(std::unordered_map< uint16_t, std::array<uint16_t,4> > NoiseCuts){
    
    for(auto const& [key, array] : NoiseCuts){
        m_NoiseCuts[key]=array;
    }
}
void jFEXDBCondData::set_PileUpValues(std::unordered_map< uint16_t, std::array<uint16_t,4> > PileUpWeight){
        
    for(auto const& [key, array] : PileUpWeight){
        m_PileUpWeight[key]=array;
    }
}

void jFEXDBCondData::set_sendDefaults(bool sendDefaults){
    m_sendDefaults = sendDefaults;
}

        // It returns the GlobalEta and GlobalPhi
std::array<int,2> jFEXDBCondData::DecodeOnlineID(uint16_t onlineID) const{
    
    int global_eta = (onlineID >> 8) - 0x80;
    int global_phi = onlineID & 0xff;
    return {global_eta, global_phi};
}

}// end of namespace LVL1
