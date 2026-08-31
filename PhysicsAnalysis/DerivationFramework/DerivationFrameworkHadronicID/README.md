# DerivationFrameworkHadronicID

This package contains the derivation configurations for the `DAOD_HID*` formats. These are combined
formats for hadronic identification: everything needed to study jet tagging, flavour tagging and
hadronic tau ID from a single derivation, instead of one format per group.

## Derivations

| Derivation | Main use | Notes |
| --- | --- | --- |
| `HID1` | Main unskimmed derivation for hadronic-identification studies | MC only. The union of the `FTAG1` content and the jet content of `JETM2`. Keeps the full track, vertex, jet, tau, truth and flavour-tagging content of `FTAG1` together with the extra jet reconstruction, low-level constituent inputs and substructure content of `JETM2`. |

`HID1` is meant to replace `FTAG1` and `JETM2` once it has been validated, so
please add new content here rather than to those formats.

## What comes from where

`HID1` does not import `FTAG1.py` or `JETM2.py`, so that both can eventually be
removed. It does reuse the shared content modules that are not tied to a single
format:

- From `DerivationFrameworkFlavourTag`: `FtagBaseContent`, `FtagDerivationConfig`
  and `FtagVRJetConfig` provide the common flavour-tagging slimming content,
  augmentations and the variable-R jet collection.
- From `DerivationFrameworkJetEtMiss`: `CommonJETMXContent` provides the
  low-level cluster, flow-element, UFO and tracking variable lists, and
  `JetCommonConfig` the helpers for adding jet collections and origin-corrected
  clusters to the slimming.

Where the two source formats disagreed, `HID1` takes the more inclusive option:

- No truth thinning. `HID1` follows `FTAG1` and applies no truth thinning.
- Jet trigger content is written, following `FTAG1`. `JETM2` writes no trigger
  content at all.
- Containers that `FTAG1` writes with all variables and `JETM2` with smart
  slimming are written with all variables.

## Running it

```bash
Derivation_tf.py --CA \
    --inputAODFile input.AOD.pool.root \
    --outputDAODFile output.pool.root \
    --formats HID1
```

This writes `DAOD_HID1.output.pool.root`. Nightly ART tests for `HID1` live in
`DerivationFrameworkART/DerivationFrameworkHadronicIDART`.

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
  action-oriented, for example: `"""Configure the derivation kernel for HID1."""`

### Naming

- Use `UpperCamelCase` for functions that return instances of the `ComponentAccumulator`.
  for example `HID1KernelCfg` and `ParentDecoratorCfg`.
- Use `snake_case` for helpers, functions and variables. Prefix with an underscore when the helper is
  private to its module, for example `_add_jet_content`.
- Use descriptive local names such as `slimming_helper`, `trigger_lists_helper`,
  `item_list`, and `extra_static_content`.
- Keep a consistent naming scheme for certain must-have parts of derivations. This
  includes `HID*Cfg`, `*KernelCfg`, `*CoreCfg`, and `*ExtraContentCfg`.
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
- Add variables/containers that are more fundamental through the shared content
  modules, `FtagBaseContent` for flavour tagging and `CommonJETMXContent` for the
  low-level jet inputs. Derivation-specific additions should be made inside the
  derivation python file itself, not via an if-else block in the shared module.
- Keep derivation-specific content grouped in clearly labeled sections:
  flavour-tagging content, jet content, Run-4 additions, trigger content, and
  output stream setup.
- Put standard output-variable definitions in the shared content modules whenever
  the content is reusable across derivations.

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
  includes mainly the actual derivation config name, e.g. `HID1Cfg`, as this
  is picked up by the derivation command.
- Keep existing stream names, derivation names, and helper signatures stable
  unless a coordinated package update is planned.

## Recommended pattern for new derivations

For a new derivation module, prefer the following layout:

1. Module header or docstring.
2. Imports and `TYPE_CHECKING` block.
3. `*KernelCfg`.
4. `*CoreCfg`.
5. `*ExtraContentCfg`, if needed.
6. Top-level `*Cfg` entry point.

This keeps the configuration entry points easy to discover and makes shared
behaviour easier to factor into the shared content modules.
