Scale Factor Tools
=====================================

This package provides a generic framework for evaluating scale factors from external JSON configuration files.

The implementation is designed to be flexible and configuration-driven, allowing different calibration schemes, binning definitions, and systematic uncertainty variations to be described without introducing tagger-specific code.

It should be usable in AnalysisBase and AthAnalysis.

Package overview
---------------------------

The main user-facing tool is:
  - `ScaleFactorTool`: generic ASG tool for retrieving scale factor from JSON calibration files and applying systematic variations through the standard CP interfaces.

### Internal Utilities ###

There are also several internal helper classes that most users should not need to interact with directly:
  - `VariableFactory`: Provides access to variables used for binning and scale factor lookup. Both floating-point and integer varaibles are supported.
  - `QuantileFactory`: Builds binning and quantisation functions from the configuration file. Several binning schemes are supported:
    - `makeCategory`: Maps discrete values to category indices.
    - `makeEnumerate`: Converts a continuous variable into a bin index using an ordered list of bin edges.
    - `makeNodes`: Builds a tree-based lookup structure used for  node-based binning, primarily for pseudo-continuous 2D tagging.
    - `makeDense`: Combines multiple binning dimensions into a  single flattened global index used for scale factor lookup.

