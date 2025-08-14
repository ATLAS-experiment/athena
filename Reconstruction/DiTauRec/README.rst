======================
Di-Tau Reconstruction
======================

:authors: David Kirchmeier
:contact: david.kirchmeier@cern.ch
:revision: Antonio De Maria
:contact: antonio.de.maria@cern.ch           

.. contents:: Table of contents

|
|
|

Introduction
============

The di-tau reconstruction package (``DiTauRec``) provides a new 
method for reconstructing boosted pairs of tau leptons. 
It is meant to be an addition to the approach of reconstructing single taus 
for a boosted region.
With the approach of single tau reconstruction it is not possible to reconstruct
tau pair topologies with low spatial separation, corresponding to a di-tau with dR < 0.4.

The current development of ``DiTauRec`` is focused on full-hadronic tau decays (hadhad). 
Additionally, a multi-variate di-tau identification algorithm based on a BDT is available to suppress QCD jet background with high efficiency.

``DiTauRec`` uses anti-kt-10 fat jets as seeds for its search for di-tau candidates. 
These are filtered for subjets. 
A candidate for a full-hadronic tau pair decay should include at least two subjets, while 
each subjet has to include at least one track. 

A general introduction into di-tau tagging in ATLAS can be found in `master thesis <https://cds.cern.ch/record/2105592>`_ or in `ATL-COM-PHYS-2019-707 <https://cds.cern.ch/record/2679279/files/ATL-COM-PHYS-2019-707.pdf>`_. 

|
|
|

Implementation
================

Di-Tau Builder
-------------------
`DiTauBuilder.cxx <src/DiTauBuilder.cxx>`_ is the main class for di-tau reconstruction.
It sets cuts on the seed jet and loops over all di-tau candidates. 
The di-tau candidate information is stored by 
`DiTauCandidateData.h <DiTauRec/DiTauCandidateData.h>`_.

`DiTauBuilderConfig.py <python/DiTauBuilderConfig.py>`_ is the main job options file.
It configures the DiTauBuilder class and defines the names of input and output containers, 
as well as the radius parameters for seed jet and subjet reconstruction.

|

Di-Tau Reconstruction Tools
---------------------------

`DiTauBuilder.cxx <src/DiTauBuilder.cxx>`_ makes use of several tools which are successively applied to a 
di-tau candidate.
The getter functions for these tools are defined in 
`DiTauToolsConfig.py <python/DiTauToolsConfig.py>`_.
These tools are applied in the following order:

* `SeedJetBuilder <src/SeedJetBuilder.cxx>`_ (stores seed jet information in 
  DiTauCandidateData)
* `SubjetBuilder <src/SubjetBuilder.cxx>`_ (reconstructs subjets within the seed jet)
* `VertexFinder <src/VertexFinder.cxx>`_ (finds the most likely primary vertex)
* `DiTauTrackFinder <src/DiTauTrackFinder.cxx>`_ (associates tracks to the seed jet and checks quality criteria)
* `CellFinder <src/CellFinder.cxx>`_ (stores information of calorimeter cells which are located inside the 
  subjets)
* `DiTauIDVarDecorator <src/DiTauIDVarDecorator.cxx>`_ (calculates the identifiation variables used by the Omni based discriminant)
* `DiTauOnnxDiscriminantTool <src/DiTauOnnxDiscriminantTool.xx>`_ (calculate the Omni score used for ditau identification)

These tools are loaded and configured in `DiTauBuilderConfig.py <python/DiTauBuilderConfig.py>`_ and are executed in `DiTauBuilder.cxx <src/DiTauBuilder.cxx>`_.

Note: the `DiTauIDVarDecorator`_ and the `DiTauOnnxDiscriminantTool`_ are not run at reconstruction level, but they are run at derivation level (xAOD -> DxAOD)

|

Di-Tau Event Data Model (EDM)
------------------------------

The implementation of the Di-Tau EDM can be found in
`Event/xAOD/xAODTau <https://gitlab.cern.ch/atlas/athena/-/tree/main/Event/xAOD/xAODTau/xAODTau/>`_.

| 
|
|

DiTauRec test in Full Reconstruction
----------------------------------

A unit test for the standalone Di-Tau reconstruction is available in `StandAloneDiTauBuilder.py <python/StandAloneDiTauBuilder.py>`_ .
To execute the test:

.. code-block:: bash

   athena.py --CA /path/to/athena/Reconstruction/DiTauRec/python/StandAloneDiTauBuilder.py

|
|
|

Di-Tau Container Readout
=========================

To get the Di-Tau container:

.. code-block:: c++
 
    // get the event
    xAOD::TEvent* event = wk()->xaodEvent();

    // get the di-tau container
    const xAOD::DiTauJetContainer* xDiTauJetContainer = 0;
    if ( !event->retrieve(xDiTauJetContainer, "DiTauJets").isSuccess() ) {
        Error("execute()", "Failed to retrieve DiTauJetContainer") );
        return EL::StatusCode::FAILURE;
    }

To loop over the di-tau candidates of the event and access their variables, e.g.:

.. code-block:: c++
  
    for (const auto* ditau: *xDiTauContainer){
      // di-tau pt
      pt = ditau->pt()

      // number of subjets
      n = ditau->nSubjets()

      // pt of the leading subjet
      pt = subjetPt(0)
      // pt of the subleading subjet
      pt = subjetPt(1)

      // f_core of the leading subjet (only if cell information was provided in sample production)
      fCoreLead = ditau->fCore(0)
      // f_core of the subleading subjet (only if cell information was provided in sample production)
      fCoreSubl = ditau->fCore(1)


To see all variables available in the DiTau EDM consult `DiTauJet_v1.cxx <https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODTau/Root/DiTauJet_v1.cxx>`_. 


|
|
|
|
