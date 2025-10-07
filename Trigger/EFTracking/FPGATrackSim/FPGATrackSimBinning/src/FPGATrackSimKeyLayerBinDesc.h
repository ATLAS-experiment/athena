// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimKeyLayerBinDesc_H
#define FPGATrackSimKeyLayerBinDesc_H

/**
 * @file FPGATrackSimKeyLayerBinDesc.h
 * @author Elliot Lipeles
 * @date Sept 10th, 2024
 * @brief Binning Classes for BinTool
 *
 * Declarations in this file (there are a series of small classes):
 *      class FPGATrackSimKeyLayerBinDesc
 *      
 * 
 * Overview of stucture:
 *    -- 
 * 

 * 
 * References:
 *
 */
#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimBinning/IFPGATrackSimBinDesc.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "FPGATrackSimBinning/IFPGATrackSimBinDesc.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"


#include "FPGATrackSimKeyLayerTool.h"

#include <cmath>
#include <initializer_list>
#include <string>
#include <vector>

#include"FPGATrackSimObjects/FPGATrackSimFunctions.h"

class FPGATrackSimKeyLayerBinDesc : public  extends<AthAlgTool, IFPGATrackSimBinDesc> {

public:
    /// Constructor
    using base_class::base_class;

    virtual StatusCode initialize() override;

    virtual const std::string &parNames(unsigned i) const override { return m_parNames[i]; }

    // convert back and forth from pT, eta, phi, d0, z0 and internal paramater set
    virtual const FPGATrackSimBinUtil::ParSet
    trackParsToParSet(const FPGATrackSimTrackPars &pars) const override {   
      FPGATrackSimKeyLayerTool::KeyLyrPars keypars = m_keylyrtool.trackParsToKeyPars(pars);
      if (m_fieldCorrection) {
        keypars.phi1+=fieldCorrection(m_fieldCorRegion, pars.qOverPt/1000.0 ,m_keylyrtool.R1());
        keypars.phi2+=fieldCorrection(m_fieldCorRegion, pars.qOverPt/1000.0 ,m_keylyrtool.R2());
      }
      keypars.phi1+=m_phiOffset;
      keypars.phi2+=m_phiOffset;
      return keyparsToParSet(keypars);
    }

    

    virtual const FPGATrackSimTrackPars parSetToTrackPars(const FPGATrackSimBinUtil::ParSet &parset) const override {
      FPGATrackSimKeyLayerTool::KeyLyrPars keypars = parSetToKeyPars(parset);
      keypars.phi1-=m_phiOffset;
      keypars.phi2-=m_phiOffset;
      return m_keylyrtool.keyParsToTrackPars(keypars);
    }

    // calculate the distance in phi or eta from a track defined by parset to a
    // hit these can be implemented as any variable in the r-phi or r-eta plane
    // (not necessarily eta and phi).
    virtual double phiResidual(const FPGATrackSimBinUtil::ParSet &parset, FPGATrackSimHit const *hit) const override {
        return m_keylyrtool.deltaX(parSetToKeyPars(parset), hit->getR(), hit->getGPhi() + m_phiOffset);
    }
  
    virtual double etaResidual(const FPGATrackSimBinUtil::ParSet &parset, FPGATrackSimHit const *hit) const override {
      return hit->getZ()- m_keylyrtool.zExpected(parSetToKeyPars(parset),hit->getR());
    }

    // figure out if step is r-phi or r-eta plan
    bool stepIsRPhi(const FPGATrackSimBinStep &step) const;
    bool stepIsREta(const FPGATrackSimBinStep &step) const;
    
    // idx should be with the definition specifed in the step
    // NOTE: the stored hit may be modified!
    virtual bool hitInBin(const FPGATrackSimBinStep &step, const FPGATrackSimBinUtil::IdxSet &idx,
                          FPGATrackSimBinUtil::StoredHit &storedhit) const override;

    
    // Structs to hold firmware LUTs for one bin
    // This allows the firmware calculation to be exactly reproduced 
    // here and comapared to the original/main algorithm
    struct phiLUTConsts { 
      double r_in;   
      double r_out;   
      double y;
      double x1p;
      double y1p;
      double cosb;
      double sinb;
      double x_m;
      double x_factor;
      double w_x;
      double w_in;
      double dw_dr;
      double phiShift(double phi, double r);
      double phiWindow(double r);
    };
    struct etaLUTConsts {
      double r_in;   
      double r_out;   
      double z_in;
      double dz_dr;
      double w_in;
      double dw_dr;
      double etaShift(double z, double r);
      double etaWindow(double r);
    };
    phiLUTConsts getPhiLUTConsts(const FPGATrackSimBinStep &step, const std::vector<unsigned>& idx) const;
    etaLUTConsts getEtaLUTConsts(const FPGATrackSimBinStep &step, const std::vector<unsigned>& idx) const;


    // Write the relevant LUT tables for firmware    
    virtual void writeLUTs(const FPGATrackSimBinStep &step) const override;
    

  private:
    // Configurable Properties
    Gaudi::Property<double> m_rin{this, "rin", {-1.0}, "Radius of inner layer for keylayer definition"};
    Gaudi::Property<double> m_rout{this, "rout", {-1.0}, "Radius of outer layer for keylayer definition"};
    Gaudi::Property<bool> m_approxMath{this, "approxMath", {false}, "Use approximate math to emulate possible firmware"};
    Gaudi::Property<double> m_d0pad{this, "D0Pad", 0.0, "Extra phi padding from d0 resolution"};
    Gaudi::Property<double> m_phipad{this, "PhiPad", 0.0, "Extra phi padding from phi resolution"};
    Gaudi::Property<double> m_qptpad{this, "QPtPad", 0.0, "Extra phi padding from q/pT resolution"};
    Gaudi::Property<double> m_z0pad{this, "Z0Pad", 0.0, "Extra eta padding from z0 resolution"};
    Gaudi::Property<double> m_etapad{this, "EtaPad", 0.0, "Extra eta padding from eta resolution"};
    Gaudi::Property<unsigned> m_region{this, "region", 0, "Region number, needed to write out lookup tables for test vectors"};
    Gaudi::Property<std::vector<double>> m_slPerEtaMod{
    this,
        "slPerEtaMod",
        std::vector<double>{19.0, 24.0, 29.0, 32.0, 18.1, 27.1, 24.1, 15.1, 30.8,
                            30.8, 26.2, 32.2, 32.2, 26.2, 54.6, 54.6, 40.2, 60.2},
        "Strip length per eta eta mod"
    };

    Gaudi::Property<double> m_phiOffset{this, "PhiOffset", 0.0, "Phi offset between local and global parameters (avoids phi wrap around effects in region code)"};

    Gaudi::Property<unsigned> m_fieldCorRegion  { this, "fieldCorRegion", 2, "region for fieldCorrection"};
    Gaudi::Property<bool> m_fieldCorrection {this, "fieldCorrection", true, "Use magnetic field correction for Hough transform"};
        
  
    // convert to/from the KeyLyrPars struct and the ParSet
    FPGATrackSimBinUtil::ParSet keyparsToParSet(const FPGATrackSimKeyLayerTool::KeyLyrPars& keypars) const {
      return FPGATrackSimBinUtil::ParSet({keypars.z1,keypars.z2,keypars.phi1,keypars.phi2,keypars.xm});
    }
    FPGATrackSimKeyLayerTool::KeyLyrPars parSetToKeyPars(const FPGATrackSimBinUtil::ParSet &parset) const {
      return FPGATrackSimKeyLayerTool::KeyLyrPars(parset);
    }

    // Internal
    FPGATrackSimKeyLayerTool m_keylyrtool;
    const std::vector<std::string> m_parNames{"zR1", "zR2", "phiR1", "phiR2", "xm"};

    const std::vector<unsigned> m_phipars{2, 3, 4};
    const std::vector<unsigned> m_etapars{0, 1};
    
};


#endif // FPGATrackSimKeyLayerBinDesc_H
