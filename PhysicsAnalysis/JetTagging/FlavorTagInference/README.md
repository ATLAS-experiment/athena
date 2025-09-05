Flavor Taggging Inference
==========================

This packages contains the trival code that FTAG uses to 
run the GNN. The isolation of this code is to make it easier
to understand, maintain, and base other projects off of.

This code is meant to be identical between the main and 24.0 branches. 

It is meant as "stand-alone" code: it should be usable in
Athena, AthAnalysis, and AnalysisBase.

Package Overview
----------------

There are several user-level tools here:
  - `GNN`: Low-level implementation of the GNN taggers. Allows
           lower-level manipulation. It uses ONNX-based backend.

  - `GNNTool`: ASG Interface to `GNN`.

  - `MultifoldGNN`, `MultifoldGNNTool`: Like `GNN`, but initializes
    with a number of network "folds". The network applied to a given
    jet is determined by the `jetFoldHash`.


  - `FoldDecoratorAlg`: adds a hash (`jetFoldHash`) to each jet based
    on some jet and event information. This should ensure that the 
    applied fold (in the `MultifoldGNN`) is random.


### Other Files ###
There are also several tools that you _probably_ don't have to touch:

  - `CustomGetterUtils`: Models rely on some information that isn't stored in
    accessors that we can get with a string (i.e. `pt`, `eta`,
    ...). These are defined in `CustomGetterUtils`.
