===============
TauSmearingTool
===============

:authors: Dirk Duschinger
:contact: dirk.duschinger@cern.ch

.. contents:: Table of contents


-------
Preface
-------

**NOTE**: To use this tool it is necessary that the decoration
``truthParticleLink`` is available for each tau. Further this link should be a
valid link, that means, that the linked truth particle needs to be
accessible. I.e. if the linking is done in derivations to the TruthTau
container, the TruthTau container must be kept. For more information on how to
achieve this, please refer to the `tau pre-recommendations TWiki
<https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/TauPreRecommendations2015#Accessing_Tau_Truth_Information>`_

For more information on truth matching please refer to `TauTruthMatchingTool
<README-TauTruthMatchingTool.rst>`_.

------------
Introduction
------------

This tool provides corrections to your tau pT according to the TES
uncertainties. The momentum of xAOD taus have already been corrected for proper
tau energy scale, and therefore the tool returns for simulation a correction
factor of 1.

The tool can in general be used, including the header file::

  #include "TauAnalysisTools/TauSmearingTool.h"

The tool is created and initialized by calling::

  TauAnalysisTools::TauSmearingTool TauSmeTool( "TauSmaringTool" );
  TauSmeTool.initialize();

The smearing can be either applied to `non-const` taus::

  TauSmeTool.applyCorrection(xTau);

or by obtaining a `non-const` copy from a `const` tau::

  TauSmeTool.correctedCopy(xTauOrig, xTauCopy);

The set of recommended systematic variations can in general be retrieved by
calling::

  CP::SystematicSet recommendedSystematicSet = TauSmeTool.recommendedSystematics();

A complete set of available (including the recommended) systematic variations
are retrieved via::

  CP::SystematicSet affectingSystematicSet = TauSmeTool.affectingSystematics();

The tool can be configured to use a specific set of systematic variations calling::

  TauSmeTool.applySystematicVariation(customSystematicSet);


--------------------
Available properties
--------------------

Overview
========

Documentation about the uncertainties and the tool setup can be also found in `TWiki
<https://twiki.cern.ch/twiki/bin/view/AtlasProtected/TauRecommendationsR22>`_ 

The tool can be used to apply tau pt smearing for a specific
``RecommendationTag``:

.. list-table::
   :header-rows: 1
   :widths: 15 10 20 55
      
   * - property name
     - type
     - default value
     - other sensible values

   * - ``RecommendationTag``
     - ``std::string``
     - ``"2022-prerec"``
     - ``"2025-prerec"``

The following table lists other properties for further configurations for "2022-prerec" tag:


.. list-table::
   :header-rows: 1
   :widths: 15 10 20 55
      
   * - property name
     - type
     - default(alt) value
     - comment

   * - ``Campaign``
     - ``std::string``
     - ``"mc21"("mc20")``
     - For ``2022-prerec``, toggle between run-2 (``"mc20"``) and run-3 (``"mc21"``) pre-recommendations
   * - ``Generator``
     - ``std::string``
     - ``"PoPy"("Sherpa")``
     - For ``2022-prerec``, toggle between smearing/uncertainties calculated from PowhegPythia or Sherpa 
   * - ``MVATESQualityCheck``
     - ``bool``
     - ``true``
     - apply a compatibility check between calo TES and MVA TES and decorate the tau with a boolean "TESCompatibility" with the check results.


The following table lists other properties for further configurations for "2025-prerec" tag:

.. list-table::
   :header-rows: 1
   :widths: 15 10 20 55
      
   * - property name
     - type
     - default(alt) value
     - comment

   * - ``Campaign``
     - ``std::string``
     - ``"mc23"("mc20")``
     - For ``2025-prerec``, toggle between run-2 (``"mc20"``) and run-3 (``"mc23"``) pre-recommendations
   * - ``MVATESQualityCheck``
     - ``bool``
     - ``true``
     - apply a compatibility check between calo TES and MVA TES and decorate the tau with a boolean "TESCompatibility" with the check results.



Release Specific Configuration for "2022-prerec" tag: 
==============================
.. list-table::
   :header-rows: 1
   :widths: 10 5 15 10 10 15
      
   * - Release
     - Run
     - ``RecommendationTag``
     - ``Campaign``
     - ``Generator``
     - ``MVATESQualityCheck``
   * - ``>=22``
     - 2
     - ``"2022-prerec"``
     - ``"mc20"``
     - ``"PoPy"`` (default)
     - ``true`` (default)
   * - ``>=22``
     - 3
     - ``"2022-prerec"``
     - ``"mc21"`` (default)
     - ``"PoPy"`` (default)
     - ``true`` (default)

---
FAQ
---

#. **Question:** How can I access systematic variations for a specific nuisance
   parameter

   **Answer:** There are many ways to do that, one is for example on AFII up
   variation::

     // create and initialize the tool
     TauAnalysisTools::TauSmearingTool TauSmeTool( "TauSmearingTool" );
     TauSmeTool.initialize();

     // create empty systematic set
     CP::SystematicSet customSystematicSet;
     
     // add systematic up variation for AFII systematic and true hadronic taus to systematic set
     customSystematicSet.insert(CP::SystematicVariation ("TAUS_TRUEHADTAU_SME_RECO_AFII", 1));

     // tell the tool to apply this systematic set
     TauSmeTool.applySystematicVariation(customSystematicSet);

     // and finally apply it to a tau
     TauSmeTool.applyCorrection(xTau);
     
   if the down variation is needed, one just needs to use a ``-1`` in the line,
   where the systematic variation is added to the systematic set.

#. **Question:** I try to apply systematic variation running on derived samples,
   but I get an error like::
     
     TauAnalysisTools::getTruthParticleType: No truth match information available. Please run TauTruthMatchingTool first.

   **Answer:** Did you follow instructions for adding truth information in
   derivations as described in `TauPreRecommendations2015 TWiki
   <https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/TauPreRecommendations2015#Accessing_Tau_Truth_Information>`_?
   If not, do so!

#. **Question:** But I seriously can't wait for new derivations, is there a way
   to avoid the error due to the non existing ``truthParticleLink``?

   **Answer:** Yes there is, but this is only for testing purpose! One simply
   needs to set the property ``SkipTruthMatchCheck`` to true::

     TauEffTool.setProperty("SkipTruthMatchCheck", true );

#. **Question:** I try to apply systematic variation running on xAOD samples,
   but I get an error like::
     
     TauAnalysisTools::getTruthParticleType: No truth match information available. Please run TauTruthMatchingTool first.

   **Answer:** If you have full access to the TruthParticle container, you can
   create a TruthTau container and the link to the matched truth taus by setting
   up the `TauTruthMatchingTool <README-TauTruthMatchingTool.rst>`_ and to the
   truth matching for each tau.

----------
Navigation
----------

* `TauAnalysisTools <../README.rst>`_

  * `TauSelectionTool <README-TauSelectionTool.rst>`_
  * `TauSmearingTool <README-TauSmearingTool.rst>`_
  * `TauEfficiencyCorrectionsTool <README-TauEfficiencyCorrectionsTool.rst>`_

    * `TauEfficiencyCorrectionsTool Trigger <README-TauEfficiencyCorrectionsTool_Trigger.rst>`_

  * `TauTruthMatchingTool <README-TauTruthMatchingTool.rst>`_
  * `TauTruthTrackMatchingTool <README-TauTruthTrackMatchingTool.rst>`_
