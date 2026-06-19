// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
//
#ifndef ASGANALYSISALGORITHMS_TREEMAKERALG_H
#define ASGANALYSISALGORITHMS_TREEMAKERALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include "AsgTools/PropertyWrapper.h"

namespace CP {

   /// Algorithm that creates an empty tree for subsequent algorithms
   /// to fill
   ///
   /// This is meant in conjunction with \ref TreeFillerAlg and one or
   /// more of tree-variable filler algorithms in-between.  The idea
   /// behind this specific design is that it allows multiple
   /// implementations of tree-variable filler algorithms to work
   /// together in filling different variables in the same tree, as
   /// well as making the configuration for each tree-variable filler
   /// algorithm simpler.
   ///
   /// @author Nils Krumnack <Nils.Erik.Krumnack@cern.ch>
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class TreeMakerAlg : public EL::AnaAlgorithm {

   public:
      /// Algorithm constructor
      using EL::AnaAlgorithm::AnaAlgorithm;

      /// @name Functions inherited from @c EL::AnaAlgorithm
      /// @{
      virtual StatusCode initialize() override;

      /// Function executed once per event
      StatusCode execute(const EventContext& /*ctx*/) override { return StatusCode::SUCCESS; }

      /// @}

   private:
      /// @name Algorithm properties
      /// @{

      /// The name of the output tree to write
      Gaudi::Property<std::string> m_treeName{
          this, "TreeName", "physics", "Name of the tree to write"};
      /// Flush setting for the output tree
      Gaudi::Property<int> m_treeAutoFlush{
          this, "TreeAutoFlush", 200, "AutoFlush value for the output tree"};

      /// @}

   }; // class TreeMakerAlg

} // namespace CP

#endif //  ASGANALYSISALGORITHMS_TREEMAKERALG_H
