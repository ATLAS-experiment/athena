# ATLAS Release Tester flavour tagging code

The purpose of this package:
- Runs the flavour tagging physics validation chain on an AOD and compares the histograms with the previous nightly, using the plots produced by [JetTagDQA](../JetTagValidation/JetTagDQA).
- Tests trackless b-tagging functionality from RDOs to AODs.
- Provides small standalone utilities to inspect b-tagging, jet link and jet constituent content of an xAOD.

In this way changes in the flavour-tagging and b-jet trigger code can be tested on a nightly basis.
More informations on how to use the b-jet code and run trackless b-tagging can be found in the [trigger implementation](https://ftag-sw.docs.cern.ch/trigger/) and [AODs with hits](https://ftag-sw.docs.cern.ch/DAOD/hits/) sections in the [Flavour Tagging Software Page](https://ftag-sw.docs.cern.ch/).
