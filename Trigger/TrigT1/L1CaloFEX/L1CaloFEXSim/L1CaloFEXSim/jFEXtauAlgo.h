/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration  
*/
//***************************************************************************  
//		jFEXtauAlgo - Algorithm for Tau Algorithm in jFEX
//                              -------------------
//     begin                : 18 02 2021
//     email                : Sergi.Rodriguez@cern.ch
//***************************************************************************

#ifndef jFEXtauAlgo_H
#define jFEXtauAlgo_H



#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IjFEXtauAlgo.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include <vector>
#include <unordered_map>

namespace LVL1 {

  class jFEXtauAlgo : public AthAlgTool, virtual public IjFEXtauAlgo{

  public:
    /** Constructors **/
    jFEXtauAlgo(const std::string& type, const std::string& name, const IInterface* parent);
   
    /** standard Athena-Algorithm method **/
    virtual StatusCode initialize() override;

    /** Destructor **/
    virtual ~jFEXtauAlgo();

    virtual StatusCode safetyTest() override;
    virtual void setup(int seed[3][3]) override;
    

    virtual bool isSeedLocalMaxima() override;
    virtual bool isSeedLocalMaxima_fwd(unsigned int TTID) override;
    virtual void setFirstEtRing(int First_ETring[36])  override;
    virtual int getClusterEt() const override;
    virtual int getFirstEtRing() const override;
    virtual bool getTauSat() const override;
    virtual void setFPGAEnergy(const std::unordered_map<int,std::vector<int> >& et_map)  override;
    
protected:

  private:
        SG::ReadHandleKey<LVL1::jTowerContainer> m_jTowerContainerKey {this, "MyjTowers", "jTowerContainer", "Input container for jTowers"};
        SG::ReadHandle<jTowerContainer> m_jTowerContainer;
        
        Gaudi::Property<std::string> m_IsoRingStr  {this, "IsolationRingMap" , "Run3L1CaloSimulation/JetMaps/2023_02_10/jFEX_FWD_1stRing.dat" , "Contains Trigger tower for the isolation"};
        Gaudi::Property<std::string> m_SearchGStr  {this, "SearchGTauMap"    , "Run3L1CaloSimulation/JetMaps/2023_02_10/jFEX_FWD_searchGTau.dat" , "Contains Trigger tower to find local max (greater than)"};
        Gaudi::Property<std::string> m_SearchGeStr {this, "SearchGeTauMap"   , "Run3L1CaloSimulation/JetMaps/2023_02_10/jFEX_FWD_searchGeTau.dat", "Contains Trigger tower to find local max (greater or equal than)"};

        std::unordered_map<unsigned int, std::vector<unsigned int> > m_IsoRingMap;
        std::unordered_map<unsigned int, std::vector<unsigned int> > m_SearchGMap;
        std::unordered_map<unsigned int, std::vector<unsigned int> > m_SearchGeMap;     
        
        StatusCode ReadfromFile(const std::string& , std::unordered_map<unsigned int, std::vector<unsigned int> >&) const;
        int getTTowerET(unsigned int TTID ) const;
        bool getTTowerSat(unsigned int TTID );
        
        int m_TTwindow[3][3]={{0}};
        int m_ClusterEt = 0;
        int m_TauIsolation = 0;
        bool m_TauSaturation = false;
          
        std::unordered_map<int,std::vector<int> > m_map_Etvalues;

        struct color {
            std::string RED      ="\033[1;31m";
            std::string ORANGE   ="\033[1;38;5;208m";
            std::string YELLOW   ="\033[1;33m";
            std::string GREEN    ="\033[1;32m";
            std::string BLUE     ="\033[1;34m";
            std::string PURPLE   ="\033[1;35m";
            std::string END      ="\033[0m";
            std::string B_BLUE   ="\033[1;44m";
            std::string B_PURPLE ="\033[1;45m";
            std::string B_ORANGE ="\033[1;48;5;208;30m";
            std::string B_GRAY   ="\033[1;100m";
            std::string B_RED    ="\033[1;41m";
            std::string B_GREEN  ="\033[1;42m";
        } m_color;


        
  };



}//end of namespace


CLASS_DEF( LVL1::jFEXtauAlgo , 121222945 , 1 )

#endif
