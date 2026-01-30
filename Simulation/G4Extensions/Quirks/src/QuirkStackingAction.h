/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef QUIRKSTACKINGACTION_H
#define QUIRKSTACKINGACTION_H

#include "G4UserStackingAction.hh"
#include "globals.hh"

class QuirkStackingAction : public G4UserStackingAction {
public:
    QuirkStackingAction(G4UserStackingAction* defaultStackingAction = 0);
    virtual ~QuirkStackingAction();
    virtual G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack);
    virtual void NewStage();
    virtual void PrepareNewEvent();

    QuirkStackingAction(const QuirkStackingAction&) = delete;
    QuirkStackingAction& operator=(const QuirkStackingAction&) = delete;

private:
    G4bool m_firstTrack;
    G4UserStackingAction* m_defaultStackingAction;
};

#endif
