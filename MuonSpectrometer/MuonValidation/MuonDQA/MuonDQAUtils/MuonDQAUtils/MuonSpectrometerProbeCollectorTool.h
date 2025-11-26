/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef Muon_MuonSpectrometerProbeCollectorTool_H
#define Muon_MuonSpectrometerProbeCollectorTool_H

#include "MuonDQAUtils/IProbeCollectorTool.h" //inheritance
#include "AthenaBaseComps/AthAlgTool.h" //inheritance

/// Gaudi Tools
#include "GaudiKernel/ToolHandle.h"
#include "MuonDQAUtils/IInsituTrackTools.h" //template argument to ToolHandle

#include <string>
namespace Rec{
  class TrackParticleContainer;
}

/** @class MuonSpectrometerProbeCollectorTool 


@author  Matthias Schott <mschott@cern.ch>
@author  Nektarios Chr. Benekos <nbenekos@illinois.edu>
*/  
namespace Muon 
{
  class MuonSpectrometerProbeCollectorTool : virtual public IProbeCollectorTool, public AthAlgTool
    {
    public:
      MuonSpectrometerProbeCollectorTool(const std::string&,const std::string&,const IInterface*);
	
      /** default destructor */
      virtual ~MuonSpectrometerProbeCollectorTool () {};
	
      /** standard Athena-Algorithm method */
      virtual StatusCode initialize();
		
      StatusCode createProbeCollection();
		
    private:
	
      Rec::TrackParticleContainer * m_MSProbeTrackContainer = nullptr;

      /// get a handle to the MuonSpectrometerProbeCollectorTool
      ToolHandle<IInsituTrackTools> m_InsituPerformanceTools;

      /** member variables for algorithm properties: */
      std::string	m_InnerTrackContainerName;
      std::string	m_MSTrackContainerName;
      std::string	m_CombinedMuonTracksContainerName;
      bool    m_RequireTrigger;
      float m_muonPtCut;

    }; 
}
#endif 
