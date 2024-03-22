/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCEVENTCOLLECTIONFILTER_HITSTRUTHRELINKBASE_H
#define MCEVENTCOLLECTIONFILTER_HITSTRUTHRELINKBASE_H

// Base class include
#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <GeneratorObjects/McEventCollection.h>
#include <GeneratorObjects/HepMcParticleLink.h>


class HitsTruthRelinkBase : public AthReentrantAlgorithm
{
public:
  HitsTruthRelinkBase(const std::string &name, ISvcLocator *pSvcLocator);

  virtual StatusCode initialize() override;

protected:
  StatusCode getReferenceBarcode(const EventContext &ctx, int *barcode) const;
  StatusCode getReferenceId(const EventContext &ctx, int *id) const;
  virtual HepMcParticleLink updatedLink(const EventContext &ctx, const HepMcParticleLink& oldLink, int referenceId, int pdgID=0) const;

  SG::ReadHandleKey<McEventCollection> m_inputTruthCollectionKey {this, "InputTruthCollection", "TruthEvent", "Input truth collection name"};
};

#endif // MCEVENTCOLLECTIONFILTER_HITSTRUTHRELINKBASE_H
