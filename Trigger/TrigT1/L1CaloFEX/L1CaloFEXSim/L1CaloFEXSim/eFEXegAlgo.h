/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXegAlgo.h  -  
//                              -------------------
//     begin                : 24 02 2020
//     email                : antonio.jacques.costa@cern.ch ulla.blumenschein@cern.ch tong.qiu@cern.ch
//  ***************************************************************************/


#ifndef eFEXegAlgo_H
#define eFEXegAlgo_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IeFEXegAlgo.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXSim/eFEXegTOB.h"
#include "L1CaloFEXSim/eTowerContainer.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"

namespace LVL1 {
  
  //Doxygen class description below:
  /** The eFEXegAlgo class calculates the egamma TOB variables: Reta, Rhad and Wstot
  */
  
  class eFEXegAlgo : public AthAlgTool, virtual public IeFEXegAlgo {

  public:
    /** Constructors */
    eFEXegAlgo(const std::string& type, const std::string& name, const IInterface* parent);

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;
    
    /** Destructor */
    virtual ~eFEXegAlgo();

    virtual StatusCode safetyTest() const override;
    virtual void setup(int inputTable[3][3], int efex_id, int fpga_id, int central_eta) override; 

    virtual void getReta(std::vector<unsigned int> & ) override;
    virtual void getRhad(std::vector<unsigned int> & ) override;
    virtual void getWstot(std::vector<unsigned int> & ) override;
    virtual void getRealPhi(float & phi) override;
    virtual void getRealEta(float & eta) override;
    virtual std::unique_ptr<eFEXegTOB> geteFEXegTOB() override;
    virtual void getClusterCells(std::vector<unsigned int> &cellETs) override;
    virtual unsigned int getET() override;
    virtual unsigned int dmCorrection(unsigned int ET, unsigned int layer) override;
    virtual void getWindowET(int layer, int jPhi, int SCID, unsigned int &) override;
    virtual bool hasSeed() const override {return m_hasSeed;};
    virtual unsigned int getSeed() const override {return m_seedID;};
    virtual unsigned int getUnD() const override {return m_seed_UnD;};
    virtual void getCoreEMTowerET(unsigned int & et) override;
    virtual void getCoreHADTowerET(unsigned int & et) override;
    virtual void getSums(unsigned int seed, bool UnD, 
                         std::vector<unsigned int> & RetaSums, 
                         std::vector<unsigned int> & RhadSums, 
                         std::vector<unsigned int> & WstotSums) override;
  private:
    void setSeed();
    bool m_seed_UnD = false; 
    unsigned int m_seedID = 999;
    int m_eFEXegAlgoTowerID[3][3]{};
    int m_efexid{};
    int m_fpgaid{};
    int m_central_eta{};
    bool m_hasSeed{};

    mutable int m_corrections[3][25] ATLAS_THREAD_SAFE = {
      {0,0,0,0,0,0,0,0x8,0,0,0xb,0x4,0x8,0x9,0x34,0x7e,0x7b,0x6b,0,0,0,0,0,0,0xc},
      {0xe,0x12,0x12,0x12,0x12,0x13,0x18,0x17,0x42,0x40,0x38,0x3d,0x3b,0x4e,0x2d,0xc,0x10,0x4,0x27,0x19,0x19,0x16,0x12,0x10,0xc},
      {0xb,0x8,0x8,0x8,0x8,0x8,0x7,0x9,0x8,0x8,0x8,0x7,0x8,0x8,0x21,0x2,0x2,0x4,0x6,0x8,0x8,0x8,0x9,0x10,0x12}
    };

    // Enable dead material corrections
    Gaudi::Property<bool> m_dmCorr  {this, "dmCorr", false, "Enable dead material correctionst"};
    Gaudi::Property<int> m_algoVersion  {this, "algoVersion", 0, "AlgoVersion, part of the L1Menu spec"};

    // Key for input towers
    SG::ReadHandleKey<LVL1::eTowerContainer> m_eTowerContainerKey {this, "MyETowers", "eTowerContainer", "Input container for eTowers"};

    // Key for reading dm corrections
    SG::ReadCondHandleKey<CondAttrListCollection> m_dmCorrectionsKey{this,"DMCorrectionsKey","",
                                                                 "Key to dead material corrections (AttrListCollection)"};

  };
  
} // end of namespace

//CLASS_DEF( LVL1::eFEXegAlgo, 32202260 , 1 )

#endif
