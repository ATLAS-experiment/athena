# PhysVal PostProcessing

## Overview

The script physvalPostProcessing.py (in scripts directory) along with physvalPostProcessingTools.py (in python directory) designed for post-processing of **PhysVal** histograms from ATLAS physics validation tasks. The code can be used to manipulate input histograms, and output the results to a new file. The yaml configuration files are also provided in share directory.

## Prerequisites

Before running the script, ensure you are in the **ROOT** environment.

## Input Data

To run the code locally, an input sample is required. A good example is the ttbar sample.

### Example ttbar sample:
- Reference sample: [ATLPHYSVAL-1225](https://its.cern.ch/jira/browse/ATLPHYSVAL-1225)
- Take reference sample from: [Input List Request](https://prodtask-dev.cern.ch/prodtask/inputlist_with_request/62487/)
- Slice 3 output (independent of any post-processing other than Tracking): [Task 44119457 Output](https://bigpanda.cern.ch/task/44119457/)

#### Example of expected input file:

- `valid1.601230.PhPy8EG_A14_ttbar_hdamp258p75_dil.merge.NTUP_PHYSVAL.e8514_s4481_s4469_r16448_p6745_p6746_p6747_tid44119457_00/NTUP_PHYSVAL.44119457._000001.pool.root.1`

---

## To run the code standalone:

### 1. Run with the Input File:

To run the code with just the input file, use:

```bash
python3 physvalPostProcessing.py <input_file>
```

Example:

```bash
python3 physvalPostProcessing.py NTUP_PHYSVAL.44119457._000001.pool.root.1
```

---

### 2. Run with Input and Output File Name:

To specify both the input file and the output file name:

```bash
python3 physvalPostProcessing.py <input_file> <output_file>
```

Example:

```bash
python3 physvalPostProcessing.py NTUP_PHYSVAL.44119457._000001.pool.root.1 output.root
```

---

### 3. Specify a Domain:

You can also specify the domain you want to process. For example, to run for **Electron** domain:

```bash
python3 physvalPostProcessing.py NTUP_PHYSVAL.root output.root --domain Electron
```

---

### 4. Crash on Error for Missing Histograms:

If you want the code to crash when some input histograms are missing, add the `--crash_on_error` flag:

```bash
python3 physvalPostProcessing.py NTUP_PHYSVAL.root --crash_on_error
```

---

## To see all available arguments and options, run the following:

```bash
python3 physvalPostProcessing.py --help
```

---

## Features

### 1. **Rebinning of 1D/2D Histograms**

Useful for reducing the number of bins or adjusting the binning to match specific needs. You can define the input histogram, output histogram path, and the rebinning factor.

Example of rebinning configuration in the config file:

```yaml
rebinning:
  - {input: "Electron/Central/All/KinPlots/Electron_Central_All_KinPlots_et", output: "Electron/Central/All/KinPlots/Electron_Central_All_KinPlots_et", delete_original: False, rebin: 5}
```

### 2. **Efficiency Calculations**

Efficiencies are computed with TEfficiency class of ROOT. To add a new efficiency, use the following configuration format i.e. provide numerator and denominator histogram paths:

```yaml
efficiencies:
  - {numerator: "Electron/Central/Iso/KinPlots/Electron_Central_Iso_KinPlots_eta", denominator: "Electron/Truth/Iso/Electron_Truth_Iso_eta", output: "Electron/Efficiency/Efficiency_reco_only_eta"}
```

### 3. **ROC Curves**

ROC curves can be added by specify the signal and background histograms along with the desired output:

```yaml
roc_curves:
  - {signal: "BTag/AntiKt4EMPFlowJets/BTag_AntiKt4EMPFlowJets_tagger_IP3D_b_matched_weight", background: "BTag/AntiKt4EMPFlowJets/BTag_AntiKt4EMPFlowJets_tagger_IP3D_u_matched_weight", output: "BTag/ROC/IP3D"}
```

### 4. **Adjustments to Histogram Range**

You can adjust the range of specific histograms, fix labels using like:

```yaml
adjustments:
  - {hist: "Photon/Photon_author", final_histo_name: "Photon/Photon_author", x_axis_label: "Photon Author", y_axis_label: "a.u.", xlow: 0, xhigh: 15}
```

### 5. **Projections of 2D Histograms**

The script supports the projection of 2D histograms onto 1D histograms, from a 2D distribution (e.g., `et` vs `eta`). You can also specify custom bin ranges for projections.

```yaml
projections:
  - {hist: "Photon/Phot/All/KinPlots/Photon_Phot_All_KinPlots_etvseta", projection_axis: "y", output: "Photon/Phot/All/KinPlots/Photon_Phot_All_KinPlots_et"}
  - {hist: "Photon/Phot/All/KinPlots/Photon_Phot_All_KinPlots_etvseta", projection_axis: "x", output: "Photon/Phot/All/KinPlots/Photon_Phot_All_KinPlots_eta"}
  - {hist: "Photon/Phot/All/KinPlots/Photon_Phot_All_KinPlots_etvseta", projection_axis: "y", bins_projection_y: [-1.37, 1.37], output: "Photon/Phot/All/KinPlots/Photon_Phot_All_KinPlots_etvseta_barrel"}
```

### 6. **Adding Histograms Together**

You can combine two histograms together by adding them and also can delete the inputs if want.

```yaml
adding_histograms:
  - {histo1_name: "Photon/Phot/All/KinPlots/Photon_Phot_All_KinPlots_et_endcap1", histo2_name: "Photon/Phot/All/KinPlots/Photon_Phot_All_KinPlots_et_endcap2", output: "Photon/Phot/All/KinPlots/Photon_Phot_All_KinPlots_etvseta_endcap", delete_inputs: True}
```

---

In case of any assistance, please contact rasheed.hammad@cern.ch