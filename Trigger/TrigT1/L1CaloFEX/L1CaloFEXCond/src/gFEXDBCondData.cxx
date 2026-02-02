/*
 Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//             Interface for gFEXBDTool - Tool to read the COOL DB for gFEX
//                              -------------------
//     begin                : 23 10 2024
//     email                : jared.little@cern.ch
//***************************************************************************

#include "L1CaloFEXCond/gFEXDBCondData.h"

namespace LVL1 {

const std::string& gFEXDBCondData::get_FWVersion() const{
    return m_FWVersion;
}
const std::array<int,12>& gFEXDBCondData::get_Aslopes() const{
    return m_Aslopes;
}
const std::array<int,12>& gFEXDBCondData::get_Bslopes() const{
    return m_Bslopes;
}
const std::array<int,12>& gFEXDBCondData::get_Cslopes() const{
    return m_Cslopes;
}

const std::array<int,12>& gFEXDBCondData::get_AnoiseCuts() const{
    return m_AnoiseCuts;
}
const std::array<int,12>& gFEXDBCondData::get_BnoiseCuts() const{
    return m_BnoiseCuts;
}
const std::array<int,12>& gFEXDBCondData::get_CnoiseCuts() const{
    return m_CnoiseCuts;
}

void gFEXDBCondData::set_FWVersion(const std::string& fwVersion){
    m_FWVersion = fwVersion;
}
void gFEXDBCondData::set_Aslopes(const std::array<int,12>& params) {
    m_Aslopes = params;
}
void gFEXDBCondData::set_Bslopes(const std::array<int,12>& params) {
    m_Bslopes = params;
}
void gFEXDBCondData::set_Cslopes(const std::array<int,12>& params) {
    m_Cslopes = params;
}

void gFEXDBCondData::set_AnoiseCuts(const std::array<int,12>& params) {
    m_AnoiseCuts = params;
}
void gFEXDBCondData::set_BnoiseCuts(const std::array<int,12>& params) {
    m_BnoiseCuts = params;
}
void gFEXDBCondData::set_CnoiseCuts(const std::array<int,12>& params) {
    m_CnoiseCuts = params;
}

}// end of namespace LVL1