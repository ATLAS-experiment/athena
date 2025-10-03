/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//		jFEXForwardElecAlgo - Algorithm for Forward Electron Algorithm in jFEX
//                              -------------------
//     begin                : 16 11 2021
//     email                : Sergi.Rodriguez@cern.ch
//     email                : sjolin@cern.ch
//***************************************************************************

#ifndef jFEXForwardElecAlgo_H
#define jFEXForwardElecAlgo_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IjFEXForwardElecAlgo.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "L1CaloFEXSim/jFEXForwardElecTOB.h"
#include "L1CaloFEXSim/jFEXForwardElecInfo.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"

namespace LVL1 {

  class jFEXForwardElecAlgo : public AthAlgTool, virtual public IjFEXForwardElecAlgo {

  public:
    /** Constructors **/
    jFEXForwardElecAlgo(const std::string& type, const std::string& name, const IInterface* parent);
    
    /** standard Athena-Algorithm method **/
    virtual StatusCode initialize() override;
    
    /** Destructor **/
    virtual ~jFEXForwardElecAlgo();
    
    /** Standard methods **/
    virtual StatusCode safetyTest() override;
    virtual StatusCode reset() override;
   
    virtual void setup(
      int inputTable[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width],
      int jfex, int fpga ) override;
    
    virtual std::unordered_map<uint, jFEXForwardElecInfo> calculateEDM() override;
    virtual void setFPGAEnergy(
      std::unordered_map<int,std::vector<int> > etmapEM,
      std::unordered_map<int,std::vector<int> > etmapHAD) override; 
    
  private:        
    SG::ReadHandleKey<LVL1::jTowerContainer> m_jTowerContainerKey {this, "MyjTowers", "jTowerContainer", "jTower input container"};
    SG::ReadHandle<jTowerContainer> m_jTowerContainer;
    std::unordered_map<int,std::vector<int> > m_map_Etvalues_EM;
    std::unordered_map<int,std::vector<int> > m_map_Etvalues_HAD;
    int m_jFEXalgoTowerID[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width];
    int m_jfex;
    int m_fpga;
    static constexpr float m_2PI = 2*M_PI;
    static constexpr float m_TT_Size_phi = M_PI/32;
    const int m_Edge_dR2 = std::round( (std::pow(2*M_PI/32,2)) * 1e5  );
    const int m_Edge_dR3 = std::round( (std::pow(3*M_PI/32,2)) * 1e5  );
    const int m_Edge_dR4 = std::round( (std::pow(4*M_PI/32,2)) * 1e5  );
        
    Gaudi::Property<std::string> m_IsoMapStr {this, "IsoMap", "Run3L1CaloSimulation/JetMaps/2024_04_09/jFEX_FWD_iso.dat", "Contains Trigger towers in (forward) EM layer used for isolation"};
    Gaudi::Property<std::string> m_Frac1MapStr {this, "Frac1Map", "Run3L1CaloSimulation/JetMaps/2024_04_09/jFEX_FWD_frac.dat", "Contains Trigger towers in FCal layer2 used for hadronic fraction 1 discriminant"};
    Gaudi::Property<std::string> m_Frac2MapStr {this, "Frac2Map", "Run3L1CaloSimulation/JetMaps/2024_04_09/jFEX_FWD_frac2.dat", "Contains Trigger towers in FCal layer3 used for hadronic fraction 2 discriminant"};
    Gaudi::Property<std::string> m_SearchGTauStr  {this, "SearchGTauMap", "Run3L1CaloSimulation/JetMaps/2024_04_09/jFEX_FWD_searchGTau.dat" , "Contains Trigger tower to find local max (greater than)"};
    Gaudi::Property<std::string> m_SearchGeTauStr {this, "SearchGeTauMap", "Run3L1CaloSimulation/JetMaps/2024_04_09/jFEX_FWD_searchGeTau.dat", "Contains Trigger tower to find local max (greater or equal than)"};
       
    std::unordered_map<unsigned int, std::vector<unsigned int> > m_SeedRingMap;
    std::unordered_map<unsigned int, std::vector<unsigned int> > m_1stRingMap;
    std::unordered_map<unsigned int, std::vector<unsigned int> > m_IsoMap;
    std::unordered_map<unsigned int, std::vector<unsigned int> > m_Frac1Map;
    std::unordered_map<unsigned int, std::vector<unsigned int> > m_Frac2Map;
    std::unordered_map<unsigned int, std::vector<unsigned int> > m_SearchGTauMap;
    std::unordered_map<unsigned int, std::vector<unsigned int> > m_SearchGeTauMap;
    
    virtual std::array<float,2> getEtaPhi(uint) override;
    virtual std::array<int,2> getEtEmHad(uint) const override;
    bool getEMSat(unsigned int ttID);
    
    bool isValidSeed(uint seedTTID) const;
    void findAndFillNextTT(jFEXForwardElecInfo& elCluster, int neta, int nphi);
    
    StatusCode ReadfromFile(const std::string& , std::unordered_map<unsigned int, std::vector<unsigned int> >&) const;
  };
  
}//end of namespace

CLASS_DEF( LVL1::jFEXForwardElecAlgo, 71453331, 1 )

#endif

