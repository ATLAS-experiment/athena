To run the standard global simulation, export a valid configuration xml and use `l1calo-ath-mon` with the `Trigger.L1.dogFex`, `Trigger.L1.doeFex` options enabled:

```bash
export GS_CFG_FILE=PATH-TO-SOURCE/athena/Trigger/TrigT1/Global/GlobalSimulation/share/globalSim_AllChainsCfg.xml 
l1calo-ath-mon --postInclude GlobalSimulation/plugin_local_cfg.py --filesInput RAW_RUN3_DATA24 --evtMax 10 -- IOVDb.GlobalTag='CONDBR2-BLKPA-2024-05' Trigger.L1.dogFex=True Trigger.L1.doeFex=True
```

Please see `l1calo-ath-mon --help` for more options and instructions.
If you want to run different chains, then you can create and export your own globalSim.xml

### Creating a new GlobalSim Algorithm
#### Source Folder
To add a new [`AlgoComputeCell`](https://gitlab.cern.ch/atlas-tdaq-p2-firmware/global-trigger/gep-fw/-/tree/develop/Sources/AlgoComputeCells?ref_type=heads) into GlobalSimulation, first check if the equivalent source folder exists in [GlobalSimulation](https://gitlab.cern.ch/atlas/athena/-/tree/main/Trigger/TrigT1/Global/GlobalSimulation/src?ref_type=heads). If it doesn't exist, then create a folder in GlobalSimulation/src with the same name as the gep-fw `AlgoComputeCell` you intend to simulate. If you are creating a hypothesis algorithm the [Hypothesis](https://gitlab.cern.ch/atlas/athena/-/tree/main/Trigger/TrigT1/Global/GlobalSimulation/src/Hypothesis?ref_type=heads) folder is the correct location, and  please refer to the [Hypothesis TIP Writers](https://gitlab.cern.ch/atlas/athena/-/blob/main/Trigger/TrigT1/Global/GlobalSimulation/src/Hypothesis/README.md?ref_type=heads) section. If you are unsure what your algorithm should be called, or where it should be placed, please contact the developers.

If you create a new folder, you will need to add this to the `athena_add_component` section of the [`CMakeLists.txt`](./CMakeLists.txt), and any new AlgTool's header needs to be added to the [`GlobalSimulation_entries`](./src/components/GlobalSimulation_entries.cxx) file.

#### TOB Writer AlgTool Creation 
Global algorithms representing `AlgoComputeCells` that manipulate and enhance data flowing through a GEP board are known as TOB Writers; they read and write Trigger Objects (TOBs) from `StoreGate`. The functionality of each individual TOB Writer `AlgoComputeCell` is implemented in [GlobalSimulation](https://gitlab.cern.ch/atlas/athena/-/blob/main/Trigger/TrigT1/Global/GlobalSimulation/src/GlobalSimComponents/README.md?ref_type=heads) by an Athena `AlgTool`, executed by an instance of `GlobalSimulationAlg`, and must extend the `IGlobalSimAlgTool` interface.

#### Implementing a TOB Writer
For an example implementation, see [`GlobalJet1AlgTool`](./src/Jet1/GlobalJet1AlgTool.h).

TOB writers derive from the `AthAlgTool` base class, which handles the I/O for each new event, and the `IGlobalSimAlgTool` interface which provides additional data collection. I.e. this example looks like,

```c++
  //Derive from the relevant base class and interface.
  class GlobalJet1AlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {
    ...
  }
```

In the `initialise()` method you must initialise all of your input and output container key.s

```c++
  //Initialize functions input and output before the first event
  StatusCode GlobalJet1AlgTool::initialize() {

    CHECK(m_gblCellTowers.initialize());
    CHECK(m_gblJet1JetsContainerKey.initialize());
    ...//Any additional I/O
     
    return StatusCode::SUCCESS;
  }
```

In the `run()` method you include any code that you want to run for each event to be simulated. You need to pass this method a pointer to an `IDataCollector`, though this can be a null pointer, and start and end it.

```c++
  // Main functional block running for each event
  StatusCode GlobalJet1AlgTool::run(const std::unique_ptr<IDataCollector>& dc,
				    const EventContext& ctx) const {

    ATH_MSG_DEBUG("Building WTAConeJets");
    if (dc){dc->collect(*this, "start");}

    //Main body of the code to be run per event goes here
   
    if (dc){dc->collect(*this, "end");}

    return StatusCode::SUCCESS;
  }
```

For GlobalSimulation debug tools, the AlgTool can write output messages to the DataCollector, first checking that the pointer is not a `nullptr`

```c++
  if (dc) {
    std::stringstream ss;
    ss << "Message goes here.";
    dc->collect((*this, ss.str());
   }
```

You can call other classes or functions from this AlgTool `run()` method. For this example the JET1 algorithmic code has been defined in the [`WTACone2PassMaker`](../../TrigGepPerf/src/WTACone2PassMaker.h) class external to the `GlobalSimulation` package. In other examples, such as the [`eGamma1BDT`](./src/Egamma1BDT/Egamma1BDTAlgTool.h), calls high-level sysnthesis (HLS) code produced during the production of the VHDL being loaded into the firmware.

`StoreGate` entries are accessed via the configurable Read/WriteHandleKeys defined in the header, which can be altered by the configuration,

```c++
  /** @brief Read key for the output cell towers as a GenericTobContainer */	
  SG::ReadHandleKey<IOBitwise::CommonTOBContainer>
  m_gblCellTowers {
      this,
      "GlobalCellTowersKey", //Key
      "GlobalCellTowers",  //Name
      "Key to the container of generic TOBS containing the cell towers"}; //Comment
```

In the body you can access any `StoreGate` entries via the defined Read/WriteHandleKeys, passing the key and the event context.

```c++
  // Read the GlobalCellTowers
  auto h_towerTOBs = SG::makeHandle(m_gblCellTowers, ctx);
  CHECK(h_towerTOBs.isValid());

  // Process the input

  auto h_Jet1TOBs = SG::makeHandle(m_gblJet1JetsContainerKey, ctx);
  auto jets = std::make_unique<IOBitwise::CommonTOBContainer>();

  // Fill the output container

  // Move the output container into SG
  CHECK(h_Jet1TOBs.record(std::move(jets))); 
```

If you need to define a new TOB which does not yet exist in the [IO](./src/IO/) folder, please contact the GlobalSimualtion developers.

#### Adding a new GlobalSim AlgTool to a job
To add a new algorithm to a GlobalSimulation job, you need to add it to a `GlobalSimAlgConfig` XML file. An example can be seen in the [globalSim_AllChainsCfg.xml](./share/globalSim_AllChainsCfg.xml).

To add a TOB writer to the config, you need to add a new line to the config in the TOBWriters section. If the `AlgTool` depends upon a previous GlobalSimulation `AlgTool`, then you need to add a `child class` line.

```xml
  <TOBWriters>
    <AlgTool class="GlobalJet1AlgTool" name="GlobalJet1AlgTool">
      <child class="GlobalCellTowerAlgTool" name="GlobalCellTowerAlgTool" slot="in0"/>
    </AlgTool>
  </TOBWriters>
```

To add a TIP writer, then you need to add to the TIPWriters section, where you can configure the cut values, TIP width and position.

```xml
  <AlgTool class="eEmEg1BDTMultAlgTool" name="eEmEg1BDTMultAlgTool_0">
    <property name="Eg1BDT" value='128'/>
    <property name="Eg1BDT_op" value="&gt;="/>
    <property name="TIPposition" value="3" type="int" />
    <property name="TIPwidth" value="3" type="int" />
    <child class="Egamma1BDTAlgTool" name="Egamma1BDTAlgTool" slot="in0"/>
  </AlgTool>
```

You can run this configuration in your job by exporting this XML to `GS_CFG_FILE`, where it will be picked up by the `l1calo-ath-mon` script.

### Enabling debugging output on components

If you want `DEBUG` level output on one of your components, simply add to your command:

```bash
cfg.<AlgorithmTypeOrName>.OutputLevel=2
```

e.g. to turn on debugging on an algorithm called `GlobalSimTestAlg` do:

```bash
l1calo-ath-mon .... -- cfg.GlobalSimTestAlg.OutputLevel=2
```

### Enabling storegate dumping

To turn on StoreGate dumping, do:

```bash
l1calo-ath-mon .... -- cfg.StoreGateSvc.Dump=True
```

### Including the GraphSvc in your job

If you want to generate a graph for your job, you can include the `GraphSvc` by creating another plugin file e.g. in your local dir with the following content:

```python
cfg.addService( CompFactory.GlobalSim.GraphSvc(), create=True )
```
then just add that to your `--postIncludes` argument.
