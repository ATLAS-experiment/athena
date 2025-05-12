/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//              jFEXLargeRJetAlgo - Algorithm for large R jet Algorithm in jFEX
//                              -------------------
//     begin                : 22 01 2021
//     email                : varsiha.sothilingam@cern.ch
//***************************************************************************

#ifndef jFEXLargeRJetAlgo_H
#define jFEXLargeRJetAlgo_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IjFEXLargeRJetAlgo.h"
#include "L1CaloFEXToolInterfaces/IjFEXSmallRJetAlgo.h"
#include "L1CaloFEXSim/jTowerContainer.h"


namespace LVL1 {

  class jFEXLargeRJetAlgo : public AthAlgTool, virtual public IjFEXLargeRJetAlgo {

  public:
    /** Constructors */
    jFEXLargeRJetAlgo(const std::string& type, const std::string& name, const IInterface* parent);
   
    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;

    /** Destructor */
    virtual ~jFEXLargeRJetAlgo();

    virtual StatusCode safetyTest() override;
    virtual void setupCluster(int inputTable[15][15]) override;
    virtual unsigned int getRingET() override;
    virtual unsigned int getLargeClusterET(unsigned int smallClusterET, unsigned int largeRingET) const override;
    virtual void setFPGAEnergy(const std::unordered_map<int,std::vector<int> >& et_map)  override;
    virtual bool getLRjetSat() override;

  protected:

  private:

    SG::ReadHandleKey<LVL1::jTowerContainer> m_jTowerContainerKey {this, "MyjTowers", "jTowerContainer", "Input container for jTowers"};
    SG::ReadHandle<jTowerContainer> m_jTowerContainer;
    ToolHandle<IjFEXSmallRJetAlgo> m_jFEXSmallRJetAlgoTool {this, "jFEXSmallRJetAlgoTool", "LVL1::jFEXSmallRJetAlgo", "Tool that runs the jFEX Small R Jet algorithm"};

    bool getTTowerSat(unsigned int TTID );
    bool m_saturation =false;
    int m_largeRJetEtRing_IDs[15][15];
    int getTTowerET(unsigned int TTID ) ;
    std::unordered_map<int,std::vector<int> > m_map_Etvalues;
  };


}//end of namespace
#endif

