#ifndef FEMUONHELPER_H
#define FEMUONHELPER_H

#include "AsgMessaging/AsgMessaging.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODPFlow/FlowElementContainer.h"

class FEMuonHelper : public asg::AsgMessaging {

public:
    FEMuonHelper ();
    ~FEMuonHelper () {};

    /** Verify if at least one combined muon in a list passes the relevant quality criteria
    Details of muon working points are here:
    https://twiki.cern.ch/twiki/bin/view/Atlas/MuonSelectionTool
    */
    bool checkMuonLinks(const std::vector < ElementLink< xAOD::MuonContainer > >& FE_MuonLinks, const std::string& qualityString ) const;

    TLorentzVector adjustNeutralCaloEnergy(const std::vector<double>& clusterMuonEnergyFracs,const xAOD::FlowElement& theFE) const;


};
#endif