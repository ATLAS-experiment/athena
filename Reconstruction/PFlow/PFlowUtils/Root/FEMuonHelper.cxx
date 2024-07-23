#include "PFlowUtils/FEMuonHelper.h"

FEMuonHelper::FEMuonHelper() : AsgMessaging("FEMuonHelper") {};

bool FEMuonHelper::checkMuonLinks(const std::vector < ElementLink< xAOD::MuonContainer > >& FE_MuonLinks, const std::string& qualityString) const{

    for (const ElementLink<xAOD::MuonContainer>& MuonLink: FE_MuonLinks){
        if (!MuonLink.isValid()){
            ATH_MSG_WARNING("JetPFlowSelectionAlg encountered an invalid muon element link. Skipping. ");
            continue; 
        }
    
        //Details of muon working points are here:
        //https://twiki.cern.ch/twiki/bin/view/Atlas/MuonSelectionTool
        const xAOD::Muon* muon = *MuonLink;
        xAOD::Muon::Quality quality = xAOD::Muon::VeryLoose;

        if (qualityString == "Loose")  quality = xAOD::Muon::Loose;
        else if (qualityString == "Medium") quality = xAOD::Muon::Medium;
        else if (qualityString == "Tight")  quality = xAOD::Muon::Tight;

        if ( muon->quality() <= quality && muon->muonType() == xAOD::Muon::Combined ){
          return true;
        }    
      }

    return false;
}

TLorentzVector FEMuonHelper::adjustNeutralCaloEnergy(const std::vector<double>& clusterMuonEnergyFracs,const xAOD::FlowElement& theFE) const{

  double totalMuonCaloEnergy = 0.0;         
  for (auto energy : clusterMuonEnergyFracs ) totalMuonCaloEnergy += energy;

  TLorentzVector newP4;
  newP4.SetPxPyPzE(theFE.p4().Px(),theFE.p4().Py(),theFE.p4().Pz(),theFE.e() - totalMuonCaloEnergy);
  return newP4;

}
