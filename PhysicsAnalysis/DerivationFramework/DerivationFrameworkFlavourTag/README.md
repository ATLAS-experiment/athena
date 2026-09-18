# DerivationFrameworkFlavourTag

This package contains flavour-tagging derivation configurations and shared
helpers for the `DAOD_FTAG*` formats. Please contact the FTAG group for any
questions. Contact details can be found in the main FTAG docs: https://ftag.docs.cern.ch/

## Derivations

| Derivation | Main use | Notes |
| --- | --- | --- |
| `FTAG1` | Main FTAG derivation for flavour-tagging studies on MC | Unskimmed, with the most complete track, vertex, jet, truth, and FTAG-specific content in this package. |
| `FTAG1LITE` | An extremely light weight training derivation. Instead of storing all collections, decorate jets with vectors representing constituent variables. | |
| `FTAG2` | `ttbar` calibration derivation with dilepton selection | Built on top of `FTAG1` content and adds a two-lepton skim plus targeted thinning. Runs on data and MC. |
| `FTAG3` | Slim derivation for boosted `g->bb` and Xbb calibration studies | Requires at least one muon and one large-`R` jet, and keeps the large-`R` / VR track-jet content needed for calibration. |
| `FTAGPU` | FTAG derivation focused on by-vertex jet content and pile-up related studies | Includes `AntiKt4EMPFlowByVertexJets`, related thinning, and FTAG augmentations for this jet view. |
| `FTAGSSV` | PHYS-like derivation for the soft b-tagging calibration | Uses `PHYS` content plus the NVSI_SecVrt_Tight* secondary-vertex containers rebuilt under each tracking systematic variation. |
| `FTAGXBB` | Skimmed derivation for Xbb calibration | Requires at least one large-`R` UFO soft-drop jet and adds Xbb-oriented large-`R` discriminant content. |

## Shared modules

- `BjetTriggerContent.py`
  Trigger content helpers for b-jet related output.
- `BTaggingContent.py`
  Central definitions of saved b-tagging and Xbb variable lists. New output
  variables should normally be added here rather than hard-coded in derivation
  files.
- `FlowEnergyDecoratorConfig.py`
  Shared configuration for the flow-energy decoration used by FTAG
  augmentations.
- `FtagBaseContent.py`
  Common slimming content, trigger setup, truth content, and shared FTAG
  augmentations.
- `FtagDerivationConfig.py`
  Small, reusable configuration helpers for truth decorators and trigger-jet
  flavour-label decoration.
- `FtagVRJetConfig.py`
  The variable-R EMPFlow jet collection used for soft flavour-tagging studies:
  jet definition, reconstruction, soft-lepton association and truth augmentation.

## Coding guidelines

For this package, we would like to follow coding guidelines to keep the python files user- and
newcomer-friendly. In the following parts, the guidelines for coding are listed. Please try to
follow those when making changes to/adding code.

### General style

- Follow PEP 8 for naming, spacing, imports, and line length.
- Prefer `from __future__ import annotations` in new Python modules.
- Add type hints to public configuration functions and helper functions.
- Write a short module docstring or header comment explaining the purpose of
  the file.
- Use triple-quoted docstrings for public functions. Keep them short and
  action-oriented, for example: `"""Configure the derivation kernel for FTAG1."""`

### Naming

- Use `UpperCamelCase` for functions that return instances of the `ComponentAccumulator`.
  for example `FTAG1KernelCfg` and `ParentDecoratorCfg`.
- Use `snake_case` for helpers, functions and variables. Prefix with an underscore when the helper is
  private to its module, for example `_get_track_collection`.
- Use descriptive local names such as `slimming_helper`, `trigger_lists_helper`,
  `item_list`, and `extra_static_content`.
- Keep a consistent naming scheme for certain must-have parts of derivations. This
  includes `FTAG*Cfg`, `*KernelCfg`, `*SlimmingCfg`, and `*ExtraContentCfg`.
  These are usually common for all derivations (all derivations need a kernel config for
  example).

### Imports

- Group imports in this order: standard library, third-party / Athena, local
  package imports.
- NO wildcard imports.
- Import under `TYPE_CHECKING` when a type is only needed for annotations.
- Keep imports ordered alphabetical within each group (where practical).

### Slimming of Containers

- To reduce the size of derivations, we can add containers via special lists that are
  stored as attributes in the slimming helper. These lists add either a pre-defined
  set of variables (`SmartCollections`), all variables (`AllVariables`), or extra
  defined variables (`ExtraVariables`) for a container.
- Add variables/containers that are more fundermental through the `FtagBaseContent`.
  Derivation-specific additions should be made inside the derivation python file itself,
  not via an if-else block in the `FtagBaseContent`.
- Keep derivation-specific content grouped in clearly labeled sections:
  baseline content, extra variables, Run-4 additions, trigger content, and
  output stream setup.
- Put standard output-variable definitions in `BTaggingContent.py` whenever the
  content is reusable across derivations.

### Strings and formatting

- Split long strings across parentheses instead of using backslashes.
- For long aux-variable lists, prefer one logical entry per line or build them
  from helper functions.
- Keep comments short and useful. Explain why a block exists, not what each
  Python statement obviously does.

### Flags and configuration logic

- Make flag-dependent behaviour explicit and localised, for example
  `_is_run4(flags)` or `_get_track_collection(flags)`.
- When a helper accepts optional extra content, use `None` defaults and create
  the mutable list or dictionary inside the function.

### Backward-compatible package conventions

- Preserve public entry-point names used by transforms and job options. This
  includes mainly the actual derivation config name, e.g. `FTAG1Cfg`, as this
  is picked up by the derivation command.
- Keep existing stream names, derivation names, and helper signatures stable
  unless a coordinated package update is planned.

## Recommended pattern for new derivations

For a new derivation module, prefer the following layout:

1. Module header or docstring.
2. Imports and `TYPE_CHECKING` block.
3. `*KernelCfg`.
4. `*SlimmingCfg` or `*CoreCfg`.
5. `*ExtraContentCfg`, if needed.
6. Top-level `*Cfg` entry point.

This keeps the configuration entry points easy to discover and makes shared
behaviour easier to factor into `FtagBaseContent.py` or
`FtagDerivationConfig.py`.
