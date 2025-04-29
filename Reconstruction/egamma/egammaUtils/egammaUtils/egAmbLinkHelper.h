/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/EventContext.h"

#include "AthContainers/Accessor.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEgamma/EgammaContainer.h"
#include "AthLinks/ElementLink.h"


namespace egAmbLinkHelper{
    template <typename SrcT, typename DestT>
    void doAmbiguityLinks(
      const EventContext& ctx,
      DataVector<SrcT> *srcContainer,
      DataVector<DestT> *destContainer
    ) {
      /// Needs the same logic as the ambiguity after building the objects (make
      /// sure they are all valid)
      static const SG::AuxElement::Accessor<
        std::vector<ElementLink<xAOD::CaloClusterContainer>>>
        caloClusterLinks("constituentClusterLinks");
  
      static const SG::AuxElement::Accessor<ElementLink<xAOD::EgammaContainer>>
        ELink("ambiguityLink");
  
      ElementLink<xAOD::EgammaContainer> dummylink;
      for (SrcT *src : *srcContainer) {
        ELink(*src) = dummylink;
        if (src->author() != xAOD::EgammaParameters::AuthorAmbiguous) {
          continue;
        }
  
        for (
          size_t destIndex = 0;
          destIndex < destContainer->size();
          ++destIndex
        ) {
          DestT *dest = destContainer->at(destIndex);
          if (dest->author() != xAOD::EgammaParameters::AuthorAmbiguous) {
            continue;
          }
  
          if (caloClusterLinks(*(dest->caloCluster())).at(0) ==
              caloClusterLinks(*(src->caloCluster())).at(0)) {
            ElementLink<xAOD::EgammaContainer> link(
              *destContainer, destIndex, ctx);
            ELink(*src) = link;
            break;
          }
        }
      }
    }
  }