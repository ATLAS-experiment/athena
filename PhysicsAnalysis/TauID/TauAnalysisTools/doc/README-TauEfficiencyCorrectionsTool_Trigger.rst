=======================================
TauEfficiencyCorrectionsTool -- Trigger
=======================================

:authors: Dirk Duschinger
:contact: dirk.duschinger@cern.ch

.. contents:: Table of contents

------------
Introduction
------------

Latest information on trigger scale factors  can be found in this  `twiki
<https://twiki.cern.ch/twiki/bin/view/Atlas/TauTriggerScaleFactors>`_. 
Other documentation is also in `link <https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/TauID/TauAnalysisTools/doc/README-TauEfficiencyCorrectionsTool.rst>`_.
Scale factors should be only applied to reconstructed taus matched to truth hadronic tau
decays. 

**IMPORTANT:** Use the tool only for reconstructed taus matched to the trigger!

-----------------
Quick start guide
-----------------
     
To get started you can do the following (for example for Run3 data)::
  
  TauAnalysisTools::TauEfficiencyCorrectionsTool TauTriggerEffTool( "TauTriggerEfficiencyCorrectionsTool" );

  CHECK(TauTriggerEffTool.setProperty("EfficiencyCorrectionTypes", std::vector<int>({SFTriggerHadTau}) ));
  CHECK(TauTriggerEffTool.setProperty("IDLevel", (int)JETIDRNNMEDIUM ));
  CHECK(TauTriggerEffTool.setProperty("TriggerName", "HLT_tau25_mediumRNN_tracktwoMVA" ));

  CHECK(TauTriggerEffTool.initialize());

Remember to use the cast to
int for enums, like in the setting of ``IDLevel`` to ``(int)JETIDRNNMEDIUM``.

Then in your loop you can apply or get scale factors from the tool by::

  TauTriggerEffTool.applyEfficiencyScaleFactor(xTau);                                     // either directly appending scale factors to the xAOD tau auxiliary store
  TauTriggerEffTool.getEfficiencyScaleFactor(xTau, dEfficiencyScaleFactor);               // or storing fake factors in variable dEfficiencyScaleFactor

Different set of scale factors are derived for Run2 and Run3, and also depending on the triggers used for analysis, for example "tracktwo" vs "tracktwoEF" in Run2. Scale factors are currently derived from the combination of two different measurements: Ztt and ttbar.    

--------------------
Available properties
--------------------

To be able to use trigger scale factors from the TauEfficiencyCorrectionsTool
one needs a separate tool instance with at least the following configuration:

.. list-table::
   :header-rows: 1
	      
   * - property name
     - variable type
     - default value
     - explanation
	 
   * - ``TriggerName``
     - ``std::string``
     - ``""`` (empty)
     - trigger name, like ``"HLT_tau25_mediumRNN_tracktwoMVA"``, other available
       option can be found below

   * - ``AutoTriggerYear``
     - ``bool``
     - ``false``
     - automatically detect the year if ``RandomRunNumber`` decoration exists
     
   * - ``IDLevel``
     - ``int``
     - ``JETIDRNNMEDIUM`` 
     - level of offline ID, it is the same property as for jet ID scale
       factors. A list of supported values can be found below

In addition the variable ``EfficiencyCorrectionTypes`` needs to be set to the
value ``std::vector<int>({SFTriggerHadTau})``

----------------------
Overview of Variations
----------------------

The recommended systematic variations are:

* ``TAUS_TRUEHADTAU_EFF_TRIGGER_STAT[X]``
* ``TAUS_TRUEHADTAU_EFF_TRIGGER_SYST[X]``

where [X] indicates the year for which the systematic uncertainty should be applied, e.g. 2023, 2022, etc

---------------------
Supported tau trigger
---------------------

**IMPORTANT:** Once again, use the tool only for taus matched to the trigger!

At the moment the following tau trigger are supported:

2022 and 2023 dataset
---------------------

* ``HLT_tau25_mediumRNN_tracktwoMVA``
* ``HLT_tau35_mediumRNN_tracktwoMVA``
* ``HLT_tau40_mediumRNN_tracktwoMVA``   
* ``HLT_tau60_mediumRNN_tracktwoMVA``
* ``HLT_tau80_mediumRNN_tracktwoMVA``
* ``HLT_tau160_mediumRNN_tracktwoMVA``

2018 dataset
------------

* ``HLT_tau25_medium1_tracktwoEF``
* ``HLT_tau35_medium1_tracktwoEF``  
* ``HLT_tau80L1TAU60_medium1_tracktwoEF``
* ``HLT_tau160L1TAU100_medium1_tracktwoEF``

2015-16-17 dataset
------------------
 
* ``HLT_tau25_medium1_tracktwo``
* ``HLT_tau35_medium1_tracktwo``  
* ``HLT_tau50L1TAU12_medium1_tracktwo``
* ``HLT_tau80L1TAU60_medium1_tracktwo``
* ``HLT_tau160_medium1_tracktwo``  

-----------------
Supported IDLevel
-----------------

At the moment the following offline ID working points are supported:

* ``JETIDRNNLOOSE``
* ``JETIDRNNMEDIUM``
* ``JETIDRNNTIGHT``

------------------
Supported binnings
------------------

The scale factors are all binned in reconstructed offline tau pt, with each scale factor binned according to the available statistic for each trigger. Scale factors for the trigger efficiency turn-on curve are also included 

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
