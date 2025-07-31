/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASTOOLS__G4UA_G4ATLASPRIMARYGENERATORACTION_H
#define G4ATLASTOOLS__G4UA_G4ATLASPRIMARYGENERATORACTION_H

// STL includes
#include <vector>

// Geant4 includes
#include <G4VUserPrimaryGeneratorAction.hh>


namespace G4UA
{

  /// @class G4AtlasPrimaryGeneratorAction
  /// @brief ATLAS subclass of the G4 primary generator action.
  ///
  /// Maintains a list of custom actions for primary generation
  /// and when invoked by Geant4 will forward the call to each of them in turn.
  ///
  /// @todo TODO lifetime management of wrapper actions.
  ///
  /// @author Julien Esseiva <julien.esseiva@cern.ch>
  ///
  class G4AtlasPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
  {

    public:

      /// @brief Geant4 method for primary generation.
      /// This method forwards the G4 call onto each of its
      /// primary generator actions.
      void GeneratePrimaries(G4Event* anEvent) override final;

      /// Add one action to the list
      void addPrimaryGeneratorAction(G4VUserPrimaryGeneratorAction* action);

    private:

      /// List of ATLAS primary generator actions
      std::vector<G4VUserPrimaryGeneratorAction*> m_actions;

  }; // class G4AtlasPrimaryGeneratorAction

}

#endif
