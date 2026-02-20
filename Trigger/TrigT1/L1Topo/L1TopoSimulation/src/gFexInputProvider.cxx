/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include <math.h> /* atan2 */

#include "gFexInputProvider.h"
#include "TrigT1CaloEvent/EnergyRoI_ClassDEF.h"
#include "TrigT1Interfaces/TrigT1CaloDefs.h"

#include "L1TopoEvent/TopoInputEvent.h"

#include <unistd.h>

using namespace std;
using namespace LVL1;


// gFex to L1Topo conversion factors
const int gFexInputProvider::m_EtJet_conversion = 2;            // 200 MeV to 100 MeV for gJet, gLJet
const double gFexInputProvider::m_EtGlobal_conversion = 0.01;   // 1 MeV to 100 MeV for gXE, gTE
const int gFexInputProvider::m_phi_conversion = 1;              // 20 x phi to 20 x phi
const int gFexInputProvider::m_eta_conversion = 40;             // eta to 40 x eta

const double gFexInputProvider::m_EtDoubleJet_conversion = 0.1;       // 100 MeV to GeV for gJet, gLJet
const double gFexInputProvider::m_EtDoubleGlobal_conversion = 0.1;    // 100 MeV to GeV for gXE, gTE
const double gFexInputProvider::m_phiDouble_conversion = 0.05;        // 20 x phi to phi
const double gFexInputProvider::m_etaDouble_conversion = 0.025;       // 40 x eta to eta


gFexInputProvider::gFexInputProvider(const std::string& type, const std::string& name, 
                                         const IInterface* parent) :
   base_class(type, name, parent)
{
   declareInterface<LVL1::IInputTOBConverter>( this );
}


StatusCode
gFexInputProvider::initialize() {
  
  ATH_CHECK(m_gJet_EDMKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_gLJet_EDMKey.initialize(SG::AllowEmpty));

  ATH_CHECK(m_gXEJWOJ_EDMKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_gMHT_EDMKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_gXENC_EDMKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_gXERHO_EDMKey.initialize(SG::AllowEmpty));

  ATH_CHECK(m_gTE_EDMKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_gESPRESSO_EDMKey.initialize(SG::AllowEmpty));
  if (! m_gESPRESSO_EDMKey.empty() ) {
    renounce(m_gESPRESSO_EDMKey); //make this optional, as its availability depends on how gFEX TOBs are being produced (cannot be simulated by gFEX sim, optional form gFEX bytestream decoders
  }
  
  if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode
gFexInputProvider::fillSRJet(TCS::TopoInputEvent& inputEvent) const {

  if (m_gJet_EDMKey.empty()) {
    ATH_MSG_DEBUG("gFex Jet input disabled, skip filling");
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::gFexJetRoIContainer> gJet_EDM(m_gJet_EDMKey);
  ATH_CHECK(gJet_EDM.isValid());

  for(const xAOD::gFexJetRoI* gFexRoI : * gJet_EDM) {

    auto jetType = gFexRoI->gFexType();
    if ( jetType != 1 and jetType != 2 ) { continue; } // 1 = gFEX gBlockLead, 2 = gFEX gBlockSub (L1Topo gJet)

    ATH_MSG_DEBUG( "EDM gFex Jet type: "
                   << gFexRoI->gFexType() 
                   << " Et: " 
		   << gFexRoI->et() // returns the et value of the jet in units of MeV
                   << " gFexTobEt: " 
		   << gFexRoI->gFexTobEt() // returns the et value of the jet in units of 200 MeV
		   << " eta: "
		   << gFexRoI->eta() // returns a floating point eta corresponding to the center of the tower (fixed values, defined in gFexJetRoI_v1.cxx)
		   << " phi: "
		   << gFexRoI->phi() // returns a floating point phi corresponding to the center of the tower
		   << " iPhiTopo: "
		   << gFexRoI->iPhiTopo() // returns an integer phi from 0 to 127
		   );
    
   unsigned int EtTopo = gFexRoI->gFexTobEt()*m_EtJet_conversion;
   unsigned int phiTopo = gFexRoI->iPhiTopo()*m_phi_conversion;
   int etaTopo = static_cast<int>(gFexRoI->eta()*m_eta_conversion);

   TCS::gJetTOB gJet( EtTopo, etaTopo, phiTopo );
 
   gJet.setEtDouble( static_cast<double>(EtTopo*m_EtDoubleJet_conversion) );
   gJet.setEtaDouble( static_cast<double>(etaTopo*m_etaDouble_conversion) );
   gJet.setPhiDouble( static_cast<double>(phiTopo*m_phiDouble_conversion) );
 
   inputEvent.addgJet( gJet );
   auto mon_h_gJetPt = Monitored::Scalar("gJetTOBPt", gJet.EtDouble());
   auto mon_h_gJetPhi = Monitored::Scalar("gJetTOBPhi", gJet.phi());
   auto mon_h_gJetEta = Monitored::Scalar("gJetTOBEta", gJet.eta());
   Monitored::Group(m_monTool, mon_h_gJetPt, mon_h_gJetPhi, mon_h_gJetEta); 

   }

   return StatusCode::SUCCESS;

}


StatusCode
gFexInputProvider::fillLRJet(TCS::TopoInputEvent& inputEvent) const {

  if (m_gLJet_EDMKey.empty()) {
    ATH_MSG_DEBUG("gFex LJet input disabled, skip filling");
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::gFexJetRoIContainer> gLJet_EDM(m_gLJet_EDMKey);
  ATH_CHECK(gLJet_EDM.isValid());

  for(const xAOD::gFexJetRoI* gFexRoI : * gLJet_EDM) {

    auto jetType = gFexRoI->gFexType();
    if ( jetType != 3 ) { continue; } // 3 = gFEX gJet (L1Topo gLJet)

    ATH_MSG_DEBUG( "EDM gFex LJet type: "
                   << gFexRoI->gFexType() 
                   << " Et: " 
		   << gFexRoI->et() // returns the et value of the jet in units of MeV
                   << " gFexTobEt: " 
		   << gFexRoI->gFexTobEt() // returns the et value of the jet in units of 200 MeV
		   << " eta: "
		   << gFexRoI->eta() // returns a floating point eta corresponding to the center of the tower (fixed values, defined in gFexJetRoI_v1.cxx)
		   << " phi: "
		   << gFexRoI->phi() // returns a floating point phi corresponding to the center of the tower
		   << " iPhiTopo: "
		   << gFexRoI->iPhiTopo() // returns an integer phi from 0 to 127
		   );
    
   unsigned int EtTopo = gFexRoI->gFexTobEt()*m_EtJet_conversion;
   unsigned int phiTopo = gFexRoI->iPhiTopo()*m_phi_conversion;
   int etaTopo = static_cast<int>(gFexRoI->eta()*m_eta_conversion);

   TCS::gLJetTOB gJet( EtTopo, etaTopo, phiTopo );
 
   gJet.setEtDouble( static_cast<double>(EtTopo*m_EtDoubleJet_conversion) );
   gJet.setEtaDouble( static_cast<double>(etaTopo*m_etaDouble_conversion) );
   gJet.setPhiDouble( static_cast<double>(phiTopo*m_phiDouble_conversion) );
 
   inputEvent.addgLJet( gJet );
   auto mon_h_gLJetPt = Monitored::Scalar("gLJetTOBPt", gJet.EtDouble());
   auto mon_h_gLJetPhi = Monitored::Scalar("gLJetTOBPhi", gJet.phi());
   auto mon_h_gLJetEta = Monitored::Scalar("gLJetTOBEta", gJet.eta());
   Monitored::Group(m_monTool, mon_h_gLJetPt, mon_h_gLJetPhi, mon_h_gLJetEta);

   }

   return StatusCode::SUCCESS;

}


StatusCode
gFexInputProvider::fillXEJWOJ(TCS::TopoInputEvent& inputEvent) const {

  if (m_gXEJWOJ_EDMKey.empty()) {
    ATH_MSG_DEBUG("gFex XEJWOJ input disabled, skip filling");
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::gFexGlobalRoIContainer> gXEJWOJ_EDM(m_gXEJWOJ_EDMKey);
  ATH_CHECK(gXEJWOJ_EDM.isValid());

  for(const xAOD::gFexGlobalRoI* gFexRoI : * gXEJWOJ_EDM) {

    auto globalType = gFexRoI->globalType();
    if ( globalType != 2 ) { continue; } // 2 = MET components (METx, METy)

    ATH_MSG_DEBUG( "EDM gFex XEJWOJ type: "
                   << gFexRoI->globalType()
                   << " Ex: " 
                   << gFexRoI->METquantityOne() // returns the Ex component in MeV
                   << " Ey: " 
		   << gFexRoI->METquantityTwo() // returns the Ey component in MeV
		   );
    
   int ExTopo = gFexRoI->METquantityOne()*m_EtGlobal_conversion;
   int EyTopo = gFexRoI->METquantityTwo()*m_EtGlobal_conversion;

   unsigned long long ExTopoLong = static_cast<unsigned long long>(ExTopo);
   unsigned long long EyTopoLong = static_cast<unsigned long long>(EyTopo);

   unsigned long long Et2Topo = ExTopoLong*ExTopoLong + EyTopoLong*EyTopoLong;
   unsigned int EtTopo  = std::sqrt(Et2Topo);
 
   TCS::gXETOB gxe( -ExTopo, -EyTopo, EtTopo, TCS::GXEJWOJ );
 
   gxe.setExDouble( static_cast<double>(ExTopo*m_EtDoubleGlobal_conversion) );
   gxe.setEyDouble( static_cast<double>(EyTopo*m_EtDoubleGlobal_conversion) );
   gxe.setEtDouble( static_cast<double>(EtTopo*m_EtDoubleGlobal_conversion) );
   gxe.setEt2( Et2Topo );
 
   inputEvent.setgXEJWOJ( gxe );
   auto mon_h_gXEJWOJPt = Monitored::Scalar("gXEJWOJTOBPt", gxe.EtDouble());
   auto mon_h_gXEJWOJPhi = Monitored::Scalar("gXEJWOJTOBPhi", atan2(gxe.Ey(), gxe.Ex()));
   Monitored::Group(m_monTool, mon_h_gXEJWOJPt, mon_h_gXEJWOJPhi);

   }

   return StatusCode::SUCCESS;

}


StatusCode
gFexInputProvider::fillMHT(TCS::TopoInputEvent& inputEvent) const {

  if (m_gMHT_EDMKey.empty()) {
    ATH_MSG_DEBUG("gFex MHT input disabled, skip filling");
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::gFexGlobalRoIContainer> gMHT_EDM(m_gMHT_EDMKey);
  ATH_CHECK(gMHT_EDM.isValid());

  for(const xAOD::gFexGlobalRoI* gFexRoI : * gMHT_EDM) {

    auto globalType = gFexRoI->globalType();
    if ( globalType != 3 ) { continue; } // 3 = MET hard term components (MHTx, MHTy)

    ATH_MSG_DEBUG( "EDM gFex MHT type: "
                   << gFexRoI->globalType()
                   << " Ex: " 
                   << gFexRoI->METquantityOne() // returns the Ex component in MeV
                   << " Ey: " 
		   << gFexRoI->METquantityTwo() // returns the Ey component in MeV
		   );
    
   int ExTopo = gFexRoI->METquantityOne()*m_EtGlobal_conversion;
   int EyTopo = gFexRoI->METquantityTwo()*m_EtGlobal_conversion;

   unsigned long long ExTopoLong = static_cast<unsigned long long>(ExTopo);
   unsigned long long EyTopoLong = static_cast<unsigned long long>(EyTopo);

   unsigned long long Et2Topo = ExTopoLong*ExTopoLong + EyTopoLong*EyTopoLong;
   unsigned int EtTopo  = std::sqrt(Et2Topo);
 
   TCS::gXETOB gxe( -ExTopo, -EyTopo, EtTopo, TCS::GMHT );
 
   gxe.setExDouble( static_cast<double>(ExTopo*m_EtDoubleGlobal_conversion) );
   gxe.setEyDouble( static_cast<double>(EyTopo*m_EtDoubleGlobal_conversion) );
   gxe.setEtDouble( static_cast<double>(EtTopo*m_EtDoubleGlobal_conversion) );
   gxe.setEt2( Et2Topo );
 
   inputEvent.setgMHT( gxe );
   auto mon_h_gMHTPt = Monitored::Scalar("gMHTTOBPt", gxe.EtDouble());
   auto mon_h_gMHTPhi = Monitored::Scalar("gMHTTOBPhi", atan2(gxe.Ey(), gxe.Ex()));
   Monitored::Group(m_monTool, mon_h_gMHTPt, mon_h_gMHTPhi);

   }

   return StatusCode::SUCCESS;

}


StatusCode
gFexInputProvider::fillXENC(TCS::TopoInputEvent& inputEvent) const {

  if (m_gXENC_EDMKey.empty()) {
    ATH_MSG_DEBUG("gFex XENC input disabled, skip filling");
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::gFexGlobalRoIContainer> gXENC_EDM(m_gXENC_EDMKey);
  ATH_CHECK(gXENC_EDM.isValid());

  for(const xAOD::gFexGlobalRoI* gFexRoI : * gXENC_EDM) {

    ATH_MSG_DEBUG( "EDM gFex XENC type: "
                   << gFexRoI->globalType()
                   << " Ex: " 
                   << gFexRoI->METquantityOne() // returns the Ex component in MeV
                   << " Ey: " 
		   << gFexRoI->METquantityTwo() // returns the Ey component in MeV
		   );
    
   int ExTopo = gFexRoI->METquantityOne()*m_EtGlobal_conversion;
   int EyTopo = gFexRoI->METquantityTwo()*m_EtGlobal_conversion;
   
   //Cast the Ex and Ey to longs to perform multiplication that is safe wrt. overflows   
   unsigned long long ExTopoLong = static_cast<unsigned long long>(ExTopo);
   unsigned long long EyTopoLong = static_cast<unsigned long long>(EyTopo);

   unsigned long long Et2Topo = ExTopoLong*ExTopoLong + EyTopoLong*EyTopoLong;
   unsigned int EtTopo  = std::sqrt(Et2Topo);
 
   TCS::gXETOB gxe( -ExTopo, -EyTopo, EtTopo, TCS::GXENC );
 
   gxe.setExDouble( static_cast<double>(ExTopo*m_EtDoubleGlobal_conversion) );
   gxe.setEyDouble( static_cast<double>(EyTopo*m_EtDoubleGlobal_conversion) );
   gxe.setEtDouble( static_cast<double>(EtTopo*m_EtDoubleGlobal_conversion) );
   gxe.setEt2( Et2Topo );
 
   inputEvent.setgXENC( gxe );
   auto mon_h_gXENCPt = Monitored::Scalar("gXENCTOBPt", gxe.EtDouble());
   Monitored::Group(m_monTool, mon_h_gXENCPt);   

   }

   return StatusCode::SUCCESS;

}


StatusCode
gFexInputProvider::fillXERHO(TCS::TopoInputEvent& inputEvent) const {

  if (m_gXERHO_EDMKey.empty()) {
    ATH_MSG_DEBUG("gFex XENC input disabled, skip filling");
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::gFexGlobalRoIContainer> gXERHO_EDM(m_gXERHO_EDMKey);
  ATH_CHECK(gXERHO_EDM.isValid());

  for(const xAOD::gFexGlobalRoI* gFexRoI : * gXERHO_EDM) {

    ATH_MSG_DEBUG( "EDM gFex XERHO type: "
                   << gFexRoI->globalType()
                   << " Ex: " 
                   << gFexRoI->METquantityOne() // returns the Ex component in MeV
                   << " Ey: " 
		   << gFexRoI->METquantityTwo() // returns the Ey component in MeV
		   );
    
   int ExTopo = gFexRoI->METquantityOne()*m_EtGlobal_conversion;
   int EyTopo = gFexRoI->METquantityTwo()*m_EtGlobal_conversion;

   unsigned long long ExTopoLong = static_cast<unsigned long long>(ExTopo);
   unsigned long long EyTopoLong = static_cast<unsigned long long>(EyTopo);

   unsigned long long Et2Topo = ExTopoLong*ExTopoLong + EyTopoLong*EyTopoLong;
   unsigned int EtTopo  = std::sqrt(Et2Topo);
 
   TCS::gXETOB gxe( -ExTopo, -EyTopo, EtTopo, TCS::GXERHO );
 
   gxe.setExDouble( static_cast<double>(ExTopo*m_EtDoubleGlobal_conversion) );
   gxe.setEyDouble( static_cast<double>(EyTopo*m_EtDoubleGlobal_conversion) );
   gxe.setEtDouble( static_cast<double>(EtTopo*m_EtDoubleGlobal_conversion) );
   gxe.setEt2( Et2Topo );
 
   inputEvent.setgXERHO( gxe );
   auto mon_h_gXERHOPt = Monitored::Scalar("gXERHOTOBPt", gxe.EtDouble());
   Monitored::Group(m_monTool, mon_h_gXERHOPt);      

   }

   return StatusCode::SUCCESS;

}


StatusCode
gFexInputProvider::fillTE(TCS::TopoInputEvent& inputEvent) const {

  if (m_gTE_EDMKey.empty()) {
    ATH_MSG_DEBUG("gFex TE input disabled, skip filling");
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::gFexGlobalRoIContainer> gTE_EDM(m_gTE_EDMKey);
  ATH_CHECK(gTE_EDM.isValid());

  for(const xAOD::gFexGlobalRoI* gFexRoI : * gTE_EDM) {

    auto globalType = gFexRoI->globalType();
    if ( globalType != 1 ) { continue; } // 1 = scalar values (MET, SumET)

    ATH_MSG_DEBUG( "EDM gFex TE type: "
                   << gFexRoI->globalType()
                   << " sumEt: " 
		   << gFexRoI->METquantityTwo() // returns sumEt in MeV
		   );
    
   unsigned int sumEtTopo = gFexRoI->METquantityTwo()*m_EtGlobal_conversion;

   TCS::gTETOB gte( sumEtTopo, TCS::GTE );
 
   gte.setSumEtDouble( static_cast<double>(sumEtTopo*m_EtDoubleGlobal_conversion) );
 
   inputEvent.setgTE( gte );
   auto mon_h_gTEsumEt = Monitored::Scalar("gTEsumEt", gte.sumEtDouble());
   Monitored::Group(m_monTool, mon_h_gTEsumEt); 

   }

   return StatusCode::SUCCESS;

}

StatusCode
gFexInputProvider::fillGESPRESSO(TCS::TopoInputEvent& inputEvent) const {
  if (m_gESPRESSO_EDMKey.empty()) {
    ATH_MSG_DEBUG("gFex ESPRESSO input disabled, skip filling");
    return StatusCode::SUCCESS;
  }
  
  SG::ReadHandle<xAOD::gFexGlobalRoIContainer> gESPRESSO_EDM(m_gESPRESSO_EDMKey);
  if (! gESPRESSO_EDM.isValid() ) {
    //gESPRESSO is only active in HI runs and only available from data. If not present simply skip it.
    ATH_MSG_DEBUG("gFex ESPRESSO input is not available, skip filling");
    return StatusCode::SUCCESS;
  }
  
  for(const xAOD::gFexGlobalRoI* gFexRoI : * gESPRESSO_EDM) {

    auto globalType = gFexRoI->globalType();
    if ( globalType != 1 ) { continue; } // 1 = scalar values (MET, SumET)

    ATH_MSG_DEBUG( "EDM gFex ESPRESSO type: "
                   << gFexRoI->globalType()
                   << " sumEt: " 
		   << gFexRoI->METquantityTwo() // returns sumEt in MeV
		   );
    
   unsigned int sumEtTopo = gFexRoI->METquantityTwo()*m_EtGlobal_conversion;

   TCS::gTETOB gespresso( sumEtTopo, TCS::GESPRESSO );
 
   gespresso.setSumEtDouble( static_cast<double>(sumEtTopo*m_EtDoubleGlobal_conversion) );
 
   inputEvent.setgESPRESSO( gespresso );
   auto mon_h_gTEsumEt = Monitored::Scalar("gESPRESSOsumEt", gespresso.sumEtDouble());
   Monitored::Group(m_monTool, mon_h_gTEsumEt); 

   }

   return StatusCode::SUCCESS;
}

StatusCode
gFexInputProvider::fillTopoInputEvent(TCS::TopoInputEvent& inputEvent) const {
  ATH_CHECK(fillSRJet(inputEvent));
  ATH_CHECK(fillLRJet(inputEvent));

  ATH_CHECK(fillXEJWOJ(inputEvent));
  ATH_CHECK(fillMHT(inputEvent));
  ATH_CHECK(fillXENC(inputEvent));
  ATH_CHECK(fillXERHO(inputEvent));

  ATH_CHECK(fillTE(inputEvent));
  ATH_CHECK(fillGESPRESSO(inputEvent));
  return StatusCode::SUCCESS;
}

