/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration  
*/
//***************************************************************************  
//		jFEXmetAlgo - Algorithm for MET Algorithm in jFEX
//                              -------------------
//     begin                : 14 05 2021
//     email                : Sergi.Rodriguez@cern.ch
//***************************************************************************

#ifndef jFEXmetAlgo_H
#define jFEXmetAlgo_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IjFEXmetAlgo.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"


namespace LVL1 {

  class jFEXmetAlgo : public AthAlgTool, virtual public IjFEXmetAlgo{

  public:
    /** Constructors **/
    jFEXmetAlgo(const std::string& type, const std::string& name, const IInterface* parent);
   
    /** standard Athena-Algorithm method **/
    virtual StatusCode initialize() override;
    virtual StatusCode reset() override;

    /** Destructor **/
    virtual ~jFEXmetAlgo();

    virtual StatusCode safetyTest() override;
    virtual void setup(int FPGA[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_thin_algoSpace_width], int hemisphere) override;
    virtual void setup(int FPGA[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width], int hemisphere) override;

    virtual void buildBarrelmet()  override;
    virtual void buildFWDmet()  override;
    virtual int GetMetXComponent()  override;
    virtual int GetMetYComponent()  override;
    virtual int getTTowerET(unsigned int TTID ) override; 
    virtual bool getjXESat() const override; 
    virtual void setFPGAEnergy(std::unordered_map<int,std::vector<int> > et_map)  override;
    
protected:

  private:
        SG::ReadHandleKey<LVL1::jTowerContainer> m_jTowerContainerKey {this, "MyjTowers", "jTowerContainer", "Input container for jTowers"};
        SG::ReadHandle<jTowerContainer> m_jTowerContainer;
        
        std::vector<std::vector<int>> m_FPGA;
        std::vector<std::vector<int>> m_FPGA_phi02;
        std::vector<std::vector<int>> m_FPGA_fcal;
        std::vector<long long> m_met;
        std::vector<float> m_met_angle;
        std::vector<long long> m_met_Xcoord;
        std::vector<long long> m_met_Ycoord;
        long long m_Totalmet_Xcoord = 0;
        long long m_Totalmet_Ycoord = 0;
        
        bool getTTowerSat(unsigned int TTID );
        bool m_saturation = false;
        
        virtual void buildMetXComponent();
        virtual void buildMetYComponent();      
        
        // To avoid using floats in the firmware.
        static constexpr unsigned int m_firmware_scale = (1 << 9);
        static constexpr unsigned int m_firmware_bit_offset = 9;
        
        std::unordered_map<int,std::vector<int> > m_map_Etvalues;
        
        int m_hemisphere;
  };



}//end of namespace


//CLASS_DEF( LVL1::jFEXmetAlgo , 121222945 , 1 )

#endif
