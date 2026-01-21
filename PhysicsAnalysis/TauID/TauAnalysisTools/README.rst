========================================================
TauAnalysisTools: Package hosting tools for tau analysis
========================================================

:authors: Dirk Duschinger
:contact: dirk.duschinger@cern.ch
:revision: Antonio De Maria
:contact: antonio.de.maria@cern.ch           

.. meta::
   :description: TauAnalysisTools: Package hosting tools for tau analysis
   :keywords: TauAnalysisTools, tau, TauEfficiencyCorrectionsTool, TauSelectionTool, TauSmearingTool, maddog

.. contents:: Table of contents

------------
Introduction
------------

This package is designed to provide dual-use tools for easy use of taus in your
analysis. Currently following tools are available:

* **TauSelectionTool:** generic tool to apply a set of requirements on tau candidates
* **TauSmearingTool:** currently support tau energy corrections
* **TauEfficiencyCorrectionsTool:** provides identification scale factors and the associated uncertainties
* **TauTruthMatchingTool:** performs matching of taus to the visible truth tau 4th-momentum
* **TauTruthTrackMatchingTool:** performs matching of tracks to truth taus and tracks to truth charged particles
    
All relevant information about the actual measurement of uncertainties
can be found here: `TauRecommendationsR22
<https://twiki.cern.ch/twiki/bin/view/AtlasProtected/TauRecommendationsR22>`_.

In case of any problems, issues or suggestions don't hesitate to contact the
authorsi or the TauCP conveners.

-----
Setup
-----

AnalysisBase (AthAnalysisBase)
---------------

First start with a clean shell and setup athena using sparse checkout, following the instructions from `Setup Sparse Checkout <://atlas-software.docs.cern.ch/athena/git/clone/#sparse-checkout>`_.

Then checkout the TauAnalysisTools package::

  git atlas addpkg TauAnalysisTools

Compile the package following the instructions from `Setting up to compile and test code <https://atlas-software.docs.cern.ch/athena/git/develop/>`_.
Please make sure to setup AnalysisBase (or AthAnalysisBase), for example typing::

  asetup AnalysisBase,25.2.12,here

---------------
General Remarks
---------------

For each tool the message level can be adjusted like::

  TauSelTool.msg().setLevel( MSG::VERBOSE );

--------
Examples
--------

An example implementation of all tools can be found for stand-alone mode in
``TauAnalysisTools/util/TauAnalysisToolsExample.cxx``. 

The example can be executed via::

  TauAnalysisToolsExample FILENAME [NUMEVENTS]

FILENAME has to point to a root file and NUMEVENTS is an integer of events to
process. If NUMEVENTS is not set all events are processed. The same is true if
the actual number of events in the root file is less than the given number. 
  
-----------------------------------
Particular information on the tools
-----------------------------------

More detailed information on how to use the tools can be found here:

* `TauSelectionTool <doc/README-TauSelectionTool.rst>`_
* `TauSmearingTool <doc/README-TauSmearingTool.rst>`_
* `TauEfficiencyCorrectionsTool <doc/README-TauEfficiencyCorrectionsTool.rst>`_
* `TauTruthMatchingTool <doc/README-TauTruthMatchingTool.rst>`_
* `TauTruthTrackMatchingTool <doc/README-TauTruthTrackMatchingTool.rst>`_
