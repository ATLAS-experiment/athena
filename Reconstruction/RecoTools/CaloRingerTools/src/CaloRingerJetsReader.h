/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// $Id: CaloRingerJetsReader.h 668867 2015-05-20 20:23:22Z wsfreund $
#ifndef CALORINGERTOOLS_CALORINGERJETSSREADER_H
#define CALORINGERTOOLS_CALORINGERJETSSREADER_H

// STL includes:
#include <string>

// Base includes:
#include "CaloRingerInputReader.h"
#include "CaloRingerTools/ICaloRingerJetsReader.h"
#include "CaloRingerReaderJetUtils.h"

// xAOD includes:
#include "xAODJet/JetContainer.h"

// StoreGate includes:
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"

namespace Ringer {

class CaloRingerJetsReader : public CaloRingerInputReader,
                                  public ICaloRingerJetsReader
{

  public:

    /// @name CaloRingerJetsReader ctors and dtors:
    /// @{
    /**
     * @brief Default constructor
     **/
    CaloRingerJetsReader(const std::string& type,
                     const std::string& name,
                     const IInterface* parent);

    /**
     * @brief Destructor
     **/
    ~CaloRingerJetsReader();
    /// @}

    /// Tool main methods:
    /// @{
    /**
     * @brief initialize method
     **/
    virtual StatusCode initialize() override;
    /**
     * @brief read electrons and populates @name decoMap with them and their
     * respective CaloRings.
     **/
    virtual StatusCode execute() override;
    /**
     * @brief finalize method
     **/
    virtual StatusCode finalize() override;
    /// @}


  private:

    /// @name CaloRingerJetsReader private methods:
    /// @{

    /// @}

    /** Add decorations for a given selector */
    StatusCode addSelectorDeco(const std::string &contName, const std::string &selName);

    /// Tool CaloRingerJetsReader props (python configurables):
    /// @{
    /**
     * @brief Jet collections.
     **/
    SG::ReadHandleKey<xAOD::JetContainer> m_inputJetContainerKey{
      this, "inputKey", "AntiKt4EMTopoJets", "Input jet container"
    };
    // SG::ReadHandleKey<xAOD::JetContainer> m_inputJetContainerKey;


    /// The CaloRings Builder functor:
    // BuildCaloRingsJetFctor<xAOD::IParticleContainer> *m_clRingsBuilderJetFctor;
    // BuildCaloRingsJetFctor<xAOD::Jet_v1> *m_clRingsBuilderJetFctor;
    BuildCaloRingsJetFctor<xAOD::JetContainer>* m_clRingsBuilderJetFctor;


    /// @}

};

} // namespace Ringer

#endif