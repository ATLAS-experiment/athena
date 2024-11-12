/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//              jFEXSmallRJetAlgo - Algorithm for small R jet Algorithm in jFEX
//                              -------------------
//     begin                : 03 11 2020
//     email                : varsiha.sothilingam@cern.ch
//***************************************************************************

#ifndef jFEXSmallRJetAlgo_H
#define jFEXSmallRJetAlgo_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IjFEXSmallRJetAlgo.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXSim/jTowerContainer.h"

#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloIdManager.h" 
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "AthenaBaseComps/AthAlgorithm.h" 
#include "StoreGate/StoreGateSvc.h" 


namespace LVL1 {

  class jFEXSmallRJetAlgo : public AthAlgTool, virtual public IjFEXSmallRJetAlgo {

  public:
    /** Constructors */
    jFEXSmallRJetAlgo(const std::string& type, const std::string& name, const IInterface* parent);
   
    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;

    /** Destructor */
    virtual ~jFEXSmallRJetAlgo();

    virtual StatusCode safetyTest() override;
    virtual void setup(int inputTable[7][7], int inputTableDisplaced[7][7]) override;
    virtual unsigned int getTTowerET(unsigned int TTID ) const override;
    virtual void buildSeeds() override; 
    virtual bool isSeedLocalMaxima(int seedThreshold) override; 
    virtual unsigned int getSeedET() const override;
    virtual unsigned int getSmallClusterET() const override;
    virtual unsigned int getSmallETRing() const override;
    virtual unsigned int getTTIDcentre() const override;
    virtual bool getSRjetSat() const override;
    virtual void setFPGAEnergy(const std::unordered_map<int,std::vector<int> >& et_map) override;
    
  private:
        SG::ReadHandleKey<LVL1::jTowerContainer> m_jTowerContainerKey {this, "MyjTowers", "jTowerContainer", "Input container for jTowers"};
        SG::ReadHandle<jTowerContainer> m_jTowerContainer;
        
        void calcSaturation();
        bool getTTowerSat(unsigned int TTID );
        bool m_JetSaturation = false;
        
        int m_jFEXalgoTowerID[7][7] = {{0}};
        int m_jFEXalgoTowerID_displaced[7][7] = {{0}};
        int m_jFEXalgoSearchWindowSeedET[5][5] = {{0}};
        int m_jFEXalgoSearchWindowSeedET_displaced[5][5] = {{0}};
        
        std::unordered_map<int,std::vector<int> > m_map_Etvalues;
        
        bool CalculateLM(int mymatrix[5][5]);
  };


}//end of namespace
#endif
