# Flavour tagging physics validation plots

This package fills the flavour tagging histograms of the physics validation
ntuples. It reads a DAOD_PHYSVAL, applies an event and jet selection, and books
and fills one set of histograms per jet collection. The plots are compared
against the previous nightly in the ART test of
[FlavourTaggingTests](../../FlavourTaggingTests), and the same histograms are
used for the release validation of new software and new simulation samples.

The whole validation procedure is described in the
[FTAG PhysVal tutorial](https://atlassoftwaredocs.web.cern.ch/guides/ftag/validation-physval/).

## Producing the ntuple

```
Derivation_tf.py --inputDAOD_PHYSVALFile DAOD_PHYSVAL.pool.root \
                 --outputNTUP_PHYSVALFile NTUP_PHYSVAL.root \
                 --format NTUP_PHYSVAL --validationFlags doBtag
```

`scripts/runJetTagDQA.py` runs the same monitoring standalone, which is handier
when developing.

## Histogram names

Every histogram is called

```
BTag_<jetCollection>_<group>_<variable>[_<selection>]
```

and the groups are

| group | content |
| --- | --- |
| `global` | per event counters: number of jets, tracks and primary vertices |
| `jet` | jet kinematics |
| `tracks` | properties of the tracks associated to a jet |
| `sv1` | the SV1 secondary vertex variables |
| `muon` | the muon associated to a jet |
| `truth` | truth quantities, such as the truth decay length |
| `tagger_<TaggerName>` | the outputs of one tagger |

The optional selection is the truth flavour of the jet (`b`, `c`, `u`, `muon`,
or one of the large-R truth classes such as `hbb`), the sample the distribution
is meant for (`ttbar`, `Zprime`), or the working point and variable a tagger
efficiency is measured against. A histogram without a selection holds all jets.

In the ntuple itself every histogram of a collection sits directly in
`BTag/<jetCollection>/`, without further structure.

## What is booked

The booking follows the jet collection: small-R collections get the full set,
and the large-R collection only gets what it can fill, which is the common
event, jet and track histograms plus its own tagger outputs. The histogram
definitions, their binning and their axis titles live in
`data/PhysValBtag_VariablesMenu.json`. Its `RUN3` and `RUN4` groups hold the
definitions that differ between the two, and only the one matching the geometry
is read.

Which taggers are filled depends on the run as well, since the validation
follows what the reconstruction actually writes. In Run 3 those are the small-R
tagger from the b-tagging configuration (GN2v01), GN3EPCLV01 and GN3V03, in
Run 4 the Run 4 small-R tagger (GN2HLv01), and for large-R jets GN3XPV01.
Working point histograms are only booked for the taggers that have working
points: GN2v01 takes its cuts from the production CDI, GN3EPCLV01 from the
values in `python/JetTagDQAConfig.py`, and the others are shown as the
discriminant alone.

## Post-processing

Efficiencies and ROC curves are not filled by this package but derived
afterwards, from the configuration in
`PhysicsAnalysis/PATJobTransforms/share/physvalPostProcessingBTagConfig.yaml`.
Its entries are templated over the collection, the tagger and the working point
and are expanded against the histograms that a file actually contains, so a
tagger or working point that did not run is skipped instead of being reported as
missing.

## Merging the output

`scripts/mergePhysValFiles.py` adds up the ntuples of several jobs and sorts the
result into one folder per group and selection:

```
python mergePhysValFiles.py --input <folder-or-files> --pattern "*PHYSVAL*" \
                            --output NTUP_MERGE_PHYSVAL.root -d BTag
```

The sorting is derived from the histogram name, so a new histogram or tagger
needs no change here. The folder layout is what the web display shows, and the
DCube configuration of the ART test addresses the histograms through it, so
`PhysicsAnalysis/JetTagging/FlavourTaggingTests/config/dcube_config_FTAG.xml`
has to be kept in step with it.

The FTAG physics validation team runs its own post-processing on top of these
ntuples, which produces the comparison plots shown in the validation meetings.
It lives in
[ftag-validation](https://gitlab.cern.ch/aft/software/physics-validation/ftag-validation).
