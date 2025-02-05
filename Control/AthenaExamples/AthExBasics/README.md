# AthExBasics

This package provides a number of minimalist Athena algorithms and tools demonstrating how to carry out basic tasks in Athena. The package can be used as a general learning resource, as a source of examples of how to do specific tasks, and (in common with the other AthenaExamples packages) as a template for creating new packages. 

## ReadTriggerDecision

This is a simple Athena algorithm demonstrating how to the read the trigger decision from AOD or DAOD files using the trigger decision tool, which is fully documented [here](https://twiki.cern.ch/twiki/bin/viewauth/Atlas/TrigDecisionTool). The output, printed once at the end of the job in the `finalise` method, is a list of all triggers that fired at least once in the range of events processed, and the number of events for which each trigger fired. It consists of:
 * ```ReadTriggerDecision.cxx,.h```: the Athena algorithm itself and the header file. Actions using the trigger decision tool (in this case getting a list of high level triggers and checking whether each event passed each of them) are done here.
 * ```ReadTriggerDecisionConfig.py```: the configuration methods, which configure the reading of the input file,  sets up the trigger decision tool and the algorithm, and governs the passing of the former to the latter. The input file can be set here, or passed via the command line

Usage with the input file specified in the python configuration:

 ```python -m  AthExBasics.ReadTriggerDecisionConfig```   

Usage with the input file specified at the command line:

```python -m  AthExBasics.ReadTriggerDecisionConfig --filesInput=input.pool.root```

## RunTriggerMatching

RunTriggerMatching extends the ReadTriggerDecision example above, using the trigger decision and trigger matching tools to obtain lists of triggers matching the user-provided string (`HLT_mu.*` by default) and then match them objects in a user-specified reconstruction object container (`Muons` by default). The end result is a list of triggers, with a count of the number of offline objects that were matched to each. Similarly to the above example the package consists of a single Athena algorithm defined in a header and source file, and a python configuration file. 

Usage with the input file specified in the python configuration:

 ```python -m  AthExBasics.RunTriggerMatchingConfig```   

Usage with the input file specified at the command line:

```python -m  AthExBasics.RunTriggerMatchingConfig --filesInput=input.pool.root```

## ReadxAOD

ReadxAOD demonstrates the reading of an offline object container (inner detector tracks) and the application of a tool to the container (the track selection tool). The structure is the same as the two examples above, with a single algorithm and one config file. In this example the track selection tool is set up in a default mode, which is defined in another configuration file provided by the inner detector group. The output is a count of how many tracks fall into different bins in pT, with the user able to specify the bin edges.

Usage with the input file specified in the python configuration:

 ```python -m  AthExBasics.ReadxAODConfig```   

Usage with the input file specified at the command line:

```python -m  AthExBasics.ReadxAODConfig --filesInput=input.pool.root```

## WritexAOD

WritexAOD is similar to ReadxAOD in that it reads an xAOD and selects a subset of tracks using a tool, but it then demonstrates how to write that subset into a new container. In this way it demonstrates how to create a new xAOD container and enter objects on the C++ side, and also how to configure the output stream to write a file containing a new xAOD collection. 

Usage with the input file specified in the python configuration:

 ```python -m  AthExBasics.WritexAODConfig```   

Usage with the input file specified at the command line:

```python -m  AthExBasics.WritexAODConfig --filesInput=input.pool.root```
