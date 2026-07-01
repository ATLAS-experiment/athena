FlavorTagInference
==================

We aim for this package to serve as a universal inference package across ATLAS ML inference workflows. 
For now, it is focused on flavor-tagging and hadronic tauID applications, but we hope to expand to other use cases in the future.
Currently, the package support ONNX models exported by [SALT](https://gitlab.cern.ch/aft/algorithms/salt/-/tree/main/salt?ref_type=heads), 
with an experimental Triton backend for remote (Inference as a Service) inference.
SALT models include metadata that allows this package to automatically map EDM inputs and model outputs, these I/O mappings follow the LWTNN convention. 
In priciple, any ONNX model with compatible metadata could be supported by this package, but SALT is the only supported export tool for now.

If you want to deploy ML models in Athena, please try to use this package as your runtime and decorator implementation.
If you face any issues, please feel free to reach out to atlas-cp-flavtag-recoalgs-conveners@cern.ch.
You can also find useful information in the [FTAG ATLAS Talk](https://atlas-talk.web.cern.ch/c/ftag/12) forum and the [FTAG Documents page](https://ftag.docs.cern.ch/).


FlavorTagInference provides the runtime used by ATLAS flavor-tagging and hadronic tauID neural networks to:

- load jet/tau and constituent features from EDM,
- run inference (local ONNX Runtime or remote Triton (Experimental) backends),
- decorate jets/taus with NN scores and auxiliary outputs,
- support single-model and multifold deployments,
- support pass-through decorations from JSON-defined variable lists.

The package is dual-use (Athena and AnalysisBase/AthAnalysis where supported)

Data Flow At A Glance
---------------------

1. Python configuration schedules `JetTagDecoratorAlg` (or
  `JetTagConditionalDecoratorAlg`) with `GNNTool`/`MultifoldGNNTool`.
2. The tool retrieves a shared `GNN` instance from `NNSharingOnnxSvc`,
  `NNSharingTritonSvc`, or `PassThroughModelSvc`.
3. `GNNDataLoader` reads model graph metadata and assembles scalar + sequence
  inputs from EDM using the appropriate constituent loaders.
4. `GNN` calls the selected backend (`SaltModel` for ONNX Runtime,
  `SaltModelTriton` for Triton) to run inference.
5. `GNN` decorates jets/taus with model outputs.
6. In multifold mode, `FoldDecoratorAlg` provides the fold hash that selects
  which model fold is applied per object.

What Is Current In This Package
-------------------------------

Core inference stack:

- `GNN`
  - Central runtime wrapper that combines EDM input loading and model inference.
  - Delegates model execution to an `ISaltModel` implementation.

- `GNNDataLoader`
  - Builds scalar and sequence inputs from model graph metadata.

- `SaltModel`
  - Local ONNX Runtime backend.
  - Reads `gnn_config` metadata from the ONNX file.

- `SaltModelTriton` (EXPERIMENTAL)
  - Triton gRPC backend.
  - Uses the local ONNX file to parse metadata and output schema,
    but executes inference remotely via Triton.


User-Facing Tools and Services
------------------------------

- `GNNTool`
  - ASG tool wrapper around `GNN`.

- `MultifoldGNNTool`
  - Wraps multiple networks (`nnFiles`) and picks fold by hash decoration
    (e.g. `jetFoldRankHash`).
  - Supports per-fold default outputs.

- `NNSharingOnnxSvc`
  - Caches `GNN` objects for local ONNX inference.

- `NNSharingTritonSvc` (EXPERIMENTAL)
  - Caches `GNN` objects for Triton inference.
  - Uses a configured map from ONNX path to Triton model name.

- `PassThroughModelSvc`
  - Builds a pass-through `GNN` from JSON config (`PassThroughSaltModel`).
  - Useful for directly writing selected jet/constituent inputs as output
    decorations without a learned model.


Algorithms
----------

- `JetTagDecoratorAlg`
  - Generic event algorithm that applies a decorator tool to all jets.

- `JetTagConditionalDecoratorAlg`
  - Applies decoration only when all configured tag-flag decorations are true;
    otherwise writes defaults.

- `FoldDecoratorAlg`
  - Produces per-jet fold hash decorations using event ID and optional jet/
    constituent entropy inputs.
  - Used by multifold deployments.


Constituent Loaders
-------------------

Constituent sequence nodes are interpreted by naming convention and mapped to
typed loaders:

- Tracks: `TracksLoader`
  - supports selection and sorting modes encoded in node names,
  - supports flip-tag track sign transformations for selected variables.

- Flow elements: `FlowElementsLoader`
- Hit-based sequences: `HitsLoader`
- Electrons: `ElectronsLoader`
- Muons: `MuonsLoader`
- Calo clusters: `CaloClusterLoader`
- Towers via ghost links: `TowerLoader`

The mapping logic is in `ConstituentsLoader.cxx` and is driven by regex-based
type matching and node-name conventions.


Python Configuration Entry Points
---------------------------------

Main module: `python/FlavorTagNNConfig.py`

- `FlavorTagNNCfg(...)`
  - Configures single-network jet tagging (`JetTagDecoratorAlg` + `GNNTool`).

- `MultifoldGNNCfg(...)`
  - Configures single- or multi-fold tagging.
  - If multiple networks are provided, also schedules fold decoration
    (`FoldDecoratorCfg`) and uses `MultifoldGNNTool`.

- `addAndReturnSharingSvc(...)`
  - Chooses Triton service when enabled and all requested models are in the
    ONNX-to-Triton map; otherwise falls back to ONNX service.

- `PassThroughModelCfg(...)`
  - Configures pass-through decorations from a JSON file.

Fold helper module: `python/FoldDecoratorConfig.py`

- `FoldDecoratorCfg(...)` schedules multiple fold-hash variants
  (with hits, without hits, rank-based).


Build/Platform Notes
--------------------

- Triton support (`SaltModelTriton`, `NNSharingTritonSvc`) is built only when
  not in `XAOD_ANALYSIS` mode.
- `MultifoldGNN` and `MultifoldGNNTool` are not built in `XAOD_STANDALONE`.


Developer Notes
---------------

- If you add a new constituent sequence type, update:
  - type/selection/sort mapping in `ConstituentsLoader.cxx`,
  - the corresponding loader implementation,
  - any pass-through JSON validation that references supported `input_key`
    values.

- If you add new output types, update:
  - `SaltModelOutput`,
  - model backend extraction (`SaltModel` and optionally `SaltModelTriton`),
  - decoration handling in `GNN`.

- If you add a new deployed tagger in python config helpers, keep
  `getDependencySet` and model default mappings synchronized.
