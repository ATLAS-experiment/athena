# Tools for rates calculations 
Description of the RatesAnalysis package.

July 2025

---

## Contents
1. [RatesAnalysis](#ratesanalysis)  
2. [L1TopoRatesCalculator](#l1toporatescalculator)
  - [Introduction](#introduction)
  - [How to Use L1TopoRatesCalculator](#how-to-use-l1toporatescalculator)
    - [Prerequisites](#prerequisites)  
    - [Input Configuration](#input-configuration)  
    - [Running the Tool](#running-the-tool)  
    - [Outputs](#outputs)  
    - [Post-processing Tools](#post-processing-tools)  
    - [Setup for grid submission](#setup-for-grid-submission)  
  - [Code Structure](#code-structure)  
  - [Internals of HLTRatesCalculator](#internals-of-hltratescalculator)  
    - [Workflow Overview](#workflow-overview)  
    - [Decoding Insights](#decoding-insights)  
        - [L1TopoSimResultsContainer Decoder](#l1toposimresultscontainer_decoder)  
  - [References](#references)  

---
## RatesAnalysis

### Rates analysis script
The script `share/RatesOverlapAnalysis.py` is an all-in-one script to analyse the Trigger Menu. The four functionalities are:
- calculating the overlap of two different streams
- calculating the overlap between a chain and a stream
- calculating the overlap between two chains
- calculating the unique rate of a set of chosen chains

The naming convention of the arguments is similar to other Trigger scripts, but new flags are introduced.

**Stepping through each flag**

These flags are used to identify the input Menu and Prescale files:
- `--HLTMenu`:          needed to select/enable the correct chains based on what streams and chains are being compared
- `--hlt_in`:           the HLT prescale file obtained from the Rulebook with the prescales already made

The following flags are used to identify or name the output prescale files.  These prescale files have a default setting and do not need to be included in the command if the default name is sufficient:
- `--hlt_ps_stream0`:    the prescale file created for the stream0 (default: Main) stream
- `--hlt_ps_stream`:     the prescale file created for the stream being compared to stream0
- `--hlt_ps_chain`:      the prescale file created for the chain selection and comparison to a stream
- `--hlt_ps_chain_ref`:  the prescale file created for the reference chain in the comparison to a chain
- `--hlt_ps_chain_comp`: the prescale file created for the comparison chain in the comparison to a chain
- `--hlt_ps_chain_set`:  the prescale file created for the set of chains chosen when finding the unique rate

The following flags are used in the rates analysis part of the code:
- `--l1_ps`:             L1 prescale file
- `--targetlumi`:        target luminosity
- `--maxEvents`:         max events, take 50000 or more for reasonable statistics
- `--workdir`:           working directory

The following flags are used to select the chains or streams being analysed:
- `--stream0`:        the stream the chain and selected stream are being compared to.  DEFAULT = Main
- `--stream`:         a list of streams that need to be analysed
- `--chain`:          a list of chains that need to be analysed
- `--chain_set`:      a list of chains that will be used in finding the unique rate of the set
- `--chain_comp`:     a list of chains to find the overlap between single chains, chains in the list are compared to the first chain in the list.  

The following flags are used to specify the output file name of the different analyses:
- `--stream_out`:     default is streamRates.txt
- `--chain_out`:       default is chainRates.txt
- `--chain_comp_out`: defualt is chainCompRates.txt

Note that if --chain is not entered, then the script only performs the stream analysis.  The same goes for `--stream` and `--chain_comp`. If the argument _regex_ is passed for `--chain`, `--chain_set` or `--chain_comp`, chains can be searched for by using regular expressions and are then added to the list.  When using regex, the terminal will go into interactive mode, where you then enter your regular expression.  A list of chains in the set is then printed out, and you will need to confirm that you want to proceed with these chains with 'y/n'.

Also note that if the development menu is being used, you need to use AODs that have been reprocessed using the dev menu.

Example commands for each case are given below.

**Comparing two streams**
```
RatesOverlapAnalysis.py /eos/atlas/atlascerngroupdisk/trig-rates/rate_aod/24.0.94_PhysP1_ATR-31329/ --HLTMenu HLTMenu_471.json --hlt_in HLTPrescale_1188.json --l1_ps L1Prescale_14423.json --targetlumi 2e34 --maxEvents 1000 --workdir streamComp --stream0 Main --stream VBFDelayed BphysDelayed TLA
```

**Comparing chains with a stream**
```
RatesOverlapAnalysis.py /eos/atlas/atlascerngroupdisk/trig-rates/rate_aod/24.0.94_PhysP1_ATR-31329/ --HLTMenu HLTMenu_471.json --hlt_in HLTPrescale_1188.json --l1_ps L1Prescale_14423.json --targetlumi 2e34 --maxEvents 1000 --workdir chainStreamComp --stream0 Main --chain HLT_6j35c_020jvt_pf_ftf_presel6c25_L14jJ40 HLT_j460_pf_ftf_preselj225_L1jJ160 HLT_j480_pf_ftf_preselj225_L1jJ160 HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160
```

```
RatesOverlapAnalysis.py /eos/atlas/atlascerngroupdisk/trig-rates/rate_aod/24.0.94_PhysP1_ATR-31329/ --HLTMenu HLTMenu_471.json --hlt_in HLTPrescale_1188.json --l1_ps L1Prescale_14423.json --targetlumi 2e34 --maxEvents 1000 --workdir chainStreamComp --stream0 Main --chain regex
```

Note that in this case, the chains are being compared to **stream0**, which is set to Main as a default. The chains need to be in the Main stream to get the overlap.

**Comparing two chains**
```
RatesOverlapAnalysis.py /eos/atlas/atlascerngroupdisk/trig-rates/rate_aod/24.0.94_PhysP1_ATR-31329/ --HLTMenu HLTMenu_471.json --hlt_in HLTPrescale_1188.json --l1_ps L1Prescale_14423.json --targetlumi 2e34 --maxEvents 50000 --workdir chainsComp --chain_comp HLT_j460_pf_ftf_preselj225_L1jJ160 HLT_j480_pf_ftf_preselj225_L1jJ160 HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160 
```

```
RatesOverlapAnalysis.py /eos/atlas/atlascerngroupdisk/trig-rates/rate_aod/24.0.94_PhysP1_ATR-31329/ --HLTMenu HLTMenu_471.json --hlt_in HLTPrescale_1188.json --l1_ps L1Prescale_14423.json --targetlumi 2e34 --maxEvents 500000 --workdir chainsComp --chain_comp regex
```

**Finding the unique rate of a set of chains**
```
RatesOverlapAnalysis.py /eos/atlas/atlascerngroupdisk/trig-rates/rate_aod/24.0.94_PhysP1_ATR-31329/ --HLTMenu HLTMenu_471.json --hlt_in HLTPrescale_1188.json --l1_ps L1Prescale_14423.json --targetlumi 2e34 --maxEvents 1000 --workdir chainSet --chain_set HLT_j460_pf_ftf_preselj225_L1jJ160 HLT_j480_pf_ftf_preselj225_L1jJ160 HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160 
```

```
RatesOverlapAnalysis.py /eos/atlas/atlascerngroupdisk/trig-rates/rate_aod/24.0.94_PhysP1_ATR-31329/ --HLTMenu HLTMenu_471.json --hlt_in HLTPrescale_1188.json --l1_ps L1Prescale_14423.json --targetlumi 2e34 --workdir chainSet --chain_set regex
``` 

### L1TopoRatesCalculator

#### Introduction

The **L1TopoRatesCalculator** is a software tool designed to analyze and optimize trigger rates for the ATLAS experiment at CERN. Triggers are essential in high-energy physics experiments, as they filter and select interesting events from millions of collisions occurring every second. Specifically, this tool works with Level-1 Topological (L1Topo) triggers, which are responsible for making fast decisions based on the spatial and kinematic relationships between detected particles.

The primary goal of the L1TopoRatesCalculator is to evaluate the rates and correlations of complex triggers defined by logical operations (e.g., AND, OR) between multiplicity/Topo triggers. These calculations provide insights into; **trigger efficiency**, how often a trigger activates.
**Redundancy**, overlaps between triggers that might indicate inefficiencies and
**optimization**, suggestions for merging or redefining triggers to improve the trigger menu for future runs.

This document has been created to provide a comprehensive explanation of the functionality and purpose of the L1TopoRatesCalculator tool. It is intended as a reference for future users working on the ATLAS experiment, to understand how the tool processes user-defined triggers, calculates rates, and generates correlation matrices. 

#### How to Use L1TopoRatesCalculator

##### Prerequisites

- Required dependencies: Compatible version of Athena, e.g.  
  - `Athena release = "25.0.22, " "Athena, main, r20"`
  - `Nightly = "r20"`

#### Input Configuration

To execute the L1TopoRatesCalculator, the user must provide a triggers.json file containing the names or definitions of the triggers. The JSON file should include two sections: L1_items and userDefinedL1items.

- **JSON File:** `triggers.json` containing:
  - **L1 Items:** This section lists the names of the triggers for which the user wants to calculate rates. These triggers must already be defined in the L1Menu.
  - **userDefinedL1items:** This section specifies new trigger combinations defined by the user. For each trigger in this section, the user must include its definition. This definition should contain the name of the topo/multiplicity trigger, the number of each type of trigger, and the operations that link them following this structure (same one as in the L1Menu):
    ```
    "definition": "trigger1[x1] & trigger2[x3] | trigger3[x1]"
    ```

- **RDOs:** In addition to the JSON file, the user has to provide the MC or data files to run over. These must be RDOs (Simulated detector output), that contain pre-reco information.

#### Running the Tool

The L1TopoRatesCalculator is executed through a Python script, /athena/Trigger/TrigCost/RatesAnalysis/python/runL1TopoRates.py, which is configured to run in the Athena framework. This script is the entry point for configuring and running the HLT (High-Level Trigger) with custom trigger definitions provided in a JSON format.

Command to execute:
```bash
python /athena/Trigger/TrigCost/RatesAnalysis/python/runL1TopoRates.py 
  --ratesJson=triggers.json \
  --filesInput=/RDO inputFile
```
Key parameters:

- **ratesJson**: Input JSON file with user-defined triggers.
- Optional: configuration for luminosity, output options, etc.

#### Outputs

The script outputs a ROOT file (**RatesHistograms.root**) containing histograms of the calculated trigger rates and a summary of the calculations. This includes the **rates_matrix** TH2D histogram containing all correlations of the User-defined triggers.

#### Post-processing tools

To provide additional flexibility and avoid rerunning the L1TopoRatesCalculator, a Python script has been developed L1TopoRatesCalculator_submatrix_plotter.

Users can specify a subset of triggers, avoiding the need to analyze the entire correlation matrix. The script generates a plot of the submatrix, with a heatmap to represent correlation values, numerical values (with errors) displayed directly in the cells and labels for the selected triggers on the axes.

The user must provide the output root file obtained after running runL1TopoRates.py which contains the correlation matrix (RatesHistograms.root), called "rates_matrix", the set of triggers the user wants to plot and the output file name. An example of the execution of the plotter is:

- **Execution:**
```bash
python L1TopoRatesCalculator_submatrix_plotter.py run/RatesHistograms.root rates_matrix L1_eTAU12 L1_jTAU20 L1_gXEJWOJ70 L1_JPSI-1M5-eEM15 L1_jJ40p30ETA49 correlation_matrix.png
```
See Section Directory Layout to find the path of the L1TopoRatesCalculator_submatrix_plotter. An example of a submatrix produced by the L1TopoRatesCalculator_submatrix_plotter is shown in Figure 2.3.

#### Setup for grid submission

If you have already setup your environment, just setup panda and then run the submission script (currently at):

```bash
Trigger/TrigCost/RatesAnalysis/share/submit_to_grid_L1TopoRates.sh
```

After submitting the jobs, you can download the following files:

```bash
user.$user.$.l1topoRates.byFile.v2_RatesHistograms_XYZ.root.tgz
```
For histograms containing the matrices:
and for logs:
```bash
user.$user.$.l1topoRates.byFile.v2_logs_XYZ.root.tgz
```
To extract them:
```bash
for t in *.tgz; do tar -xzf "$t"; done
```
After extraction, process the files to get the final combined matrices.
Run the following ROOT script:

```bash
root -l -b -q 'Trigger/TrigCost/RatesAnalysis/share/combine_L1TopoRates_grid_matrices.cxx'
```
where you will have to set the inputDir and might need to change the pattern of your RatesHistograms files, the default is "RatesHistograms_data24_13p6TeV.".
The output will be a single file "CombinedMatrices.root" containing the combination of the matrices: counts_matrix_combined, rates_matrix_combined and L1TopoScore_matrix_combined.
Finally, you can run:

```bash
python Trigger/TrigCost/RatesAnalysis/share/L1TopoRatesCalculator_submatrix_plotter.py CombinedMatrices.root L1_10DR-MU14FCH-MU5VF L1_13DR25-25M70-2eEM12L
```
to plot the matrices and triggers you decide.
#### Code structure

The L1TopoRatesCalculator source code is organized in the following directory structure.

Directory Layout:

```bash
/athena/Trigger/TrigCost/RatesAnalysis/src/
    L1TopoRatesCalculator.cxx          # Main source file where the core logic resides
    L1TopoRatesCalculator.h            # Header file for the L1TopoRatesCalculator class
    RatesAnalysisAlg.cxx            # Handles the creation of the rates matrix 

/athena/Trigger/TrigCost/RatesAnalysis/share/
    L1TopoRatesCalculator.py
    L1TopoRatesCalculator_submatrix_plotter.py # Post-processing tool to plot submatrices
    runL1TopoRates.py # Execution of L1TopoRatesCalculator
    triggers.json # Example of the input json file
    combine_L1TopoRates_grid_matrices.cxx # Combines the output matrices from the grid submission and creates the final matrices
    submit_to_grid_L1TopoRates.sh # Grid submission script
```
#### Internals of L1TopoRatesCalculator

The L1TopoRatesCalculator is designed to calculate trigger rates and provide insights into the correlation between different Level-1 (L1) trigger items in the ATLAS experiment. Below is a detailed explanation of its internal structure, key components, and how the various processes are connected.

##### Workflow Overview

1. **Initialization**
  - Input Configuration: The tool first loads the user-defined triggers from the provided JSON input file. It then matches these user-defined triggers with the L1 trigger menu and parses them into individual components, including Topo/multiplicity triggers and the logical operations between them (e.g., AND, OR). These are saved in the map m_triggerMap.

2. **Decision Decoding**

  - The core of the calculation process lies in decoding the decisions for each Topo/multiplicity trigger. This is done by extracting the decision data from the L1TopoSimResultsContainer, which contains the results of the L1 trigger evaluation in the simulation. See section **Decoding insights**.
  - Each trigger’s decision is evaluated based on its definition (multiplicities, logical operations).

3. **Rate Calculation**

  - Once all the trigger decisions have been decoded, the tool computes the rates of the user-defined triggers. This step involves calculating how often a trigger fires over a given period (luminosity) and then updating the rates matrix to reflect the correlations between different triggers.

  - The rates formula is [1]:

  $$
R = \sum^N_{e=1}\frac{w(e)}{\Delta t} \quad , \quad R_{err} = \frac{\sqrt{\sum^N_{e=1}w(e)^2}}{\Delta t} \quad , \quad w(e) = w_{EB}(e)\cdot w_C(e)\cdot w_L(e)
$$

Where $R$ is the rate in Hz and $R_{err}$ its statistical error, the sum runs over all $e$ = 1, 2, . . . , $N$ events in the enhanced bias dataset, the weight $w(e)$ is the effective number of events passed by the chain/combination in event $e$ and $\Delta t$ is the time period over which the enhanced bias data sample was collected, typically around one hour. The obtained rate corresponds to the average instantaneous luminosity over this time period.

Here for event $e$, $w_{EB}(e)$ is the enhanced bias weight, $w_C(e)$ is the chain/combination weight and $w_L(e)$ is a luminosity extrapolation weight. In the L1TopoRatesCalculator this calculations can be found at L1TopoRatesCalculator.cxx.

4. **Finalization**

- After the rate calculations are completed, the rates matrix is normalized and stored. The final matrix, along with any associated statistics (e.g., errors) (m_ratesMatrixHist), is saved for later analysis or visualization.

##### Decoding insights

Two main functions are responsible for extracting the decision from the L1TopoSimResultsContainer; L1TopoSimResultsContainer_decoder and extractResult. 

**L1TopoSimResultsContainer_decoder** is the high-level function responsible for evaluating whether a multiplicity/Topo trigger fires. It processes the simulation results and uses extractResult as a helper function to decode specific trigger results from the raw data stream.

**extractResult** focuses on the low-level details of bit extraction, isolating the relevant trigger data from a large data block.

##### L1TopoSimResultsContainer_decoder

The L1TopoSimResultsContainer_decoder function is the core of the trigger decision decoding process. It uses the extractResult function to evaluate whether a specific trigger has fired based on its definition and the data stored in the L1TopoSimResultsContainer.

- **Inputs**:

  - definition: The trigger's definition object, which specifies:
    - flatindex: Starting bit position.
    - nBit: Bit width.
    - conID: Connector ID.
    - Other parameters such asfromSim (simulation mode).

  - cont: A handle to the L1TopoSimResultsContainer, which contains the results of the L1Topo simulation for all triggers.

- **Process**:
  1. Loops through the results in the L1TopoSimResultsContainer. Filters results based on the connectionId (conID) to ensure only the relevant connector’s data is processed.
  2. Handles electrical connectors (32-bit results) and optical connectors (64-bit results) differently, accounting for overflow bits when necessary. Reads the data into a vector of connector contents (connectorWords).
  3. For each valid result, calls extractResult to decode the trigger's specific bits based on its definition.

    - **extractResult**: responsible for extracting a specific trigger result from the L1Topo simulation data, based on the trigger's position (index) and size (bit width) in the data stream.
        - Inputs:
          - connectorContents: A vector of 32-bit words representing the data read from the L1Topo hardware for a specific trigger. This data includes the trigger's decision bits across multiple words.
          - definition}: An object containing the trigger's definition, including the flatindex (the starting position of the trigger's bits in the data stream) and nBit (the number of bits representing the trigger).
          - startOffset: An offset used to adjust the starting position of the trigger's data based on its connector type (e.g., optical or electrical).
        - Process:
          - Determine Start and End Positions: Computes the exact range of bits (startindex to endindex) for the trigger based on its flatindex and nBit. Determines which 32-bit words (firstWord and lastWord) in the connectorContents vector contain the trigger's data.
          - Bit Extraction: Iterates over the relevant words in connectorContents to extract the trigger's bits. Applies bit masking and shifts to isolate the specific bits corresponding to the trigger.
          - Result Assembly: Combines the extracted bits into a single result using bitwise operations. Handles cases where the trigger spans multiple words.
          - Return Value: Returns the extracted trigger result as a 32-bit unsigned integer. If multiple results (e.g. from different clock cycles) exist, it combines them using a logical OR.

  4.  If results are available from multiple clock cycles, combines them into a final decision using logical operations. Returns the decoded trigger decision (resultValue), representing whether the trigger condition was satisfied.

### References
[1] The ATLAS Collaboration,
_Trigger monitoring and rate predictions using Enhanced Bias data
from the ATLAS Detector at the LHC_
,
ATL-DAQ-PUB-2016-002.
