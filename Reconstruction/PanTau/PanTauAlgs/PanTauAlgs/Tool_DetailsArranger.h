/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PANTAUALGS_TOOL_DETAILSARRANGER_H
#define PANTAUALGS_TOOL_DETAILSARRANGER_H

#include <string>
#include <vector>

#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/PropertyWrapper.h"

#include "xAODTau/TauJet.h"
#include "xAODParticleEvent/ParticleContainer.h"
#include "xAODPFlow/PFOContainer.h"
#include "TruthUtils/ParticleConstants.h"

#include "PanTauAlgs/ITool_DetailsArranger.h"
#include "PanTauAlgs/ITool_InformationStore.h"
#include "PanTauAlgs/TauConstituent.h"

#include <vector>
#include <string>

namespace PanTau {
    class PanTauSeed;
    class PanTauDetails;
}

namespace PanTau {

    /** @class Tool_DetailsArranger
        Tool for PID of Tau Seeds
        @author Sebastian Fleischmann
        @author Christian Limbach <limbach@physik.uni-bonn.de>
    */
  class Tool_DetailsArranger : public asg::AsgTool, virtual public PanTau::ITool_DetailsArranger {
    
    ASG_TOOL_CLASS1(Tool_DetailsArranger, PanTau::ITool_DetailsArranger)
    
    public:
        
        enum PanTauDetailsType {
            t_Int,
            t_Float,
            t_NTypes
        };
        
        Tool_DetailsArranger(const std::string &name);
        virtual ~Tool_DetailsArranger ();
        
        virtual StatusCode initialize();
        
        virtual StatusCode execute(PanTau::PanTauSeed* inSeed, xAOD::ParticleContainer& pi0Container, xAOD::PFOContainer& neutralPFOContainer) const;
        
    protected:
        
        ToolHandle<PanTau::ITool_InformationStore>  m_Tool_InformationStore{this, "Tool_InformationStore", "PanTau::Tool_InformationStore/Tool_InformationStore", "Tool handle to the information store tool"};

        Gaudi::Property<std::string> m_Tool_InformationStoreName{this, "Tool_InformationStoreName", "", "Tool handle to the information store tool"};
        
        void addPanTauDetailToTauJet(PanTauSeed* inSeed,
				     const std::string& featName,
				     xAOD::TauJetParameters::PanTauDetails detailEnum,
				     PanTauDetailsType detailType) const;
	
        StatusCode arrangePFOLinks(PanTau::PanTauSeed* inSeed, xAOD::TauJet* tauJet, xAOD::ParticleContainer& pi0Container, xAOD::PFOContainer& neutralPFOContainer) const;

        static void SetHLVTau(PanTau::PanTauSeed* inSeed, xAOD::TauJet* tauJet, const std::string& varTypeName_Basic) ;

	bool HasMultPi0sInOneCluster(const xAOD::PFO* pfo, int decayModeProto) const ;

	static void SetNeutralConstituentMass(xAOD::PFO* neutral_pfo, double mass) ;

	static void SetNeutralConstituentVectorMasses(const std::vector< ElementLink<xAOD::PFOContainer> >& neutralPFOLinks, xAOD::PFOContainer& neutralPFOContainer, double mass) ;

	std::vector< ElementLink< xAOD::PFOContainer > > CollectConstituentsAsPFOLinks( PanTau::PanTauSeed* inSeed,
											const std::vector< ElementLink< xAOD::PFOContainer > >& cellbased_neutralPFOLinks,
											PanTau::TauConstituent::Type type ) const;

	static void createPi0Vectors(xAOD::TauJet* tauJet, std::vector<TLorentzVector>& vPi0s, std::vector< std::vector< ElementLink<xAOD::PFOContainer> > > &vec_pi0pfos) ;
	
	static const constexpr float MASS_PI0 = ParticleConstants::piZeroMassInMeV;
        
        std::string m_varTypeName_Basic;

	bool m_init=false;
  public:
	inline bool isInitialized(){return m_init;}
        
    };
} // end of namespace PanTau
#endif // PANTAUALGS_TAUDISCRIMINANTTOOL_H
