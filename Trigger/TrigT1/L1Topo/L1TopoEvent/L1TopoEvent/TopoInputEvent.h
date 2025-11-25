// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#ifndef L1TopoEvent_TopoInputEvent
#define L1TopoEvent_TopoInputEvent

#include <iostream>
#include <vector>

#include "L1TopoEvent/ClusterTOBArray.h"
#include "L1TopoEvent/eEmTOBArray.h"
#include "L1TopoEvent/jEmTOBArray.h"
#include "L1TopoEvent/eTauTOBArray.h"
#include "L1TopoEvent/cTauTOBArray.h"
#include "L1TopoEvent/JetTOBArray.h"
#include "L1TopoEvent/jTauTOBArray.h"
#include "L1TopoEvent/JetTOBArray.h"
#include "L1TopoEvent/jLJetTOBArray.h"
#include "L1TopoEvent/gLJetTOBArray.h"
#include "L1TopoEvent/jJetTOBArray.h"
#include "L1TopoEvent/gJetTOBArray.h"
#include "L1TopoEvent/LateMuonTOBArray.h"
#include "L1TopoEvent/MuonNextBCTOBArray.h"
#include "L1TopoEvent/MuonTOBArray.h"
#include "L1TopoEvent/MetTOBArray.h"
#include "L1TopoEvent/jXETOBArray.h"
#include "L1TopoEvent/jTETOBArray.h"
#include "L1TopoEvent/gXETOBArray.h"
#include "L1TopoEvent/gTETOBArray.h"
#include "L1TopoCommon/StatusCode.h"
#include "L1TopoCommon/Types.h"

#include "TrigConfBase/TrigConfMessaging.h"

namespace TCS {
   class TopoInputEvent;
}

namespace TCS {

  std::ostream & operator<<(std::ostream &, const TCS::TopoInputEvent &);

   class TopoInputEvent : public TrigConf::TrigConfMessaging {
   public:
      
      TopoInputEvent();
      ~TopoInputEvent();
      
      const ClusterTOBArray & clusters() const { return m_clusters; }
      const eEmTOBArray & eEms() const { return m_eEms; }
      const jEmTOBArray & jEms() const { return m_jEms; }
      const eTauTOBArray & eTaus() const { return m_eTaus; }
      const jTauTOBArray & jTaus() const { return m_jTaus; }
      const cTauTOBArray & cTaus() const { return m_cTaus; }
      const ClusterTOBArray & taus() const { return m_taus; }
      const JetTOBArray & jets() const { return m_jets; }
      const jLJetTOBArray & jLJets() const { return m_jLJets; }
      const gLJetTOBArray & gLJets() const { return m_gLJets; }
      const jJetTOBArray & jJets() const { return m_jJets; }
      const gJetTOBArray & gJets() const { return m_gJets; }
      const MuonTOBArray & muons() const { return m_muons; }
      const LateMuonTOBArray & lateMuons() const { return m_lateMuons; }
      const MuonNextBCTOBArray & muonsNextBC() const { return m_muonsNextBC; }
      const MetTOB & met() const { return m_met[0]; }
      const jXETOB & jxe() const { return m_jxe[0]; }
      const jTETOB & jte() const { return m_jte[0]; }
      const jTETOB & jtec() const { return m_jtec[0]; }
      const jTETOB & jtefwd() const { return m_jtefwd[0]; }
      const jTETOB & jtefwda() const { return m_jtefwda[0]; }
      const jTETOB & jtefwdc() const { return m_jtefwdc[0]; }
      const gXETOB & gxe() const { return m_gxejwoj[0]; }
      const gXETOB & gmht() const { return m_gmht[0]; }
      const gXETOB & gxenc() const { return m_gxenc[0]; }
      const gXETOB & gxerho() const { return m_gxerho[0]; }
      const gTETOB & gte() const { return m_gte[0]; }
      const jXETOB & cxe() const { return m_cxe[0]; }
      const gTETOB & gespresso() const { return m_gespresso[0]; }
      uint32_t run_number()        const { return m_runNo; }
      uint32_t event_number()      const { return m_evtNo; }
      uint32_t lumi_block()        const { return m_lumiB; }
      uint32_t bunch_crossing_id() const { return m_BCID;  }
      /** @defgroup groupOverflowGetters
       *  @brief getter function for overflow bits
       *  @{
       */
      bool overflowFromEmtauInput () const { return m_overflowFromEmtauInput ; }
      bool overflowFromJetInput   () const { return m_overflowFromJetInput   ; }
      bool overflowFromeEmInput   () const { return m_overflowFromeEmInput   ; }
      bool overflowFromjEmInput   () const { return m_overflowFromjEmInput   ; }
      bool overflowFromeTauInput   () const { return m_overflowFromeTauInput   ; }
      bool overflowFromjTauInput   () const { return m_overflowFromjTauInput   ; }
      bool overflowFromcTauInput   () const { return m_overflowFromcTauInput   ; }
      bool overflowFromjLJetInput   () const { return m_overflowFromjLJetInput   ; }
      bool overflowFromgLJetInput   () const { return m_overflowFromgLJetInput   ; }
      bool overflowFromjJetInput   () const { return m_overflowFromjJetInput   ; }
      bool overflowFromgJetInput   () const { return m_overflowFromgJetInput   ; }
      bool overflowFromEnergyInput() const { return m_overflowFromEnergyInput; }
      bool overflowFromjXEInput   () const { return m_overflowFromjXEInput   ; }
      bool overflowFromjXECInput   () const { return m_overflowFromjXECInput   ; }
      bool overflowFromjXEPERFInput   () const { return m_overflowFromjXEPERFInput   ; }
      bool overflowFromjTEInput   () const { return m_overflowFromjTEInput   ; }
      bool overflowFromjTECInput   () const { return m_overflowFromjTECInput   ; }
      bool overflowFromjTEFWDInput   () const { return m_overflowFromjTEFWDInput   ; }
      bool overflowFromjTEFWDAInput   () const { return m_overflowFromjTEFWDAInput   ; }
      bool overflowFromjTEFWDCInput   () const { return m_overflowFromjTEFWDCInput   ; }
      bool overflowFromgXEJWOJInput   () const { return m_overflowFromgXEJWOJInput   ; }
      bool overflowFromgXENCInput   () const { return m_overflowFromgXENCInput   ; }
      bool overflowFromgXERHOInput   () const { return m_overflowFromgXERHOInput   ; }
      bool overflowFromgMHTInput   () const { return m_overflowFromgMHTInput   ; }
      bool overflowFromgTEInput   () const { return m_overflowFromgTEInput   ; }
      bool overflowFromcXEInput   () const { return m_overflowFromcXEInput   ; }
      bool overflowFromgESPRESSOInput   () const { return m_overflowFromgESPRESSOInput   ; }
      bool overflowFromMuonInput  () const { return m_overflowFromMuonInput  ; }
      bool overflowFromLateMuonInput  () const { return m_overflowFromLateMuonInput  ; }
      bool overflowFromMuonNextBCInput  () const { return m_overflowFromMuonNextBCInput  ; }
      /** @} */ // end of groupOverflowGetters
      StatusCode addCluster(const ClusterTOB & cluster);
      StatusCode addeEm(const eEmTOB & eEm);
      StatusCode addjEm(const jEmTOB & jEm);
      StatusCode addeTau(const eTauTOB & eTau);
      StatusCode addjTau(const jTauTOB & tau);
      StatusCode addcTau(const eTauTOB & eTau);
      StatusCode addcTau(const jTauTOB & jTau);
      StatusCode addTau(const ClusterTOB & tau);
      StatusCode addMuon(const MuonTOB & muon);
      StatusCode addLateMuon(const LateMuonTOB & muon);
      StatusCode addMuonNextBC(const MuonNextBCTOB & muon);
      StatusCode addJet(const JetTOB & jet);
      StatusCode addjLJet(const jLJetTOB & jet);
      StatusCode addgLJet(const gLJetTOB & jet);
      StatusCode addjJet(const jJetTOB & jet);
      StatusCode addgJet(const gJetTOB & jet);
      StatusCode setMET(const MetTOB & met);
      StatusCode setjXE(const jXETOB & jXE);
      StatusCode setjXEC(const jXETOB & jXEC);
      StatusCode setjXEPERF(const jXETOB & jXEPERF);
      StatusCode setjTE(const jTETOB & jTE);
      StatusCode setjTEC(const jTETOB & jTEC);
      StatusCode setjTEFWD(const jTETOB & jTEFWD);
      StatusCode setjTEFWDA(const jTETOB & jTEFWDA);
      StatusCode setjTEFWDC(const jTETOB & jTEFWDC);
      StatusCode setgXEJWOJ(const gXETOB & gXEJWOJ);
      StatusCode setgXENC(const gXETOB & gXENC);
      StatusCode setgXERHO(const gXETOB & gXERHO);
      StatusCode setgMHT(const gXETOB & gMHT);
      StatusCode setgTE(const gTETOB & gTE);
      StatusCode setcXE(const jXETOB & cXE);
      StatusCode setgESPRESSO(const gTETOB & gESPRESSO);
      StatusCode setEventInfo(const uint32_t runNo, const uint32_t evtNo, const uint32_t lumiB, const uint32_t BCID);
      /** @defgroup groupOverflowSetters
       *  @brief setter function for overflow bits
       *  The bits are being propagated from the input providers.
       *  @{
       */
      void setOverflowFromEmtauInput (bool v);
      void setOverflowFromJetInput   (bool v);
      void setOverflowFromeEmInput   (bool v);
      void setOverflowFromjEmInput   (bool v);
      void setOverflowFromeTauInput   (bool v);
      void setOverflowFromjTauInput   (bool v);
      void setOverflowFromcTauInput   (bool v);
      void setOverflowFromjLJetInput   (bool v);
      void setOverflowFromgLJetInput   (bool v);
      void setOverflowFromjJetInput   (bool v);
      void setOverflowFromgJetInput   (bool v);
      void setOverflowFromEnergyInput(bool v);
      void setOverflowFromjXEInput(bool v);
      void setOverflowFromjXECInput(bool v);
      void setOverflowFromjXEPERFInput(bool v);
      void setOverflowFromjTEInput(bool v);
      void setOverflowFromjTECInput(bool v);
      void setOverflowFromjTEFWDInput(bool v);
      void setOverflowFromjTEFWDAInput(bool v);
      void setOverflowFromjTEFWDCInput(bool v);
      void setOverflowFromgXEJWOJInput(bool v);
      void setOverflowFromgXENCInput(bool v);
      void setOverflowFromgXERHOInput(bool v);
      void setOverflowFromgMHTInput(bool v);
      void setOverflowFromgTEInput(bool v);
      void setOverflowFromcXEInput(bool v);
      void setOverflowFromgESPRESSOInput(bool v);
      void setOverflowFromMuonInput  (bool v);
      void setOverflowFromLateMuonInput  (bool v);
      void setOverflowFromMuonNextBCInput  (bool v);
      /** @} */ // end of groupOverflowSetters
      // access all inputs by type
      const InputTOBArray * inputTOBs(TCS::inputTOBType_t) const;
      /// @brief whether there are input overflows from Mioct for muon and from CMX for calo
      bool hasInputOverflow(TCS::inputTOBType_t) const;

      // to be calles after each event
      StatusCode clear();

      // to enable dumping of the input after each event
      void enableInputDump( const std::string& filename, bool enable = true );

      // to trigger the writing of the event
      void dumpStart();
      void dump();
      void dumpFinish();

      void print() const;

   private:
      friend std::ostream & operator<<(std::ostream &, const TCS::TopoInputEvent &);

      // data is owned by the TopoInputEvent
      ClusterTOBArray   m_clusters;
      eEmTOBArray       m_eEms;
      jEmTOBArray       m_jEms;
      eTauTOBArray      m_eTaus;
      jTauTOBArray      m_jTaus;
      cTauTOBArray      m_cTaus;
      ClusterTOBArray   m_taus;
      JetTOBArray       m_jets;
      jLJetTOBArray      m_jLJets;
      gLJetTOBArray      m_gLJets;
      jJetTOBArray      m_jJets;
      gJetTOBArray      m_gJets;
      MuonTOBArray      m_muons;
      LateMuonTOBArray  m_lateMuons;
      MuonNextBCTOBArray  m_muonsNextBC;
      MetTOBArray       m_met;      // will have size 1
      jXETOBArray       m_jxe;      // will have size 1
      jXETOBArray       m_jxec;     // will have size 1
      jXETOBArray       m_jxeperf;  // will have size 1
      jTETOBArray       m_jte;      // will have size 1
      jTETOBArray       m_jtec;     // will have size 1
      jTETOBArray       m_jtefwd;   // will have size 1
      jTETOBArray       m_jtefwda;  // will have size 1
      jTETOBArray       m_jtefwdc;  // will have size 1
      gXETOBArray       m_gxejwoj;  // will have size 1
      gXETOBArray       m_gxenc;    // will have size 1
      gXETOBArray       m_gxerho;   // will have size 1
      gXETOBArray       m_gmht;     // will have size 1
      gTETOBArray       m_gte;      // will have size 1
      jXETOBArray       m_cxe;      // will have size 1
      gTETOBArray       m_gespresso;// will have size 1

      uint32_t m_runNo {0};
      uint32_t m_evtNo {0};
      uint32_t m_lumiB {0};
      uint32_t m_BCID {0};

      bool m_dumpEnabled { false };
      bool m_overflowFromEmtauInput { false };
      bool m_overflowFromJetInput { false };
      bool m_overflowFromeEmInput { false };
      bool m_overflowFromjEmInput { false };
      bool m_overflowFromeTauInput { false };
      bool m_overflowFromjTauInput { false };
      bool m_overflowFromcTauInput { false };
      bool m_overflowFromjLJetInput { false };
      bool m_overflowFromgLJetInput { false };
      bool m_overflowFromjJetInput { false };
      bool m_overflowFromgJetInput { false };
      bool m_overflowFromEnergyInput { false };
      bool m_overflowFromjXEInput { false };
      bool m_overflowFromjXECInput { false };
      bool m_overflowFromjXEPERFInput { false };
      bool m_overflowFromjTEInput { false };
      bool m_overflowFromjTECInput { false };
      bool m_overflowFromjTEFWDInput { false };
      bool m_overflowFromjTEFWDAInput { false };
      bool m_overflowFromjTEFWDCInput { false };
      bool m_overflowFromgXEJWOJInput { false };
      bool m_overflowFromgXENCInput { false };
      bool m_overflowFromgXERHOInput { false };
      bool m_overflowFromgMHTInput { false };
      bool m_overflowFromgTEInput { false };
      bool m_overflowFromcXEInput { false };
      bool m_overflowFromgESPRESSOInput { false };
      bool m_overflowFromMuonInput { false };
      bool m_overflowFromLateMuonInput { false };
      bool m_overflowFromMuonNextBCInput { false };
      std::string m_inputDumpFile { "" };

   };
   
}


#endif
