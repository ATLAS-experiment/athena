# Expected Usage

```python
>>> import ColumnarToolWrapperPython
>>> muon_eff_sf_tool_handle = ColumnarToolWrapperPython.PythonToolHandle()
>>> muon_eff_sf_tool_handle.set_type_and_name("CP::MuonEfficiencyScaleFactors/unique0")
>>> muon_eff_sf_tool_handle.initialize()
ToolSvc.unique0          INFO    Efficiency type is = RECO
ToolSvc.unique0          INFO    JPsi based low pt SF will start to rock below 10 GeV!
ToolSvc.unique0          INFO    Trying to initialize, with working point Medium, using calibration release 251211_Preliminary_r24run3
ToolSvc.unique0          INFO    Successfully initialized!
```
