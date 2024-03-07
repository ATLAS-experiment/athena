// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#include "./eFexInputProvider.h"

#include <math.h>

#include "TrigT1CaloEvent/EmTauROI_ClassDEF.h"
#include "TrigT1CaloEvent/CPCMXTopoData.h"

#include "TrigT1Interfaces/CPRoIDecoder.h"

#include "L1TopoEvent/eEmTOB.h"
#include "L1TopoEvent/TopoInputEvent.h"

#include "GaudiKernel/PhysicalConstants.h"

using namespace std;
using namespace LVL1;


// eFex to L1Topo conversion factors
const double eFexInputProvider::m_EtDouble_conversion = 0.1;      // 100 MeV to GeV
const double eFexInputProvider::m_phiDouble_conversion = 0.05;    // 20 x phi to phi
const double eFexInputProvider::m_etaDouble_conversion = 0.025;   // 40 x eta to eta


eFexInputProvider::eFexInputProvider(const std::string& type, const std::string& name, 
                                       const IInterface* parent) :
   base_class(type, name, parent)
{
   declareInterface<LVL1::IInputTOBConverter>( this );
}

eFexInputProvider::~eFexInputProvider()
{}

StatusCode
eFexInputProvider::initialize() {

   CHECK(m_eEM_EDMKey.initialize(SG::AllowEmpty));
   CHECK(m_eTau_EDMKey.initialize(SG::AllowEmpty));

   if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());
   return StatusCode::SUCCESS;
}

StatusCode
eFexInputProvider::fillEM(TCS::TopoInputEvent& inputEvent) const {
  if (m_eEM_EDMKey.empty()) {
    ATH_MSG_DEBUG("eFex EM input disabled, skip filling");
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::eFexEMRoIContainer> eEM_EDM(m_eEM_EDMKey);
  ATH_CHECK(eEM_EDM.isValid());

  for(const auto it : * eEM_EDM){
    const xAOD::eFexEMRoI* eFexRoI = it;
    ATH_MSG_DEBUG( "EDM eFex Number: " 
		   << +eFexRoI->eFexNumber() // returns an 8 bit unsigned integer referring to the eFEX number 
		   << " et: " 
		   << eFexRoI->et() // returns the et value of the EM cluster in MeV
		   << " etTOB: " 
		   << eFexRoI->etTOB() // returns the et value of the EM cluster in units of 100 MeV
		   << " eta: "
		   << eFexRoI->eta() // returns a floating point global eta
		   << " phi: "
		   << eFexRoI->phi() // returns a floating point global phi
		   << " iEtaTopo: "
		   << eFexRoI->iEtaTopo() // returns 40 x eta (custom function for L1Topo)
		   << " iPhiTopo: "
		   << eFexRoI->iPhiTopo() // returns 20 x phi (custom function for L1Topo)
		   << " reta: "
		   << eFexRoI->RetaThresholds() // jet disc 1
		   << " rhad: "
		   << eFexRoI->RhadThresholds() // jet disc 2
		   << " wstot: "
		   << eFexRoI->WstotThresholds() // jet disc 3
		  );


    unsigned int EtTopo = eFexRoI->etTOB();
    int etaTopo = eFexRoI->iEtaTopo();
    int phiTopo = eFexRoI->iPhiTopo();
    unsigned int reta = eFexRoI->RetaThresholds();
    unsigned int rhad = eFexRoI->RhadThresholds();
    unsigned int wstot = eFexRoI->WstotThresholds();

    //Em TOB
    TCS::eEmTOB eem( EtTopo, etaTopo, static_cast<unsigned int>(phiTopo), TCS::EEM , static_cast<long int>(eFexRoI->word0()) );
    eem.setEtDouble( static_cast<double>(EtTopo*m_EtDouble_conversion) );
    eem.setEtaDouble( static_cast<double>(etaTopo*m_etaDouble_conversion) );
    eem.setPhiDouble( static_cast<double>(phiTopo*m_phiDouble_conversion) );
    eem.setReta( reta );
    eem.setRhad( rhad );
    eem.setWstot( wstot );
    
    inputEvent.addeEm( eem );

    auto mon_hEmEt = Monitored::Scalar("eEmTOBEt", eem.EtDouble());
    auto mon_hEmREta = Monitored::Scalar("eEmTOBREta", eem.Reta());
    auto mon_hEmRHad = Monitored::Scalar("eEmTOBRHad", eem.Rhad());
    auto mon_hEmWsTot = Monitored::Scalar("eEmTOBWsTot", eem.Wstot());
    auto mon_hEmPhi = Monitored::Scalar("eEmTOBPhi", eem.phi());
    auto mon_hEmEta = Monitored::Scalar("eEmTOBEta", eem.eta());
    Monitored::Group(m_monTool, mon_hEmEt, mon_hEmREta, mon_hEmRHad, mon_hEmWsTot, mon_hEmPhi, mon_hEmEta);   

  }

  return StatusCode::SUCCESS;
}


StatusCode
eFexInputProvider::fillTau(TCS::TopoInputEvent& inputEvent) const {
  if (m_eTau_EDMKey.empty()) {
    ATH_MSG_DEBUG("eFex Tau input disabled, skip filling");
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::eFexTauRoIContainer> eTau_EDM(m_eTau_EDMKey);
  ATH_CHECK(eTau_EDM.isValid());

  for(const auto it : * eTau_EDM){
    const xAOD::eFexTauRoI* eFexTauRoI = it;
    ATH_MSG_DEBUG( "EDM eFex Number: " 
		   << +eFexTauRoI->eFexNumber() // returns an 8 bit unsigned integer referring to the eFEX number 
		   << " et: " 
		   << eFexTauRoI->et() // returns the et value of the Tau cluster in MeV
		   << " etTOB: " 
		   << eFexTauRoI->etTOB() // returns the et value of the Tau cluster in units of 100 MeV
		   << " eta: "
		   << eFexTauRoI->eta() // returns a floating point global eta 
		   << " phi: "
		   << eFexTauRoI->phi() // returns a floating point global phi
		   << " iEtaTopo: "
		   << eFexTauRoI->iEtaTopo() // returns 40 x eta (custom function for L1Topo)
		   << " iPhiTopo: "
		   << eFexTauRoI->iPhiTopo() // returns 20 x phi (custom function for L1Topo)
		   << " rCore: "
		   << eFexTauRoI->tauOneThresholds() // rCore or BDT working point (eTau algo agnostic accessor)
		   << " rHad: "
		   << eFexTauRoI->tauTwoThresholds() // rHad
		  );


    unsigned int EtTopo = eFexTauRoI->etTOB();
    int etaTopo = eFexTauRoI->iEtaTopo();
    int phiTopo = eFexTauRoI->iPhiTopo();
    unsigned int rCore = eFexTauRoI->tauOneThresholds();
    unsigned int rHad = eFexTauRoI->tauTwoThresholds();

    //Tau TOB
    TCS::eTauTOB etau( EtTopo, etaTopo, static_cast<unsigned int>(phiTopo), TCS::ETAU );
    etau.setEtDouble(  static_cast<double>(EtTopo*m_EtDouble_conversion) );
    etau.setEtaDouble( static_cast<double>(etaTopo*m_etaDouble_conversion) );
    etau.setPhiDouble( static_cast<double>(phiTopo*m_phiDouble_conversion) );

    etau.setRCore( rCore );
    etau.setRHad( rHad );

    inputEvent.addeTau( etau );
    inputEvent.addcTau( etau );

    auto mon_hTauEt = Monitored::Scalar("eTauTOBEt", etau.EtDouble());
    auto mon_hTauRCore = Monitored::Scalar("eTauTOBRCore", etau.rCore());
    auto mon_hTauRHad = Monitored::Scalar("eTauTOBRHad", etau.rHad());
    auto mon_hTauPhi = Monitored::Scalar("eTauTOBPhi", etau.phi());
    auto mon_hTauEta = Monitored::Scalar("eTauTOBEta", etau.eta());
    Monitored::Group(m_monTool, mon_hTauEt, mon_hTauRCore, mon_hTauRHad, mon_hTauPhi, mon_hTauEta); 
  }

  return StatusCode::SUCCESS;
}

StatusCode
eFexInputProvider::fillTopoInputEvent(TCS::TopoInputEvent& inputEvent) const {
  ATH_CHECK(fillEM(inputEvent));
  ATH_CHECK(fillTau(inputEvent));
  return StatusCode::SUCCESS;
}


void 
eFexInputProvider::CalculateCoordinates(int32_t roiWord, double & eta, double & phi) const {
   CPRoIDecoder get;
   double TwoPI = 2 * M_PI;
   CoordinateRange coordRange = get.coordinate( roiWord );
   
   eta = coordRange.eta();
   phi = coordRange.phi();
   if( phi > M_PI ) phi -= TwoPI;
}
