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

// Use IdxSet and ParSet from FPGATrackSimUtil
using FPGATrackSimBinUtil::IdxSet;
using FPGATrackSimBinUtil::ParSet;
using FPGATrackSimBinUtil::StoredHit;

class FPGATrackSimKeyLayerBinDesc : public  extends<AthAlgTool, IFPGATrackSimBinDesc> {

public:
    FPGATrackSimKeyLayerBinDesc(const std::string& algname, const std::string &name, const IInterface *ifc) :
    base_class(algname, name, ifc), m_parNames({"zR1", "zR2", "phiR1", "phiR2", "xm"})
    {
      declareInterface<IFPGATrackSimBinDesc>(this);
    }

    virtual StatusCode initialize() override;

    virtual const std::string &parNames(unsigned i) const override { return m_parNames[i]; }

    // convert back and forth from pT, eta, phi, d0, z0 and internal paramater set
    virtual const ParSet
    trackParsToParSet(const FPGATrackSimTrackPars &pars) const override {
      return keyparsToParSet(m_keylyrtool.trackParsToKeyPars(pars));
    }
    virtual const FPGATrackSimTrackPars parSetToTrackPars(const ParSet &parset) const override {
      return m_keylyrtool.keyParsToTrackPars(parSetToKeyPars(parset));
    }

    // calculate the distance in phi or eta from a track defined by parset to a
    // hit these can be implemented as any variable in the r-phi or r-eta plane
    // (not necessarily eta and phi).
    virtual double phiResidual(const ParSet &parset, FPGATrackSimHit const *hit) const override {
        return m_keylyrtool.deltaX(parSetToKeyPars(parset), hit);
    }
  
    virtual double etaResidual(const ParSet &parset, FPGATrackSimHit const *hit) const override {
      return hit->getZ()- m_keylyrtool.zExpected(parSetToKeyPars(parset),hit->getR());
    }

    // figure out if step is r-phi or r-eta plan
    bool stepIsRPhi(const FPGATrackSimBinStep &step) const;
    bool stepIsREta(const FPGATrackSimBinStep &step) const;
    
    // idx should be with the definition specifed in the step
    // NOTE: the stored hit may be modified!
    virtual bool hitInBin(const FPGATrackSimBinStep &step, const IdxSet &idx,
                          StoredHit &storedhit) const override;

  private:
    // Configurable Properties
    Gaudi::Property<double> m_rin{this, "rin", {-1.0}, "Radius of inner layer for keylayer definition"};
    Gaudi::Property<double> m_rout{this, "rout", {-1.0}, "Radius of outer layer for keylayer definition"};
    Gaudi::Property<bool> m_approxMath{this, "approxMath", {false}, "Use approximate math to emulate possible firmware"};

    // convert to/from the KeyLyrPars struct and the ParSet
    ParSet keyparsToParSet(const FPGATrackSimKeyLayerTool::KeyLyrPars& keypars) const {
      return ParSet({keypars.z1,keypars.z2,keypars.phi1,keypars.phi2,keypars.xm});
    }
    FPGATrackSimKeyLayerTool::KeyLyrPars parSetToKeyPars(const ParSet &parset) const {
      return FPGATrackSimKeyLayerTool::KeyLyrPars(parset);
    }

    // Internal
    FPGATrackSimKeyLayerTool m_keylyrtool;
    const std::vector<std::string> m_parNames;

    const std::vector<unsigned> m_phipars{2, 3, 4};
    const std::vector<unsigned> m_etapars{1, 2};
    
};


#endif // FPGATrackSimKeyLayerBinDesc_H
