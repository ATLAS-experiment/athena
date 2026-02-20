# AthExHistNtup

`AthExHistNtup` demonstrates basic ROOT I/O features available in the Athena framework through the `THistSvc` service: 

* Writing a simple TH1F histogram that was filled for every Event in the EventLoop

* Writing a small Tuple of values, collected for every Event - using ROOT TTree/RBranch storage technology

It can serve as an example of how to write out simple data from Athena jobs, that is not part of the official ATLAS EDM and does not require the use of the full-fledged Athena persistency mechanism.

The job requires no input and produces two output files: `hist.root` and `ntuple.root`. A reading python script is provided to inspect the created files: `read_hist_ntuple.py`.


## Running the example

To run the example you should first set up the ATLAS software and then run the configuration file:

```
asetup Athena,main,latest
python -m AthExHistNtup.HistNtupConfig
```

This will run the job according to [this configuration file](https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/AthenaExamples/AthExHistNtup/python/HistNtupConfig.py) and will create the Tuple and Histogram ROOT files.
Next, you can run the reading script to inspect the content of the NTuple (For the Histogram only the number of entries is printed)

```
python -m AthExHistNtup/read_hist_ntuple
```


## Package description

The package contains two Athena algorithms called `Hist` and `Ntup`. They are defined in the C++ part of the package. They are also declared as Gaudi Components (in the "src/components" directory), so they framework can dynamically create them. Both algorithms use `THistSvc` to write some simple data to their ROOT output file. The collected data is derived from the EventInfo object that the EventLoop supplies to the job (the EventInfo is fabricated, as there is no actual input for this job). The EventLoop actually provides McEventInfo object, which needs to be converted into xAOD::EventInfo that our algorithms can consume by the `EventInfoCnvAlgCfg` algorithm (which is added to the job by the configuration script). 

The output configuration of `THistSvc` is done in the configuration script `HistNtupConfig`. Each algorithm has a configuration function where it retrieves the service and specifies the output file details, together with output alias that the C++ code uses to connect to the right file. `EventInfoCnvAlgCfg` configuration is also done in these functions. The functions are both fully self-sufficients and independent, so both algorithms can be run independently as well.

The [read_hist_ntuple.py](https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/AthenaExamples/AthExHistNtup/python/read_hist_ntuple.py) reading script can be used to inspect the created output files. It accesses ROOT files directly, without relying on any Athena functionality.
