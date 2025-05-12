/*                                                                             
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration                              
*/

///////////////////////////////////////////////////////////////////////////                          
// Utils for the main sTGCRawDataMonAlg.cxx                                                            
// Part of StgcRawDataMonAlg.h                                                                         
// see StgcRawDataMonAlg.cxx                                
///////////////////////////////////////////////////////////////////////////                      
         
#include "StgcRawDataMonitoring/StgcRawDataMonAlg.h"

int sTgcRawDataMonAlg::getFEBs(int eta, int layer) const {
  int feb=-1;
  if (std::abs(eta)==1) feb=layer-1;
  else if(std::abs(eta)==2) feb=layer+7;
  else if(std::abs(eta)==3) feb=layer+15;
    
  //ATH_MSG_DEBUG("FEB: " << feb << " Eta: " << eta << " Layer: " << layer);  
  return feb;
}

int sTgcRawDataMonAlg::getSectors(const Identifier& id) const { 
  return m_idHelperSvc -> sector(id)*(m_idHelperSvc -> stationEta(id) > 0 ? 1. : -1.);
}

int sTgcRawDataMonAlg::getLayer(int multiplet, int gasGap) const {
  return 4*(multiplet - 1) + gasGap;
}

int32_t sTgcRawDataMonAlg::sourceidToSector(uint32_t sourceid, bool isSideA) const {
  uint32_t sectorNumber = sourceid & 0xf;
  return (isSideA) ? sectorNumber + 1: -sectorNumber - 1;
}

int sTgcRawDataMonAlg::getSignedPhiId(const uint32_t phiid) const {
  // 1 bit of sign (0 = positive) followed by 5 bits of phiid
  constexpr size_t nbitsPhi{5};
  constexpr size_t mask{(1 << nbitsPhi) - 1};
  return std::pow(-1, phiid >> nbitsPhi) * (phiid & mask);
}

std::optional<Identifier> sTgcRawDataMonAlg::getPadId(uint32_t sourceid, uint32_t pfeb, uint32_t tdschan) const {
  bool isValid = false;
  const int side = (decoder::isA(sourceid)) ? 1 : -1;
  const auto vmm = tdschan / NVMMCHAN + FIRSTPFEBVMM;
  const auto vmmchan = tdschan % NVMMCHAN;
  const auto sec = decoder::sector(sourceid);
  const auto& help = m_idHelperSvc -> stgcIdHelper();
  const auto pad_id = help.channelID(help.elementID(decoder::offlineStationName(sec),
						    decoder::offlineStationAbsEta(pfeb) * side,
						    decoder::offlineStationPhi(sourceid)),
				                    decoder::offlineMultilayer(pfeb), 
				                    decoder::offlineGasgap(pfeb),
				                    Muon::nsw::OFFLINE_CHANNEL_TYPE_PAD,
				                    decoder::offlineChannelNumber(sec, pfeb, vmm, vmmchan), isValid);
  
  if (!isValid) {
    ATH_MSG_WARNING("Pad Identifier not valid, skipping");
    return std::nullopt;
  }
  
  return std::make_optional(pad_id);
}

std::optional<std::tuple<Identifier, const Trk::RIO_OnTrack*>> sTgcRawDataMonAlg::getRotIdAndRotObject(const Trk::TrackStateOnSurface* trkState) const {
  if (!trkState->type(Trk::TrackStateOnSurface::Measurement)) return std::nullopt;

  Identifier surfaceId = (trkState) -> surface().associatedDetectorElementIdentifier();
  if(!m_idHelperSvc -> issTgc(surfaceId)) return std::nullopt;
  
  const Trk::MeasurementBase* meas = trkState->measurementOnTrack();
  if(!meas) return std::nullopt;
  
  const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(meas);
  if(!rot) return std::nullopt;
  
  Identifier rot_id = rot -> identify();

  if(!rot_id.is_valid()) {
    ATH_MSG_WARNING("Invalid identifier found in Trk::RIO_OnTrack");
    return std::nullopt;
  }
  
  return std::make_tuple(rot_id, rot);
}
    
std::optional<Identifier> sTgcRawDataMonAlg::getRotId(const Trk::TrackStateOnSurface* trkState) const {
  std::optional<std::tuple<Identifier, const Trk::RIO_OnTrack*>> status = getRotIdAndRotObject(trkState);
  if (!status.has_value()) return std::nullopt;
  std::tuple<Identifier, const Trk::RIO_OnTrack*> rotIDtuple = status.value();
  
  return std::make_optional(std::get<Identifier>(rotIDtuple));
}

std::optional<std::tuple<int, int, std::string, std::string, int>> sTgcRawDataMonAlg::getPadEtaPhiTuple(uint32_t sourceid, uint32_t pfeb, uint32_t tdschan) const {
  bool isValid = false;
  const int side = (decoder::isA(sourceid)) ? 1 : -1;
  const auto vmm = tdschan / NVMMCHAN + FIRSTPFEBVMM;
  const auto vmmchan = tdschan % NVMMCHAN;
  const auto sec = decoder::sector(sourceid);
  
  mapper mapperSTG;
  
  int sector_type = decoder::isLarge(sec) ? 1 : 0;
  int feb_radius = decoder::radius(pfeb);
  int layer = decoder::layer(pfeb);
  int channel_number = mapperSTG.channel_number(Muon::nsw::OFFLINE_CHANNEL_TYPE_PAD, sector_type, feb_radius, layer, vmm, vmmchan);
 
  const auto& help = m_idHelperSvc -> stgcIdHelper(); 
  const auto pad_id = help.channelID(help.elementID(decoder::offlineStationName(sec),
						    decoder::offlineStationAbsEta(pfeb) * side,
						    decoder::offlineStationPhi(sourceid)),
				     decoder::offlineMultilayer(pfeb), 
				     decoder::offlineGasgap(pfeb),
				     Muon::nsw::OFFLINE_CHANNEL_TYPE_PAD,
				     channel_number, isValid);

  
  if (!isValid) {
    ATH_MSG_WARNING("Pad Identifier not valid, skipping");
    return std::nullopt;
  }
  
  int padPhi = help.padPhi(pad_id);
  int padEta = help.padEta(pad_id);
  int padPhiMax = help.padPhiMax();
  int padEtaMax = help.padEtaMax();

  int padPhiTotal = padPhi + (padPhiMax + 1)*(decoder::offlineStationPhi(sourceid) - 1);
  int padEtaTotal = padEta + (padEtaMax + 1)*(decoder::offlineStationAbsEta(pfeb) - 1);
  std::string sideName = (side == 1) ? "A" : "C";
  std::string sizeName = (decoder::offlineStationName(sec) == "STS") ? "S" : "L";
    
  return std::make_optional(std::make_tuple(padPhiTotal, padEtaTotal, sideName, sizeName, layer + 1));
}

std::optional<double> sTgcRawDataMonAlg::band2theta(double rPosAtNSW, const MuonGM::MuonDetectorManager* muonDetectorManagerObject) const {
  const auto& help = m_idHelperSvc -> stgcIdHelper();
  bool isValid = false;
  
  const Identifier maxNSWZid = m_idHelperSvc -> stgcIdHelper().channelID("STL", 1, help.stationPhiMin(), help.multilayerMax(), help.gasGapMax(), sTgcIdHelper::sTgcChannelTypes::Pad, 1, isValid);

  if (!isValid) {
		ATH_MSG_WARNING("Identifier for maximum value of NSW global Z-coordinate is invalid!");
		return std::nullopt;
  }

  Amg::Vector3D posNSW{Amg::Vector3D::Zero()};
  (muonDetectorManagerObject -> getsTgcReadoutElement(maxNSWZid)) -> stripGlobalPosition(maxNSWZid, posNSW);
  float posNSWZ = posNSW.z();
  
  double theta = std::atan(rPosAtNSW/posNSWZ);
  return std::make_optional(theta);
}

std::optional<double> sTgcRawDataMonAlg::band2eta(double rPosAtNSW, const MuonGM::MuonDetectorManager* muonDetectorManagerObject) const {
  std::optional<double> status = band2theta(rPosAtNSW, muonDetectorManagerObject);
  if (!status.has_value()) return std::nullopt;
  double theta = status.value();
  if (!std::isnormal(theta/2.)) return std::nullopt;
  double eta = -std::log(std::tan(theta/2.));
  return std::make_optional(eta);
}

std::optional<double> sTgcRawDataMonAlg::rPosAtNsw2eta(double rPosAtNSW, bool isA, const MuonGM::MuonDetectorManager* muonDetectorManagerObject) const {
  std::optional<double> status = band2eta(rPosAtNSW, muonDetectorManagerObject);
  if (!status.has_value()) return std::nullopt;
  double band2eta = status.value();
  if (isA) {
    return  std::make_optional(band2eta);
  }
  else {
    return std::make_optional(-band2eta);
  }
}

std::optional<double> sTgcRawDataMonAlg::bandId2eta(int bandid, bool isLarge, bool isA, const MuonGM::MuonDetectorManager* muonDetectorManagerObject) const {
  double rPosAtNSW = MuonGM::sTgcReadoutElement::triggerBandIdToRadius(isLarge,bandid);
  std::optional<double> status = rPosAtNsw2eta(rPosAtNSW, isA, muonDetectorManagerObject);
  if (!status.has_value()) return std::nullopt;
  double rPosAtNsw2eta = status.value();
  
  return std::make_optional(rPosAtNsw2eta);
}

double sTgcRawDataMonAlg::triggersectorphiid2phi(uint32_t sourceid, int trigger_phiid) const {
  MuonSectorMapping sectorMapping;
  double trigger_sector_phicenter = sectorMapping.sectorPhi((sourceid & 0xf) + 1);
  double trigger_phi = trigger_sector_phicenter + (trigger_phiid*9.)/1000.;
  /// Sector 8 (A09/C09) is a special case, is where the phi changes sign (-pi to +pi) 
  if((sourceid & 0xf) == 8 && trigger_phiid < 0) {
    trigger_phi = -trigger_sector_phicenter + (trigger_phiid*9./1000.);
  }
  return trigger_phi;
}
