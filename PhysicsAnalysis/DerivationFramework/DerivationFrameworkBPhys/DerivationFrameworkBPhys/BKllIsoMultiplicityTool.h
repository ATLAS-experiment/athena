#ifndef DERIVATIONFRAMEWORK_BKllIsoMultiplicityTool_H
#define DERIVATIONFRAMEWORK_BKllIsoMultiplicityTool_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "RecoToolInterfaces/ITrackIsolationTool.h"
#include <vector>

/** THE reconstruction tool
 */
namespace DerivationFramework {

  class BKllIsoMultiplicityTool : public AthAlgTool, public IAugmentationTool {
    public: 
      BKllIsoMultiplicityTool(const std::string& t, const std::string& n, const IInterface* p);

      StatusCode initialize();
      StatusCode finalize();
      
      virtual StatusCode addBranches() const;
      bool isTrackInVertex(const xAOD::Vertex* theVtx, const <const xAOD::TrackParticle*> thePart) const;
      
    private:

	std::string m_trackContainerName;
	std::string m_vertexContainerName;
	std::vector<unsigned int> m_cones;
  }; 
}

#endif
