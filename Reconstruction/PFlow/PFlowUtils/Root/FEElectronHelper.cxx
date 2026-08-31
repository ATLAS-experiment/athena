#include "PFlowUtils/FEElectronHelper.h"

static constexpr float GeV = 1000.f;

FEElectronHelper::FEElectronHelper() : AsgMessaging("FEElectronHelper") {};

 bool FEElectronHelper::checkElectronLinks(const std::vector < ElementLink< xAOD::ElectronContainer > >& FE_ElectronLinks, const std::string& qualityString) const{

    for (const ElementLink<xAOD::ElectronContainer>& ElectronLink: FE_ElectronLinks){
        if (!ElectronLink.isValid()){
            ATH_MSG_WARNING("JetPFlowSelectionAlg encountered an invalid electron element link. Skipping. ");
            continue; 
        }
    
        const xAOD::Electron* electron = *ElectronLink;
        bool passElectronID = false;
        bool gotID = electron->passSelection(passElectronID, qualityString);
        if (!gotID) {
            ATH_MSG_WARNING("Could not get Electron ID");
            continue;
        }
    
        if( electron->pt() > 10*GeV && passElectronID){
          return true;
        }
      }
    return false;
 }
