/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
 
 * @author Mirko Casolino, version w/o barcodes by Andrii Verbytskyi 2024
 * @date June 2015
 * @brief tool to compute oring of hadron to flag ttbar+HF
 * 
 */

#ifndef  DerivationFrameworkMCTruth_HadronOriginClassifier_H
#define  DerivationFrameworkMCTruth_HadronOriginClassifier_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODEventInfo/EventInfo.h"

#include <map>
#include <set>
#include <string>

namespace DerivationFramework{


  class HadronOriginClassifier: public AthAlgTool {


  public:
    HadronOriginClassifier(const std::string& t, const std::string& n, const IInterface* p);
    virtual ~HadronOriginClassifier();
    
    virtual StatusCode initialize() override;

    typedef enum {extrajet=0,
		  c_MPI     =-1, b_MPI      =1,
		  c_FSR     =-2, b_FSR      =2,
		  c_from_W  =-3, b_from_W   =3,
		  c_from_top=-4, b_from_top =4,
		  c_from_H  =-5, b_from_H   =5} HF_id;
    
    enum class GEN_id { Pythia6=0, Pythia8=1, HerwigPP=2, Sherpa=3 };
        
    std::map<const xAOD::TruthParticle*, HF_id> GetOriginMap() const;
    
  private:

    void fillHadronMap(std::set<const xAOD::TruthParticle*>& usedHadron, std::map<const xAOD::TruthParticle*,int>& mainHadronMap, const xAOD::TruthParticle* mainhad, const xAOD::TruthParticle* ihad, bool decayed=false) const;

    void buildPartonsHadronsMaps(std::map<const xAOD::TruthParticle*,int>& mainHadronMap,
                                 std::map<const xAOD::TruthParticle*,HF_id>& partonsOrigin) const;

    bool isCHadronFromB(const xAOD::TruthParticle* part, std::shared_ptr<std::set<const xAOD::TruthParticle*>> checked = nullptr) const;
    
    const xAOD::TruthParticle* findInitial(const xAOD::TruthParticle* part, std::shared_ptr<std::set<const xAOD::TruthParticle*>> checked = nullptr) const;
    
    bool isFromTop(const xAOD::TruthParticle* part) const;
    static bool isDirectlyFromTop(const xAOD::TruthParticle* part) ;
    bool isDirectlyFromWTop(const xAOD::TruthParticle* part) const;

    static bool isDirectlyFromGluonQuark(const xAOD::TruthParticle* part) ;
    bool isFromGluonQuark(const xAOD::TruthParticle* part) const;
    bool isDirectlyFSRPythia6(const xAOD::TruthParticle* part) const;

    bool isDirectlyFromQuarkTop(const xAOD::TruthParticle* part) const;
    bool isFromQuarkTop(const xAOD::TruthParticle* part) const;
    bool isDirectlyFSR(const xAOD::TruthParticle* part) const;
    bool isFromWTop(const xAOD::TruthParticle* part) const;

    static bool isDirectlyMPIPythia6(const xAOD::TruthParticle* part) ;

    bool isDirectlyMPIPythia8(const xAOD::TruthParticle* part) const;
    bool isDirectlyFromQuarkTopPythia8(const xAOD::TruthParticle* part) const;
    bool isFromQuarkTopPythia8(const xAOD::TruthParticle* part) const;
    bool isDirectlyFSRPythia8(const xAOD::TruthParticle* part) const;

    static bool isDirectlyMPISherpa(const xAOD::TruthParticle* part) ;

     
    inline bool IsHerwigPP() const {return m_GenUsed==GEN_id::HerwigPP;};
    inline bool IsPythia8() const {return m_GenUsed==GEN_id::Pythia8;};
    inline bool IsPythia6() const {return m_GenUsed==GEN_id::Pythia6;};
    inline bool IsSherpa() const {return m_GenUsed==GEN_id::Sherpa;};

    SG::ReadHandleKey<xAOD::TruthEventContainer> m_mcName {this, "MCCollectionName", "TruthEvents", "TruthEventContainer key"};
    Gaudi::Property<double> m_HadronPtMinCut{this, "HadronpTMinCut", 5000.}; /// MeV
    Gaudi::Property<double> m_HadronEtaMaxCut{this, "HadronetaMaxCut", 2.5};
    Gaudi::Property<int> m_DSID{this, "DSID", 410000};
    GEN_id m_GenUsed{};
    
  };

} //namespace


#endif //DerivationFrameworkMCTruth_HadronOriginClassifier_H

