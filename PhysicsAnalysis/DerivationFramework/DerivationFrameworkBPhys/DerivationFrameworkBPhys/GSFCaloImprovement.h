/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  Contact: Tomas Jakoubek <tomas.jakoubek@cern.ch>
*/

#ifndef DERIVATIONFRAMEWORKBPHYS_GSFCALOIMPROVEMENT_H
#define DERIVATIONFRAMEWORKBPHYS_GSFCALOIMPROVEMENT_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/TrackParticle.h"

namespace Trk {
    class ITrackParticleCreatorTool;
    class ITrackSummaryTool;
}

class IegammaTrkRefitterTool;

namespace DerivationFramework {
    class GSFCaloImprovement : public AthAlgTool, public IAugmentationTool {
    public: 
        /** @brief Default constructor **/
        GSFCaloImprovement(const std::string& t, const std::string& n, const IInterface* p);
        /** @brief Destructor **/
        virtual ~GSFCaloImprovement();
        /** @brief initialize method **/
        virtual StatusCode initialize();
        /** @brief finalize method **/
        virtual StatusCode finalize();
        /** @brief addBranches method **/
        virtual StatusCode addBranches() const override;

    private:
        /** @brief retrieve ImprovedRefitTool **/
        StatusCode RetrieveImprovedRefitTool();
        /** @brief retrieve ParticleCreatorTool **/
        StatusCode RetrieveParticleCreatorTool();
        /** @brief retrieve ParticleSummaryTool **/
        StatusCode RetrieveParticleSummaryTool();

        /** @brief Copy TrackParticle info from the original TP **/
        void copyInfo(const xAOD::TrackParticle& original, xAOD::TrackParticle& created, bool isRefitted) const;

        /** @brief Helper function to copy TP summary **/
        void copySummaryValue(const xAOD::TrackParticle& original, xAOD::TrackParticle& created, xAOD::SummaryType type) const {
            uint8_t dummy(0);
            uint8_t value = original.summaryValue(dummy, type) ? dummy : 0;
            created.setSummaryValue(value, type);
        }

        /** @brief The track refitter **/
        ToolHandle<IegammaTrkRefitterTool> m_trkImprovedRefitTool;
        /** @brief Tool to create track particle **/
        ToolHandle<Trk::ITrackParticleCreatorTool> m_particleCreatorTool;
        /** @brief Tool for Track summary **/
        ToolHandle<Trk::ITrackSummaryTool> m_summaryTool;

        /** @brief Option if running on AOD **/
        bool m_isAOD;

        /** @brief Option to do truth **/
        bool m_doTruth;

        /** @brief Option to copy pixel holes estimation **/
        bool m_doPix;

        /** @brief Option to copy SCT holes estimation **/
        bool m_doSCT;

        /** @brief Option to copy TRT holes estimation **/
        bool m_doTRT;

        /** @brief Minimum number of silicon hits on track before it is allowed to be refitted **/
        int m_minNSiHits;

        /** @brief Name of the electron input collection **/
        std::string m_electronCollectionKey;
        /** @brief Name of the GSF-CALO-refit output collection **/
        std::string m_gsfCaloOutputName;

        /** Counters **/
        mutable unsigned int m_allElectrons;
        mutable unsigned int m_noTP;
        mutable unsigned int m_onlyTRT;
        mutable unsigned int m_noTrk;
        mutable unsigned int m_failedFits;
        mutable unsigned int m_successfulFits;
        mutable unsigned int m_noRefTP;
        mutable unsigned int m_allNewTP;
    }; 
}

#endif // DERIVATIONFRAMEWORKBPHYS_GSFCALOIMPROVEMENT_H
