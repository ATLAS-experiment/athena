# TrigTauHypo package

This package contains the HLT Hypothesis algorithms and tools required for all Tau triggers.

As per the usual Athena conventions, the Hypothesis algorithms are only responsible of building the output trigger Decision for each chain, and for preparing the input data required by the HypoTools. It's the HypoTools' task to evaluate the actual Decisions based on their configuration. Each chain will have a distinctive HypoTool for each step, configured with the appropriate selection by the functions in [`TrigTauHypoTool.py`](python/TrigTauHypoTool.py).

## Hypothesis algorithms and tools

The following Hypothesis algorithms and associated tools are available:

<table>
    <thead>
        <tr>
            <th>Step(s)</th>
            <th>HypoAlg</th>
            <th>HypoTool(s)</th>
            <th>Cuts</th>
            <th>Online monitoring</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td><code>CaloMVA</code></td>
            <td><code>TrigTauJetHypoAlg</code></td>
            <td><code>TrigTauCaloHypoTool</code></td>
            <td><ul>
                <li><code>TauJet</code> p<sub>T</sub></li>
            </ul></td>
            <td>:white_large_square:</td>
        </tr>
        <tr>
            <td><code>FTFCore</code>/<code>Iso</code>/<code>LRT</code></td>
            <td><code>TrigTauTrackingHypoAlg</code></td>
            <td><code>TrigTauTrackingHypoTool</code></td>
            <td>-</td>
            <td>:white_large_square:</td>
        </tr>
        <tr>
            <td><code>PrecTrackIso</code>/<code>LRT</code></td>
            <td><code>TrigTauTrackingHypoAlg</code></td>
            <td><code>TrigTauTrackingHypoTool</code></td>
            <td>-</td>
            <td>:white_large_square:</td>
        </tr>
        <tr>
            <td rowspan="2"><code>Precision</code>/<code>LRT</code></td>
            <td rowspan="2"><code>TrigTauJetHypoAlg</code></td>
            <td><code>TrigTauPrecisionIDHypoTool</code></td>
            <td><ul>
                <li><code>TauJet</code> p<sub>T</sub></li>
                <li>Number of <code>TauJet</code> Core/Isolation tracks (with <code>perf</code> p<sub>T</sub> preselection)</li>
                <li><code>TauJet</code> TauID score</li>
            </ul></td>
            <td>:white_check_mark:</td>
        </tr>
        <tr>
            <td><code>TrigTauPrecisionDiKaonHypoTool</code></td>
            <td><ul>
                <li><code>TauJet</code> p<sub>T</sub></li>
                <li>Number of <code>TauJet</code> Core/Isolation tracks (with <code>perf</code> p<sub>T</sub> preselection)</li>
                <li><code>TauJet</code> kinematic quantities</li>
                <li>Invariant mass of systems of particles built from different combinations of the <code>TauJet</code>'s Core tracks, under multiple tau-decay mass hypothesis.</li>
            </ul></td>
            <td>:white_check_mark:</td>
        </tr>
    </tbody>
</table>

The monitored histograms of `TrigTauPrecisionIDHypoTool`s and `TrigTauPrecisionDiKaonHypoTool`s are defined in [`TrigTauHypoMonitoring.py`](python/TrigTauHypoMonitoring.py).

The `TrigTauPrecisionDiKaonHypoTool` is only used for B-Physics combined chains.
