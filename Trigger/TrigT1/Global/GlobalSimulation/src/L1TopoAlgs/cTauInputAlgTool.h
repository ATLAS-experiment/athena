/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_CTAUINPUTALGTOOL_H
#define GLOBALSIM_CTAUINPUTALGTOOL_H

/**
 * AlgTool to obtain a GlobalSim::cTAUTOBArray
 * This class uses ReadHandles to jFex and eFex Tau Rois
 * If these objects are needed by another algorithm, runtime duplication
 * will occur. Future impreovement : have separate input Algorithms for
 *  jFex and eFex RoIs, and have cTauInputAlgTool use these results.
 */

#include "../IGlobalSimAlgTool.h"
#include "../IO/cTauTOBArray.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODTrigger/eFexTauRoIContainer.h"
#include "xAODTrigger/jFexTauRoIContainer.h"

#include "../IO/cTauTOBArray.h"


#include <string>


namespace GlobalSim {
  class cTauInputAlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {
    
  public:
    cTauInputAlgTool(const std::string& type,
			const std::string& name,
			const IInterface* parent);
    
    virtual ~cTauInputAlgTool() = default;

    StatusCode initialize() override;

    // adapted from L1TopoSimulation eFexInputProvider

    virtual StatusCode
    run(const EventContext& ctx) const override;
    
    virtual std::string toString() const override;
    
    
  private:

    SG::ReadHandleKey<xAOD::jFexTauRoIContainer>
    m_jTauRoIKey {this, "jTauRoIKey", "L1_jFexTauRoI",
	"jFex jTau container key"};

    SG::WriteHandleKey<GlobalSim::cTauTOBArray>
    m_cTauTOBArrayWriteKey {this, "TOBArrayWriteKey", "",
			   "key to write out a cTauTOBArray"};
 
    SG::ReadHandleKey<xAOD::eFexTauRoIContainer>
    m_eTauRoIKey {this, "eTauRoIKey", "L1_eTauRoI", "eFex eTau container key"};

    // used for jFex RoI to cTau TOB Conversion
    static constexpr int s_Et_conversion{2};            // 200 MeV to 100 MeV
    
    // used for eFex and jFex RoI to cTau TOB Conversion
    static constexpr double s_EtDouble_conversion{0.1};     // 100 MeV to GeV
    static constexpr double s_phiDouble_conversion{0.05};   // 20 x phi to phi
    static constexpr double s_etaDouble_conversion{0.025};  // 40 x eta to eta

    // j/eFex_cTaus fill a cTauTOBArray using inputs from jFex and eFEx

    StatusCode jFex_cTaus(GlobalSim::cTauTOBArray&,
			  const EventContext& ) const;
    StatusCode eFex_cTaus(GlobalSim::cTauTOBArray&,
			  const EventContext&) const;
  };
}
#endif
