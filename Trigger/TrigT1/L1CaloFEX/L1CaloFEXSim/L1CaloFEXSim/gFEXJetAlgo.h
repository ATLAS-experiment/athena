/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXJetAlgo - JetFinder algorithm for gFEX
//                              -------------------
//     begin                : 01 04 2021
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#ifndef gFEXJetAlgo_H
#define gFEXJetAlgo_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IgFEXJetAlgo.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXSim/gFEXJetTOB.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"
#include <vector>
#include <memory>
#include <array>



namespace LVL1 {

  typedef  std::array<std::array<int, 6>, 32> gTowersJetEngine;
  typedef  std::array<std::array<int, 4>, 32> gTowersPartialSums;

  class gFEXJetAlgo : public AthAlgTool, virtual public IgFEXJetAlgo {

  public:
    /** Constructors */
    gFEXJetAlgo(const std::string& type, const std::string& name, const IInterface* parent);

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;

    virtual void pileUpCalculation(gTowersType &twrs, int rhoThreshold_Max, int inputScale,  int &PUCp) const override;
    
    virtual std::vector<std::unique_ptr<gFEXJetTOB>> largeRfinder(const gTowersType& Atwr, 
                                                                  const gTowersType& Btwr,
                                                                  const gTowersType& CNtwr,
                                                                  const gTowersType& Asat, 
                                                                  const gTowersType& Bsat,
                                                                  const gTowersType& CNsat,
                                                                  int pucA, int pucB, int pucC, int gLJ_seedThrA, int gLJ_seedThrB, int gLJ_seedThrC,
                                                                  int gJ_ptMinToTopoCounts1, int gJ_ptMinToTopoCounts2, 
                                                                  int jetThreshold, int gLJ_ptMinToTopoCounts1, int gLJ_ptMinToTopoCounts2,
                                                                  std::array<uint32_t, 7> & ATOB1_dat, std::array<uint32_t, 7> & ATOB2_dat,
                                                                  std::array<uint32_t, 7> & BTOB1_dat, std::array<uint32_t, 7> & BTOB2_dat,
                                                                  std::array<uint32_t, 7> & CTOB1_dat, std::array<uint32_t, 7> & CTOB2_dat) const override;

  private:

    virtual void singleHalf(const gTowersType & twrs, gTowersType & FPGAsum) const;

    virtual void InternalPartialAB(const gTowersType & twrs, gTowersPartialSums & lps, gTowersPartialSums & rps) const;
    
    virtual void addInternalLin(gTowersType & jets, gTowersPartialSums & partial) const;

    virtual void addInternalRin(gTowersType & jets, gTowersPartialSums & partial) const;

    virtual void pileUpCorrectionAB(gTowersType &jets, int puc) const;

    virtual void ZeroNegative( gTowersType & jets ) const;

    virtual void SaturateJets( gTowersType & jets, const gTowersType & sat ) const;

    virtual void SaturateBlocks( gTowersType & gBlkSum, const gTowersType & sat ) const;

    virtual void gBlockAB(const gTowersType& twrs, gTowersType & gBlkSum, gTowersType & hasSeed, int seedThreshold) const;
    
    virtual void blkOutAB(gTowersType & blocks, std::array<int, 32> & jetOutL, std::array<int, 32> & etaIndL, std::array<int, 32> & jetOutR, std::array<int, 32> & etaIndR) const;

    virtual void gBlockMax2(const gTowersType & gBlkSum, int BjetColumn, int localColumn, std::array<int, 3> & gBlockV, std::array<int, 3> & gBlockEta, std::array<int, 3> & gBlockPhi) const;
    
    virtual void gBlockMax192(const gTowersJetEngine& gBlkSum, std::array<int, 3> & gBlockVp, std::array<int, 3> & gBlockEtap, std::array<int, 3> & gBlockPhip, int index) const;

    virtual void gBlockVetoAB(gTowersType &jets, gTowersType& hasSeed) const;

    virtual void RemotePartialAB(const gTowersType& twrs, gTowersPartialSums & lps, gTowersPartialSums & rps) const;

    virtual void RemotePartialCN(const gTowersJetEngine& twrs, gTowersPartialSums & rps ) const;

    virtual void RemotePartialCP(const gTowersJetEngine& twrs, gTowersPartialSums & lps ) const;

    virtual void addRemoteRin(gTowersType & jets, const gTowersPartialSums & partial, int ps_upper, int ps_lower, int ps_shift) const;

    virtual void addRemoteLin(gTowersType & jets, const gTowersPartialSums & partial, int ps_upper, int ps_lower, int ps_shift) const;

    virtual void addRemoteCNin(gTowersType & jets, const gTowersPartialSums & partial, int ps_upper, int ps_lower, int ps_shift ) const;

    virtual void addRemoteCPin(gTowersType & jets, const gTowersPartialSums & partial, int ps_upper, int ps_lower, int ps_shift ) const;

    virtual void gJetVetoAB( gTowersType & twrs ,int jet_threshold ) const;
    
    virtual void jetOutAB(const gTowersType & jets,
                          std::array<int, 32> & jetOutL, std::array<int, 32> & etaIndL,
                          std::array<int, 32> & jetOutR, std::array<int, 32> & etaIndR) const;

    virtual void gJetTOBgen(const std::array<int, FEXAlgoSpaceDefs::ABCrows>& jetOut,
                            const std::array<int, FEXAlgoSpaceDefs::ABCrows>& etaInd,
                            int TOBnum, int jetThreshold, std::array<int, FEXAlgoSpaceDefs::gJetTOBfib> & gJetTOBs,
                            std::array<int, FEXAlgoSpaceDefs::gJetTOBfib> & gJetTOBv,
                            std::array<int, FEXAlgoSpaceDefs::gJetTOBfib> & gJetTOBeta,
                            std::array<int, FEXAlgoSpaceDefs::gJetTOBfib> & gJetTOBphi ) const;

    // virtual void singleAB(const gTowers& twrs, gTowersType & FPGAsum) const;
  
    SG::ReadHandleKey<LVL1::gTowerContainer> m_gFEXJetAlgo_gTowerContainerKey {this, "MyGTowers", "gTowerContainer", "Input container for gTowers"};
    
  };

} // end of namespace


#endif
