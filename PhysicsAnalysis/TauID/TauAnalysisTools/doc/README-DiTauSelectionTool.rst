================
DiTauSelectionTool
================

:authors: Dirk Duschinger, David Kirchmeier
:contact: antonio.de.maria@cern.ch,guillermo.nicolas.hamity@cern.ch, dirk.duschinger@cern.ch, david.kirchmeier@cern.ch

.. contents:: Table of contents 

------------
Introduction
------------


This tool intends to perform simple selections on a set of ditau properties. By
default a set of recommended cuts are applied. To use it you first need to
include the corresponding header file::

  #include "TauAnalysisTools/DiTauSelectionTool.h"
  
The tool at least needs to be created and initialized like::

  TauAnalysisTools::DiTauSelectionTool DiTauSelTool( "DiTauSelectionTool" );
  DiTauSelTool->initialize();
  
This creates the tool with no default cuts  

------------------
Tool configuration
------------------

The default config file looks like this::

  SelectionCuts: PtMin AbsEtaRegion

  PtMin: 20
  AbsEtaRegion: 0; 1.37; 1.52; 2.5

The top line lists the cuts to be applied. Below are the configurations on the
cuts, like the pt threshold of 20 GeV. If there is a cut specified, e.g. `PtMin:
20` but `PtMin` is not listed in `SelectionCuts`, the cut will not be made.

Entries with semicolons are treated as list of values, which only makes sense
for setups of vector type (e.g. ``AbsEtaRegion``).

It is possible to use a hashtag (``#``) as the first character of the line, to
ignore the line.

The following table gives an overview of all currently available cuts and their
setup:

.. list-table:: 
   :header-rows: 1
   :widths: 5 5 5 55 30
   
   * - Cut
     - Cut setup
     - Type
     - Description
     - Note
     
   * - ``CutPt``
     - ``PtRegion``
     - ``std::vector<double>``
     - accepting taus within pt regions (in GeV), each `odd` in the vector is a lower bound, each `even` is an upper bound
     -
     
   * -
     - ``PtMin``
     - ``double``
     - accepting taus with a pt above a lower bound (in GeV)
     - if ``PtMin`` is configured, ``PtRegion`` configuration wont be considered

   * -
     - ``PtMax``
     - ``double``
     - accepting taus with a pt below an upper bound (in GeV)
     - if ``PtMax`` is configured, ``PtRegion`` configuration wont be considered

   * - ``CutAbsEta``
     - ``AbsEtaRegion``
     - ``std::vector<double>``
     - accepting taus within absolute eta regions, each `odd` in the vector is a lower bound, each `even` is an upper bound
     -
     
   * -
     - ``AbsEtaMin``
     - ``double``
     - accepting taus with an absolute eta above a lower bound
     - if ``AbsEtaMin`` is configured, ``AbsEtaRegion`` configuration wont be considered

   * -
     - ``AbsEtaMax``
     - ``double``
     - accepting taus with an absolute eta below an upper bound
     - if ``AbsEtaMax`` is configured, ``AbsEtaRegion`` configuration wont be considered

   * - ``CutNSubjets``
     - ``NSubjetsRegion``
     - ``std::vector<float>``
     - accepting taus within number of subjets regions, each `odd` in the vector is a lower bound, each `even` is an upper bound
     -
     
   * -
     - ``NSubjetsMin``
     - ``float``
     - accepting taus with a number of subjets above a lower bound
     - if ``NSubjetsMin`` is configured, ``NSubjetsRegion`` configuration wont be considered

   * -
     - ``NSubjetsMax``
     - ``float``
     - accepting taus with a number of subjets below an upper bound
     - if ``NSubjetsMax`` is configured, ``NSubjetsRegion`` configuration wont be considered

   * - ``CutAbsCharge``
     - ``AbsCharges``
     - ``std::vector<int>``
     - accepting taus with a set of absolute charges, each value in the vector will be accepted
     - 

   * - 
     - ``AbsCharge``
     - ``double``
     - accepting taus with the given absolute charge
     - if ``AbsCharge`` is configured, ``AbsCharges`` configuration wont be considered


   * - ``CutOmniScore``
     - ``OmniScoreRegion``
     - ``std::vector<double>``
     - accepting ditaus within OmniScore regions, each `odd` in the vector is a lower bound, each `even` is an upper bound

   * -
     - ``OmniScoreMin``
     - ``double``
     - accepting ditaus with a OmniScore above a lower bound

   * -
     - ``OmniScoreMax``
     - ``double``
     - accepting ditaus with a OmniScore below an upper bound

If one wants to use a different setup one has three options:

1. Using an own config file
===========================

One needs to create a new file like the recommended_selection_r22.conf and
modify it as needed. You then have to tell the tool where it can find your
configuration file via::

  DiTauSelTool.setProperty( "ConfigPath", "/PATH/TO/CONFIG/FILE"); 

**IMPORTANT:** the last line of the file needs to be an empty line or should
only contain a comment (starting with the number sign #). Otherwise the tool
might be not properly configured (the last line is ignored by the file parser).

2. Overwrite particular cut setups or the list of cuts to be executed
=====================================================================

If particular cuts are modified, e.g. if one wants to select only ditaus above pT
> 100 GeV one would do::

  DiTauSelTool.setProperty("PtMin", 100.);

only the property will be overwritten, but all other cuts in the config file
will be applied as they are defined in the file.

Notes:

#. If one wants to specify the list of cuts to be applied, one can set the
   property ``SelectionCuts`` to a combination of enums defined in
   `DiTauSelectionTool.h <../TauAnalysisTools/DiTauSelectionTool.h>`_, which need to
   be casted to int, e.g.::

     DiTauSelTool.setProperty("SelectionCuts", int(TauAnalysisTools::DiTauCutPt |
                                                 TauAnalysisTools::DiTauCutAbsEta);

#. Vector based variables need to get a vector of the correct type. I.e. to
   achieve the same configuration as in the config file::
     
     AbsEtaRegion: 0; 1.37; 1.52; 2.5

   one needs the following code lines::

     std::vector<double> vAbsEtaRegion = {0, 1.37, 1.52, 2.5};
     DiTauSelTool.setProperty("AbsEtaRegion", vAbsEtaRegion);
   
3. Don't load any config file
=============================

If the property ``ConfigPath`` is set to an empty string::

  DiTauSelTool.setProperty( "ConfigPath", "");

no config file will be loaded. In this case, if no other properties are
configured, the tool will accept any ditau. 
     
----------------
Tool application
----------------

To test if a tau has passed all selection requirements just ask::

  DiTauSelTool.accept(xDiTau);

where xTau needs to be of type ``xAOD::DiTauJets`` or ``xAOD::IParticle*``. The
function returns a Root::TAccept value, equivalent to ``true``, in case all cuts
defined in the property ``"SelectionCuts"`` are passed, and equivalent to
``false`` otherwise. I.e. most users might make use of the following line in
their analyses::

  if (DiTauSelTool.accept(xDiTau))
  {
    // do stuff with accepted ditaus
    // ...
  }


------------------
Control histograms
------------------
     
This tool has the ability to create control histograms (currently it work not in
EventLoop). Therefore the `option` "CreateControlPlots" must be set to true::
     
  TauSelTool.setProperty("CreateControlPlots", true );

Also the tool needs to know where to write the histograms which is configured by
passing a pointer to the output file::

  TauSelTool.setOutFile( fOutputFile );

After all wanted selections have been made the histograms are written to the
file, via::

  TauSelTool.writeControlHistograms();
  
This adds a folder to the output file named by concatenating the tool name with
the prefix "_control". This folder contains a cutflow histogram showing the
number of processed tau objects before all cuts, and after each applied
cut. Additional control distributions before and after after all cuts are
stored in this folder.

---
FAQ
---

#. **Question:** How can I explicitly not perform a specific cut?

   **Answer:** This can be done by removing the cut name in the line starting
   with *SelectionCuts*.

#. **Question:** How can I find out, whether I correctly configured the tool and
   which cuts will be applied?

   **Answer:** If the tool is initialized with DEBUG message level
   (``TauSelTool->msg().setLevel( MSG::DEBUG );``) you will see for example such
   an output::

    TauSelectionTool          DEBUG Pt: 20 to inf
    TauSelectionTool          DEBUG AbsEta: 0 to 1.37
    TauSelectionTool          DEBUG AbsEta: 1.52 to 2.5
    TauSelectionTool          DEBUG cuts: Pt AbsEta

   **Note:** only the cuts in the last line will be processed

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
