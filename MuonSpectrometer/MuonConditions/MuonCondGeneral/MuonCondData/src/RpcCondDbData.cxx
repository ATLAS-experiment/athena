/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondData/RpcCondDbData.h"

// --- writing identifiers -------
void RpcCondDbData::setEfficiency(Identifier chamberId, double efficiency){
    m_cachedEfficiency.insert(std::make_pair(chamberId, efficiency));
}
void RpcCondDbData::setFracClusterSize1(Identifier chamberId, double fcs){
    m_cachedFracClusterSize1.insert(std::make_pair(chamberId, fcs));
}
void RpcCondDbData::setFracClusterSize2(Identifier chamberId, double fcs){
    m_cachedFracClusterSize2.insert(std::make_pair(chamberId, fcs));
}
void RpcCondDbData::setFracDeadStrip(Identifier chamberId, double fds){
    m_cachedFracDeadStrip.insert(std::make_pair(chamberId, fds));
}
void RpcCondDbData::setGapEfficiency(Identifier chamberId, double efficiency){
    m_cachedGapEfficiency.insert(std::make_pair(chamberId, efficiency));
}
void RpcCondDbData::setMeanClusterSize(Identifier chamberId, double mcs){
    m_cachedMeanClusterSize.insert(std::make_pair(chamberId, mcs));
}
void RpcCondDbData::setProjectedTrack(Identifier chamberId, int projectedTracks){
    m_cachedProjectedTracks.insert(std::make_pair(chamberId, projectedTracks));
}
// setStripTime
void RpcCondDbData::setStripTime(Identifier stripId, const std::vector<double>& time){
    if(m_cachedStripTime.count(stripId)) return;
    m_cachedStripTime[stripId] = time[0];
}

// --- reading identifiers -------
std::optional<double> RpcCondDbData::getEfficiency(const Identifier & Id) const {
	auto itr = m_cachedEfficiency.find(Id);
    return itr==m_cachedEfficiency.end() ? std::nullopt : std::make_optional<double>(itr->second);
} 
std::optional<double> RpcCondDbData::getFracClusterSize1(const Identifier & Id) const{
   auto itr = m_cachedFracClusterSize1.find(Id);
   return itr==m_cachedFracClusterSize1.end() ? std::nullopt : std::make_optional<double>(itr->second);
} 
std::optional<double> RpcCondDbData::getFracClusterSize2(const Identifier & Id) const{
	auto itr = m_cachedFracClusterSize2.find(Id);
    return itr ==m_cachedFracClusterSize2.end() ? std::nullopt : std::make_optional<double>(itr->second);
} 
std::optional<double> RpcCondDbData::getFracDeadStrip(const Identifier & Id) const {
	auto itr = m_cachedFracDeadStrip.find(Id);
    return itr == m_cachedFracDeadStrip.end() ? std::nullopt : std::make_optional<double>(itr->second);
} 
std::optional<double> RpcCondDbData::getGapEfficiency(const Identifier & Id) const {
    auto itr = m_cachedGapEfficiency.find(Id);
    return itr == m_cachedGapEfficiency.end() ? std::nullopt : std::make_optional<double>(itr->second);
} 
std::optional<double> RpcCondDbData::getMeanClusterSize(const Identifier & Id) const {
	auto itr = m_cachedMeanClusterSize.find(Id);
    return itr==m_cachedMeanClusterSize.end() ? std::nullopt : std::make_optional<double>(itr->second);
} 
std::optional<int> RpcCondDbData::getProjectedTrack(const Identifier & Id) const{
    auto itr = m_cachedProjectedTracks.find(Id);
    return itr ==m_cachedProjectedTracks.end() ? std::nullopt : std::make_optional<int>(itr->second);
} 
std::optional<double> RpcCondDbData::getStripTime(const Identifier & Id) const {
	auto itr = m_cachedStripTime.find(Id);
    return itr == m_cachedStripTime.find(Id) ? std::nullopt : std::make_optional<double>(itr->second);
}