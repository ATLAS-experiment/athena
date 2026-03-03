/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// BoostedJetTaggerAlg.h
//
// Algorithm that will decorate taggers score using the JSSTaggersUtils
//
// Author: Antonio Giannini
///////////////////////////////////////////////////////////////////

#ifndef BOOSTEDJETTAGGERS_BOOSTEDJETTAGGERALG_H
#define BOOSTEDJETTAGGERS_BOOSTEDJETTAGGERALG_H


#include <AthenaBaseComps/AthAlgorithm.h>
#include <AthContainers/ConstDataVector.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODJet/JetContainer.h>

#include "BoostedJetTaggers/JSSTaggerBase.h"
#include "BoostedJetTaggers/ScaleFactors.h"

namespace BJT{

    /// \brief An algorithm for counting containers
    class BoostedJetTaggerAlg final : public AthAlgorithm {
        /// \brief The standard constructor
        public: 
            BoostedJetTaggerAlg(const std::string &name, ISvcLocator *pSvcLocator);

            /// \brief Initialisation method, for setting up tools and other persistent
            /// configs
            virtual StatusCode initialize() override;
            /// \brief Execute method, for actions to be taken in the event loop
            virtual StatusCode execute() override;
            /// We use default finalize() -- this is for cleanup, and we don't do any

        private:

            CP::SysListHandle m_systematicsList{this};
            CP::SysReadHandle<xAOD::JetContainer> m_jets{ this, "jets", "", "jet container to read"};
            
            // jet tagger WP tool
            // ToDo: should add interface?
            ToolHandle<JSSTaggerBase> m_tagger {this, "tagger", "", "Tagger Tool"};
            
            // scale factors tool
            ToolHandle<IJetDecorator> m_scalefactor {this, "scalefactor", "", "Scale Factors Tool"};

    };
}
#endif
