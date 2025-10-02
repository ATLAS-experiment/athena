/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigJiveXML/xAODEmTauROIRetriever.h"

#include <string>

#include "CLHEP/Units/SystemOfUnits.h"

#include "xAODTrigger/EmTauRoIContainer.h"

namespace JiveXML {

  //--------------------------------------------------------------------------

  xAODEmTauROIRetriever::xAODEmTauROIRetriever(const std::string& type, const std::string& name, const IInterface* parent):
    AthAlgTool(type, name, parent), m_typeName("EmTauROI")
  {

    declareInterface<IDataRetriever>(this);
  }

  //--------------------------------------------------------------------------

  StatusCode xAODEmTauROIRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

  DataVect phi;
  DataVect eta; 
  DataVect energy; 
  DataVect energyEM;
  DataVect energyTAU;
  DataVect roiWord; 
  DataVect thrPattern; 

  //// which of those two option is working:

  const xAOD::EmTauRoIContainer* emTauRoIs = 0; 


    // L1JetObject -not- available
    m_sgKey = "LVL1EmTauRoIs"; 
    if ( evtStore()->retrieve(emTauRoIs,m_sgKey).isFailure() ) {
      if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) <<  "No LVL1EmTauROIs found in SG " << endmsg;
      return StatusCode::SUCCESS;
    } 
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) <<  "Found LVL1EmTauROIs in SG ! " << endmsg;

    xAOD::EmTauRoIContainer::const_iterator itEM  = emTauRoIs->begin();
    xAOD::EmTauRoIContainer::const_iterator itEMe = emTauRoIs->end();


    int counter = 0;
    for (; itEM != itEMe; ++itEM)
        {
        phi.push_back(DataType( (*itEM)->phi()) );
	eta.push_back(DataType( (*itEM)->eta()) );

       if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "xAOD EmTauRoI #" << counter++ 
          << ", eta: " << (*itEM)->eta() << ", phi: " << (*itEM)->phi() << endmsg;

// Placeholders ! No direct access to those in Run-2
// Reference:
//    Event/xAOD/xAODTrigger/trunk/Root/EmTauRoI_v2.cxx
// jpt 9Jan15:
        energy.push_back(DataType( 0. ));
        energyEM.push_back(DataType( 0. ));
        energyTAU.push_back(DataType( 0. ));
        roiWord.push_back(DataType( 0. ));
	thrPattern.push_back(DataType( 0. ));
      }

    DataMap myDataMap;
    const auto nPhi = phi.size();
    myDataMap["energy"] = energy;
    myDataMap["phi"] = std::move(phi);
    myDataMap["eta"] = std::move(eta);
    myDataMap["energy"] = std::move(energy);
    myDataMap["energyEM"] = std::move(energyEM);
    myDataMap["energyTAU"] = std::move(energyTAU);
    myDataMap["roiWord"] = std::move(roiWord);
    myDataMap["thrPattern"] = std::move(thrPattern);

    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << dataTypeName() << ": "<< nPhi
					    << " from: " << m_sgKey << endmsg;

    //forward data to formating tool
    return FormatTool->AddToEvent(dataTypeName(), m_sgKey, &myDataMap);
  }
}
