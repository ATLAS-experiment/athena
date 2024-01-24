# Usage
See the instructions of the [TrigGlobalEfficiencyCorrectionTool](https://twiki.cern.ch/twiki/bin/viewauth/Atlas/TrigGlobalEfficiencyCorrectionTool) twiki page (ATLAS internal). 

Note that the tool interface header `ITrigGlobalEfficiencyCorrectionTool.h` is hosted in another package, `PhysicsAnalysis/Interfaces/TriggerAnalysisInterfaces`, so usually in order to use this tool you only need to link to the `TriggerAnalysisInterfaces` library in your CMakeLists.txt file. 



# Examples of configuration of the tool

Several examples are provided, corresponding to the source files `util/TrigGlobEffCorrExample*.cxx` (dual-use (Ath)AnalysisBase executables). To run them:
```
TrigGlobEffCorrExample0 [--debug] <input DxAOD file>.root
```

Illustrated features:
- Example 0: minimal configuration
- Example 1: singe+dilepton triggers combination
- Example 2: [removed]
- Examples 3a-3e: usage of lepton selection tags
- Example 4: usage of the static helper method suggestElectronMapKeys()
- Example 5a: photon triggers, simplest example (symmetric diphoton trigger)
- Example 5b: photon triggers, more complex (asymmetric diphoton trigger)
- Example 06: trigger matching
More details can be found in the comments of each source file. 


# Formulas used to calculate the combined efficiencies

The following references the formulas that are hardcoded in the tool; most were verified by comparing against the alternative pseudoexperiments-based calculation that the tool can be configured to provide instead --- see `util/TrigGlobEffCorrValidation.cxx`. 

## Definitions

Making generous use of the formulas to expand probabilities of unions or intersections of $n$ events 
(RHS sums have $2^n-1$ terms in total):
$$
\mathbb{P}(A_1\vee \ldots \vee A_n)  = \mathbb{P}(A_1) + \ldots - \mathbb{P}(A_1\wedge A_2) - \ldots + (-1)^{n+1} \mathbb{P}(A_1\wedge\ldots\wedge A_n)\\
\mathbb{P}(A_1\wedge \ldots \wedge A_n)  = \mathbb{P}(A_1) + \ldots - \mathbb{P}(A_1\vee A_2) - \ldots + (-1)^{n+1} \mathbb{P}(A_1\vee\ldots\vee A_n)
$$


## Single lepton triggers
### One $e/\mu$ trigger, $n$ leptons
$$
\mathbb{P}(T_n) =\mathbb{P}(\ell_1\vee\ldots\vee\ell_n)\\
= 1 - \mathbb{P}(\bar\ell_1\wedge\ldots\wedge\bar\ell_n)\\
= 1- \prod_{1\leq k\leq n} (1-\mathbb{P}(\ell_k))\qquad{(1)}
$$

This is the formula implemented in the muon trigger scale factor tool. 

### One $e$ + one $\mu$ trigger, $n_e+n_\mu$ leptons

$$
\mathbb{P}(T_{n_e+n_\mu})=\mathbb{P}(E_{n_e}\vee M_{n_\mu})\\
=1 - \mathbb{P}(\bar E_{n_e})\mathbb{P}(\bar M_{n_\mu})\\
= 1- \prod_{1\leq k\leq n_e} (1-\mathbb{P}(e_k))\prod_{1\leq k\leq n_\mu} (1-\mathbb{P}(\mu_k))\qquad{(2)}
$$

This is the same formula as the previous case. Note however that the total efficiency can't be factorized into separate contributions for electrons and muons (because of this leading $1 -$ term breaking linearity), so the scale factor has to be evaluated at once considering both lepton flavours. 

### Several $e/\mu$ triggers, $n$ leptons

If there are $k$ triggers in the combination: 

$$
\mathbb{P}(T_n)=\mathbb{P}(T^{1}_n\vee\ldots\vee T^{k}_n) \\
= 1 - \mathbb{P}(\bar T^1_n\wedge\ldots\wedge\bar T_n^k) \\
= \mathbb{P}(\bar \ell_1^1\vee\ldots\bar\ell_1^k)\times\ldots\times\mathbb{P}(\bar \ell_n^1\vee\ldots\bar\ell_n^k)\\
= 1- \prod_{1\leq j\leq n} (1-\mathbb{P}(\ell_j^{\lambda_j}))\qquad{(3)}
$$

This is, conveniently, the same expression as for a single trigger (1), except that for each lepton 
one needs to consider the probability of triggering the loosest trigger leg according to the established hierarchy, 
The index of this loosest leg is indicated by $\lambda_j\in [0,k-1]$. 
As indicated by the subscript $j$, this index may vary from one lepton 
to the other since a given lepton might not be on plateau for all $k$ legs; 
also, the hierarchy itself is sometimes $p_\mathrm{T}$-dependent. 

### Several $e$ + $\mu$ triggers, $n_e+n_\mu$ leptons

Straightforward combination of (2) and (3):
$$
\mathbb{P}(T_n)=
1- \prod_{1\leq j\leq n_e} (1-\mathbb{P}(e_j^{\lambda_j}))\prod_{1\leq j\leq n_\mu} (1-\mathbb{P}(\mu_j^{\lambda_j}))\qquad{(4)}
$$

## Dilepton triggers, simple cases

### One $e\mu$ trigger, $n_e+n_\mu$ leptons

Need to fire both the electron and the muon legs, things factorize nicely: 
$$
\mathbb{P}(T_{n_e+n_\mu})=\mathbb{P}(E_{n_e}\wedge M_{n_\mu}) \\
= \mathbb{P}(E_{n_e})\times\mathbb{P}(M_{n_\mu})\\
=\left(1- \prod_{1\leq k\leq n_e} (1-\mathbb{P}(e_k))\right)\times\left(1- \prod_{1\leq k\leq n_\mu} (1-\mathbb{P}(\mu_k))\right)\qquad{(5)}
$$

### One symmetric $ee/\mu\mu$ trigger, $n$ leptons

For example $\textrm{2e12\_lhloose}$. 
The efficiency is computed by induction over the number of leptons $n$. 
Let's define $S_{n-1}=\ell_1\vee\ldots\vee\ell_{n-1}$, i.e. when at least one leg was fired by one of the first $n-1$ leptons. 
Then: 

$$
\mathbb{P}(T_n) = \mathbb{P}(T_{n-1}  \vee (\ell_n\wedge S_{n-1}))\\
= \mathbb{P}(T_{n-1}) + \mathbb{P}(\ell_n\wedge S_{n-1}) - \mathbb{P}(T_{n-1}\wedge \ell_n\wedge S_{n-1})\\
= (1-\mathbb{P}(\ell_n))\mathbb{P}(T_{n-1})+\mathbb{P}(\ell_n)\mathbb{P}(S_{n-1})\quad\text{since }T_{n-1}\wedge S_{n-1}=T_{n-1}\\
= (1-\mathbb{P}(\ell_n))\mathbb{P}(T_{n-1})+\mathbb{P}(\ell_n)\left[1-\!\!\prod_{1\leq k\leq n-1}\!\!\!(1-\mathbb{P}(\ell_k))\right]\qquad{(6)}
$$
This is straightforward to implement: 

```python
p[0] = 0
s[0] = 1
for k = 1 to n:
    p[k] = (1-e[k])*p[k-1] + e[k]*(1-s[k-1])
    s[k] *= (1-e[k])
return p[n]
```

### One asymmetric $\ell\ell$ trigger, $n$ leptons

For example $\textrm{mu18\_mu8noL1}$. The two legs are differentiated by superscripts ($\ell_k^1$, $\ell_k^2$), 
with the leg (1) having a higher $p_\text{T}$ threshold than the leg (2). 
Using induction again: 

$$
\mathbb{P}(T_n) = 
\mathbb{P}(T_{n-1}\vee (\ell_n^1 \wedge S_{n-1}^2) \vee (\ell_n^2 \wedge S_{n-1}^1))\\
=(1-\mathbb{P}(\ell_n^{\lambda_n}))\mathbb{P}(T_{n-1})
+\mathbb{P}(\ell_n^1)\mathbb{P}(S_{n-1}^2) + \mathbb{P}(\ell_n^2)\mathbb{P}(S_{n-1}^1)\\
\qquad- \mathbb{P}(\ell_n^{\tau_n})\mathbb{P}(S_{n-1}^1\wedge S_{n-1}^2)\\
=(1-\mathbb{P}(\ell_n^{\lambda_n}))\mathbb{P}(T_{n-1})
+(\mathbb{P}(\ell_n^{\lambda_n})-\mathbb{P}(\ell_n^{\tau_n}))\mathbb{P}(S_{n-1}^{\tau_n})\\
\qquad + \mathbb{P}(\ell_n^{\tau_n}) \mathbb{P}(S_{n-1}^1\vee S_{n-1}^2)\qquad{(7)}
$$
where $\lambda_k$ (resp. $\tau_k$) is the index of the trigger leg that is the loosest 
(resp. tightest) according to the established hierarchy at the lepton $k$'s $p_\text{T}$. 

The expressions for $\mathbb{P}(S_{n-1}^1)$ and $\mathbb{P}(S_{n-1}^2)$ are given by (1), 
and $\mathbb{P}(S_{n}^1\vee S_{n}^2)$ is simply the probability 
for a *logical or* of two same-flavour two single-lepton triggers, hence is given by (4). 

## Combination of dilepton and single lepton triggers, only one lepton flavour

### One symmetric $\ell\ell$ + one or more same-flavour $\ell$ triggers, $n$ leptons

First addressing the case of one single-lepton trigger, then generalizing. 
Superscripts 1 and 2 will respectively correspond to the single lepton trigger leg, and the dilepton trigger leg. 
$$
\mathbb{P}(T_n) = \mathbb{P}(T_{n-1} \vee \ell_n^1 \vee (\ell_n^2 \wedge S_{n-1}^2))\\
= \mathbb{P}(T_{n-1})[1-\mathbb{P}(\ell_n^1)] + \mathbb{P}(\ell_n^1)
+ [\mathbb{P}(\ell_n^2)-\mathbb{P}(\ell_n^{\tau_n})][\mathbb{P}(S_{n-1}^2) - \mathbb{P}(T_{n-1}\wedge S_{n-1}^2)] \\
= \mathbb{P}(T_{n-1})[1-\mathbb{P}(\ell_n^1)] + \mathbb{P}(\ell_n^1)
+ [\mathbb{P}(\ell_n^2)-\mathbb{P}(\ell_n^{\tau_n})][\mathbb{P}(T_{n-1}\vee S_{n-1}^2) - \mathbb{P}(T_{n-1})]\\
= \mathbb{P}(T_{n-1})[1-\mathbb{P}(\ell_n^1)] + \mathbb{P}(\ell_n^1)
+ [\mathbb{P}(\ell_n^2)-\mathbb{P}(\ell_n^{\tau_n})][\mathbb{P}(S_{n-1}^1\vee S_{n-1}^2) - \mathbb{P}(T_{n-1})]\\
= \mathbb{P}(T_{n-1})[1-\mathbb{P}(\ell_n^{\lambda_n})] + \mathbb{P}(\ell_n^1)
+ \mathbb{P}(S_{n-1}^1\vee S_{n-1}^2)[\mathbb{P}(\ell_n^2)-\mathbb{P}(\ell_n^{\tau_n})]\\
= \mathbb{P}(T_{n-1})[1-\mathbb{P}(\ell_n^{\lambda_n})] + \mathbb{P}(\ell_n^1)
+ \delta_2^{\lambda_n}\mathbb{P}(S_{n-1}^1\vee S_{n-1}^2)[\mathbb{P}(\ell_n^2)-\mathbb{P}(\ell_n^1)]
$$

The expression for $\mathbb{P}(S_{n-1}^1\vee S_{n-1}^2)$ is given by (4). 

For more than one single-lepton trigger, we denote $Z_n:=S_n^{1,1}\vee\ldots\vee S_n^{1,k}$ the union of the 
$k$ single-lepton triggers. We also denote by the superscript $1$ the loosest single-lepton trigger for the lepton $n$.  
Then: 
$$
\mathbb{P}(T_n) 
= \mathbb{P}(T_{n-1} \vee (\ell_n^{1,1}\vee\ldots\vee\ell_n^{1,k}) \vee (\ell_n^2 \wedge S_{n-1}^2))\\
= \mathbb{P}(T_{n-1} \vee \ell_n^{1} \vee (\ell_n^2 \wedge S_{n-1}^2))\\
= \mathbb{P}(T_{n-1})[1-\mathbb{P}(\ell_n^{\lambda_n})] + \mathbb{P}(\ell_n^{1})
+ \delta_2^{\lambda_n}
\mathbb{P}(Z_{n-1}\vee S_{n-1}^2)[\mathbb{P}(\ell_n^2)-\mathbb{P}(\ell_n^{1})]\qquad{(8)}
$$


### One asymmetric $\ell\ell$ + one or more same-flavour $\ell$ trigger, $n$ leptons

Superscripts $2$ and $3$ indicate the two legs of the dilepton trigger, while $1$ indicates the loosest of the single-lepton trigger legs 
for the lepton $n$ (i.e. equivalent to $\lambda_n^1$ in the previous section). However, $S_{n-1}^1$ still represents the event of triggering with any of the single-lepton triggers for one of the first $n-1$ leptons. 
 
$$
\mathbb{P}(T_n) = \mathbb{P}(T_{n-1} \
\vee (\ell_n^{1} \wedge S_{n-1}^{2}) \vee (\ell_n^{2} \wedge S_{n-1}^{1}) \vee \ell_n^3) \\

=[1-\mathbb{P}(\ell_n^3)]\mathbb{P}(T_{n-1}) + \mathbb{P}(\ell_n^3)
+ [\mathbb{P}(\ell_n^{1}) - \mathbb{P}(\ell_n^{\tau_n^{1,3}})][\mathbb{P}(S_{n-1}^{2}) - \mathbb{P}(T_{n-1}\wedge S_{n-1}^2) ]\\
\qquad + [\mathbb{P}(\ell_n^{2}) - \mathbb{P}(\ell_n^{\tau_n^{2,3}})][\mathbb{P}(S_{n-1}^{1})-\mathbb{P}(T_{n-1}\wedge S_{n-1}^1)]\\
\qquad +  [\mathbb{P}(\ell_n^{\tau_{n}}) - \mathbb{P}(\ell_n^{\tau_{n}^{1,2}}) ]
[\mathbb{P}(S_{n-1}^{1}\wedge S_{n-1}^{2}) - \mathbb{P}(T_{n-1}\wedge S_{n-1}^{1}\wedge S_{n-1}^{2})]\\

=[1-\mathbb{P}(\ell_n^3)]\mathbb{P}(T_{n-1}) + \mathbb{P}(\ell_n^3)
 + [\mathbb{P}(\ell_n^{1}) - \mathbb{P}(\ell_n^{\tau_n^{1,3}})][\mathbb{P}(T_{n-1}\vee  S_{n-1}^2) - \mathbb{P}(T_{n-1})]\\
\qquad + [\mathbb{P}(\ell_n^{2}) - \mathbb{P}(\ell_n^{\tau_n^{2,3}})][\mathbb{P}(T_{n-1}\vee  S_{n-1}^1) - \mathbb{P}(T_{n-1})]\\
\qquad +  [\mathbb{P}(\ell_n^{\tau_{n}}) - \mathbb{P}(\ell_n^{\tau_{n}^{1,2}}) ]
[\mathbb{P}(T_{n-1} \vee S_{n-1}^{1})+\mathbb{P}(T_{n-1}\vee S_{n-1}^{2}) 
- \mathbb{P}(T_{n-1})-\mathbb{P}(T_{n-1} \vee S_{n-1}^{1} \vee S_{n-1}^{2})]\\

= [1-\mathbb{P}(\ell_n^{\lambda_n})]\mathbb{P}(T_{n-1}) + \mathbb{P}(\ell_n^3)
+ (1-\delta_3^{\lambda_n})[\mathbb{P}(\ell_n^{\lambda_n}) - \mathbb{P}(\ell_n^{m_n})]\mathbb{P}(T_{n-1}\vee  S_{n-1}^{\tau_n^{1,2}})\\
\qquad +  \delta_3^{\tau_n}[\mathbb{P}(\ell_n^{m_n}) - \mathbb{P}(\ell_n^3) ]
\mathbb{P}(T_{n-1} \vee (S_{n-1}^{1}\wedge S_{n-1}^{2}))\\

= [1-\mathbb{P}(\ell_n^{\lambda_n})]\mathbb{P}(T_{n-1}) + \mathbb{P}(\ell_n^3)
+ (1-\delta_3^{\lambda_n})[\mathbb{P}(\ell_n^{\lambda_n}) -\mathbb{P}(\ell_n^{m_n})]\mathbb{P}(S_{n-1}^3\vee  S_{n-1}^{\tau^{1,2}_n})\\
\qquad + \delta_3^{\tau_n} [\mathbb{P}(\ell_n^{m_n}) - \mathbb{P}(\ell_n^3) ]
\mathbb{P}(S_{n-1}^1 \vee S_{n-1}^{2}\vee S_{n-1}^{3})\qquad\text{(9)}
$$
where $m_n$ stands for the *medium* leg (neither the tightest, nor the loosest) according to the hierarchy for the lepton $n$. 
The different terms can be evaluated with (4), and using induction. 


### Two sym. $\ell\ell$ + several $\ell$ triggers, $n$ leptons

Superscripts $1$ and $2$ stand for the symmetric triggers, and superscript $3$ for the loosest single lepton trigger for the lepton $n$. $S_{n-1}^j$ represents the event of triggering the leg $j$ with any of the first $n-1$ leptons 
(for $j=3$, it should be interpreted as "any single lepton trigger"). 
$m_n$ stands for the sub-tightest leg for the lepton $n$. 

$$
\mathbb{P}(T_n)=
\mathbb{P}(T_{n-1}\vee (\ell_n^1\wedge S_{n-1}^1) \vee (\ell_n^2\wedge S_{n-1}^2) \vee \ell_n^3)
$$

This happens to be the same starting expression as (8), except that the reduction of the $(T_{n-1}\vee S_{n-1}\ldots)$ 
terms in the last step is different; here the remaining terms resolve to:
- $T_{n-1}\vee S_{n-1}^{1,2} = D_{n-1}^{2,1}\vee Z_{n-1}\vee S_{n-1}^{1,2}$, then evaluated with (8)
- $T_{n-1}\vee S_{n-1}^{1}\vee S_{n-1}^{2} = Z_{n-1} \vee S_{n-1}^{1}\vee S_{n-1}^2$, with (2)

where $D^{1,2}$ stand for the symmetric triggers and $Z$ for the combination of single-lepton triggers. 

### One sym. $\ell\ell$ + one asym. $\ell\ell$ + several $\ell$ triggers, $n$ leptons

Superscript $1$ stands for the symmetric trigger, superscripts $2$ and $3$ for the two legs of the asymmetric trigger, 
and superscript $4$ for the loosest single lepton trigger for the lepton $n$. $S_{n-1}^j$ represents the event of 
triggering the leg $j$ with any of the first $n-1$ leptons (for $j=4$, it should be interpreted as "any single lepton trigger"). 
$m_n$ stands for the second tightest leg for the lepton $n$, $\nu_n$ for the third. 
$$
\mathbb{P}(T_n)=
\mathbb{P}(T_{n-1}\vee (\ell_n^1\wedge S_{n-1}^1) \vee (\ell_n^2\wedge S_{n-1}^3) 
\vee (\ell_n^3\wedge S_{n-1}^2) \vee \ell_n^4)\\

=\mathbb{P}(\ell_n^4) + \mathbb{P}(T_{n-1})[1-\mathbb{P}(\ell_n^4)]
+ [\mathbb{P}(S_{n-1}^1) - \mathbb{P}(T_{n-1}\wedge S_{n-1}^1)][\mathbb{P}(\ell_n^1)-\mathbb{P}(\ell_n^{\tau_n^{1,4}})])\\
\qquad+[\mathbb{P}(S_{n-1}^2) - \mathbb{P}(T_{n-1}\wedge S_{n-1}^2)][\mathbb{P}(\ell_n^3)-\mathbb{P}(\ell_n^{\tau_n^{3,4}})]
+ [\mathbb{P}(S_{n-1}^3) - \mathbb{P}(T_{n-1}\wedge S_{n-1}^3)][\mathbb{P}(\ell_n^2)-\mathbb{P}(\ell_n^{\tau_n^{2,4}})])\\
\qquad + [\mathbb{P}(S_{n-1}^1\wedge S_{n-1}^2) - \mathbb{P}(T_{n-1}\wedge S_{n-1}^1\wedge S_{n-1}^2)]
[\mathbb{P}(\ell_n^{\tau_n^{1,3,4}})-\mathbb{P}(\ell_n^{\tau_n^{1,3}})])\\
\qquad+ [\mathbb{P}(S_{n-1}^1\wedge S_{n-1}^3) - \mathbb{P}(T_{n-1}\wedge S_{n-1}^1\wedge S_{n-1}^3)]
[\mathbb{P}(\ell_n^{\tau_n^{1,2,4}})-\mathbb{P}(\ell_n^{\tau_n^{1,2}})])\\
\qquad+ [\mathbb{P}(S_{n-1}^2\wedge S_{n-1}^3) - \mathbb{P}(T_{n-1}\wedge S_{n-1}^2\wedge S_{n-1}^3)]
[\mathbb{P}(\ell_n^{\tau_n^{2,3,4}})-\mathbb{P}(\ell_n^{\tau_n^{2,3}})])\\
\qquad+ [\mathbb{P}(S_{n-1}^1\wedge S_{n-1}^2\wedge S_{n-1}^3) 
- \mathbb{P}(T_{n-1}\wedge S_{n-1}^1\wedge S_{n-1}^2\wedge S_{n-1}^3)]
[\mathbb{P}(\ell_n^{\tau_n^{1,2,3}})-\mathbb{P}(\ell_n^{\tau_n^{1,2,3,4}})]\\

= \mathbb{P}(\ell_n^4) + \mathbb{P}(T_{n-1})[1 - \mathbb{P}(\ell_n^{\lambda_n})]
+ \delta_1^{\lambda_n}\mathbb{P}(T_{n-1}\vee S_{n-1}^1)
[\mathbb{P}(\ell_n^1)-\mathbb{P}(\ell_n^{\nu_n})])\\
\qquad+\delta_3^{\lambda_n}\mathbb{P}(T_{n-1}\vee S_{n-1}^2)
[\mathbb{P}(\ell_n^3)-\mathbb{P}(\ell_n^{\nu_n})]
+ \delta_2^{\lambda_n}\mathbb{P}(T_{n-1}\vee S_{n-1}^3)
[\mathbb{P}(\ell_n^2)-\mathbb{P}(\ell_n^{\nu_n})])\\
\qquad -\mathbb{P}(T_{n-1}\vee S_{n-1}^1\vee S_{n-1}^2)
[\mathbb{P}(\ell_n^{m_n})-\mathbb{P}(\ell_n^{\tau_n^{1,3}})]
-\mathbb{P}(T_{n-1}\vee S_{n-1}^1\vee S_{n-1}^3)[\mathbb{P}(\ell_n^{m_n})-\mathbb{P}(\ell_n^{\tau_n^{1,2}})])\\
\qquad -\mathbb{P}(T_{n-1}\vee S_{n-1}^2\vee S_{n-1}^3)[\mathbb{P}(\ell_n^{m_n})-\mathbb{P}(\ell_n^{\tau_n^{2,3}})]
+ \delta_4^{\tau_n}\mathbb{P}(T_{n-1}\vee S_{n-1}^1\vee S_{n-1}^2\vee S_{n-1}^3)]
[\mathbb{P}(\ell_n^{m_n})-\mathbb{P}(\ell_n^{4})]\qquad{(10)}
$$

The different terms forming this expression can be evaluated with already-established formulas for simpler combinations of triggers, 
by using that: 
- $T_{n-1}\vee S_{n-1}^1 = A_{n-1} \vee Z_{n-1} \vee S_{n-1}^1$, then evaluated with (9)
- $T_{n-1}\vee S_{n-1}^{2,3} = D_{n-1} \vee Z_{n-1} \vee S_{n-1}^{2,3}$, with (8)
- $T_{n-1}\vee S_{n-1}^1\vee S_{n-1}^{2,3} = Z_{n-1} \vee S_{n-1}^1\vee S_{n-1}^{2,3}$, with (2)
- $T_{n-1}\vee S_{n-1}^2\vee S_{n-1}^3= D_{n-1}\vee Z_{n-1} \vee S_{n-1}^2\vee S_{n-1}^3$, with (8)
- $T_{n-1}\vee S_{n-1}^1\vee S_{n-1}^2\vee S_{n-1}^3= Z_{n-1} \vee S_{n-1}^1\vee S_{n-1}^2\vee S_{n-1}^3$, with (2)

where $D$ stands for the symmetric trigger, $A$ for the asymmetric trigger, and $Z$ for the combination of single-lepton triggers. 



## Combinations of dilepton and single lepton triggers, mixing lepton flavours

### One $e\mu$ + several single-$e/\mu$ triggers, $n_e+n_\mu$ leptons

We denote by $Z_e$ and $Z_\mu$ the unions of the single-electron (resp. single-muon) triggers, 
and by $S_e^2/S_\mu^2$ the legs of the $e\mu$ trigger. Then: 

$$
\mathbb{P}(T_{n_e+n_\mu}) = \mathbb{P}((S_e^2\wedge S_\mu^2)\vee Z_e \vee Z_\mu)\\
= \mathbb{P}(Z_e\vee Z_\mu) + [\mathbb{P}(S_e^2) - \mathbb{P}(S_e^2\wedge Z_e)][\mathbb{P}(S_\mu^2) - \mathbb{P}(S_\mu^2\wedge Z_\mu)]\\
= \mathbb{P}(Z_e\vee Z_\mu)+ [\mathbb{P}(Z_e\vee S_e^2) - \mathbb{P}(Z_e)][\mathbb{P}(Z_\mu\vee S_\mu^2) - \mathbb{P}(Z_\mu) ]\\
= 1 - [1-\mathbb{P}(Z_e)][1-\mathbb{P}(Z_\mu)]+ [\mathbb{P}(Z_e\vee S_e^2) - \mathbb{P}(Z_e)][\mathbb{P}(Z_\mu\vee S_\mu^2) - \mathbb{P}(Z_\mu) ]\qquad{(11)}
$$

### One $ee/\mu\mu$ + one $e\mu$ trigger, $n_e+n_\mu$ leptons

$$
\mathbb{P}(T_{n_e+n_\mu}) = \mathbb{P}(D_e \vee (S_e\wedge S_\mu))\\
= \mathbb{P}(D_e)(1-\mathbb{P}(S_\mu)) + \mathbb{P}(S_\mu)\mathbb{P}(D_e\vee S_e)\qquad{(12)}
$$

the different terms can be evaluated with (1), (6), (7) or (8), 
depending whether the $ee/\mu\mu$ trigger is symmetric or not. 

### One $ee$ + one $\mu\mu$ + one $e\mu$ trigger, $n_e+n_\mu$ leptons

$$
\mathbb{P}(T_{n_e+n_\mu}) = \mathbb{P}(D_e \vee (S_e\wedge S_\mu) \vee D_\mu)\\
= \mathbb{P}(D_e)[1-\mathbb{P}(D_\mu\vee S_\mu)] + \mathbb{P}(D_\mu)[1-\mathbb{P}(D_e\vee S_e)]\\
\qquad +  \mathbb{P}(D_e\vee S_e) \mathbb{P}(D_\mu\vee S_\mu)\qquad{(13)}
$$

The different terms can be evaluated with (1), (6) or (8). 


### One $ee$ + one $\mu\mu$ + one $e\mu$ trigger + several $e/\mu$ triggers, $n_e+n_\mu$ leptons

We denote $Z_e:=S_e^{1a}\vee\ldots\vee S_e^{1{k_e}}$ the union of the $k_e$ single-electron triggers, 
and similarly $Z_\mu$ for the $k_\mu$ single-muon triggers. Then: 
$$
\mathbb{P}(D_e \vee (S_e^2\wedge S_\mu^2) \vee D_\mu \vee Z_e \vee Z_\mu)\\
=
\mathbb{P}(D_\mu \vee Z_\mu)
+ [1-\mathbb{P}(D_\mu \vee Z_\mu)]\mathbb{P}(D_e \vee Z_e)\\
\qquad+ [\mathbb{P}(S_\mu^2) - \mathbb{P}(S_\mu^2\wedge D_\mu) 
- \mathbb{P}(S_\mu^2\wedge Z_\mu)+ \mathbb{P}(S_\mu^2\wedge D_\mu\wedge Z_\mu)]\\
\qquad\qquad\times[\mathbb{P}(S_e^2) - \mathbb{P}(S_e^2\wedge D_e) - \mathbb{P}(S_e^2\wedge Z_e) + \mathbb{P}(S_e^2\wedge D_e\wedge Z_e)]\\
=
1
- [1-\mathbb{P}(D_\mu \vee Z_\mu)][1-\mathbb{P}(D_e \vee Z_e)]\\
\qquad+[\mathbb{P}(D_\mu\vee Z_\mu\vee S_\mu^2) - \mathbb{P}(D_\mu\vee Z_\mu)]
[\mathbb{P}(D_e\vee Z_e \vee S_e^2) - \mathbb{P}(D_e\vee Z_e)]\qquad{(14)}
$$

The evaluation of the different terms (one dilepton trigger + several single-lepton triggers) can be performed 
with (8) and (9). 


### Two $ee$ + two $\mu\mu$ + two $e\mu$ + several $e/\mu$ triggers, $n_e+n_\mu$ leptons

Notation: $E=$ all dielectron or single electron triggers, $M=$ all dimuon or single muon triggers, 
$S_e^k$ (resp. $S_\mu^k$) the electron leg (resp. muon leg) of the $k$-th $e\mu$ trigger. 

$$
\mathbb{P}(T_n)=\mathbb{P}(E\vee (S_e^1\wedge S_\mu^1)\vee(S_e^1\wedge S_\mu^1)\vee M)\\
= 1 - (1-\mathbb{P}(E))(1-\mathbb{P}(M)) + [\mathbb{P}(E\vee S_e^1)-\mathbb{P}(E)][\mathbb{P}(M\vee S_\mu^1)-\mathbb{P}(M)]\\
\qquad + [\mathbb{P}(E\vee S_e^2)-\mathbb{P}(E)][\mathbb{P}(M\vee S_\mu^2)-\mathbb{P}(M)]\\
\qquad + [\mathbb{P}(E\vee S_e^1)+\mathbb{P}(E\vee S_e^2)-\mathbb{P}(E)-\mathbb{P}(E\vee S_e^1\vee S_e^2)]\times\\
\qquad\qquad\times[-\mathbb{P}(M\vee S_\mu^1)-\mathbb{P}(M\vee S_\mu^2)+\mathbb{P}(M)+\mathbb{P}(M\vee S_\mu^1\vee S_\mu^2)]\qquad\text{(15)}
$$

## Trilepton triggers

### Fully symmetric $3e/3\mu$ trigger, $n$ leptons
By induction:
$$
\mathbb{P}(T_n) = \mathbb{P}(T_{n-1} \vee (D_{n-1}\wedge \ell_n))\\
= \mathbb{P}(T_{n-1})(1-\mathbb{P}(\ell_n)) + \mathbb{P}(\ell_n)\mathbb{P}(D_{n-1})\qquad\text{(16)}
$$
with $\mathbb{P}(D_{n-1})$ given by (6). 

### Mixed $2e\_\mu/2\mu\_e/e\_e\_\mu/\mu\_\mu\_e$ trigger, $n_e+n_\mu$ leptons


$$
\mathbb{P}(T_{n_e+n_\mu}) = \mathbb{P}(E_{n_e})\mathbb{P}(M_{n_\mu})\qquad\text{(17)}
$$
with $\mathbb{P}(M_{n_\mu})$ given by (1) and $\mathbb{P}(E_{n_e})$ by either (6) or (7)
depending whether the two electrons legs are identical or not. 




### Half-symmetric $e\_2e/\mu\_2\mu$ trigger, $n$ leptons
Superscript 1 indicates the leg of the symmetric part, and 2 the other leg; $D_{n-1}$ 
and $A_{n-1}$ stand for pseudo dilepton triggers built respectively with legs $1+1$ and $1+2$. 
By induction: 
$$
\mathbb{P}(T_n) = \mathbb{P}(T_{n-1} \vee (A_{n-1}\wedge \ell_n^1) \vee (D_{n-1}\wedge \ell_n^2))\\
= [1-\mathbb{P}(\ell_n^{\lambda_n})]\mathbb{P}(T_{n-1}) +  [\mathbb{P}(\ell_n^1) - \mathbb{P}(\ell_n^{\tau_n})]\mathbb{P}(A_{n-1})\\
\qquad+ [\mathbb{P}(\ell_n^2) - \mathbb{P}(\ell_n^{\tau_n})]\mathbb{P}(D_{n-1})
+ \mathbb{P}(\ell_n^{\tau_n})\mathbb{P}(D_{n-1}\vee A_{n-1})\qquad\text{(18)}
$$

with $\mathbb{P}(D_{n-1})$ given by (6), $\mathbb{P}(A_{n-1})$ by (7), 
 and $\mathbb{P}(D_{n-1}\vee A_{n-1})$ by (10); 
the latter's expression is however greatly simplified since there is no single-lepton trigger involved 
and the two "dilepton" triggers have a leg in common. 
Its expression is therefore: 
$$
\mathbb{P}(D_{n}\vee A_{n}) = [1-\mathbb{P}(\ell_n^{\lambda_n})]\mathbb{P}(D_{n-1}\vee A_{n-1}) + [\mathbb{P}(\ell_n^{\lambda_n})-\mathbb{P}(\ell_n^1)]\mathbb{P}(S_{n-1}^1)
+\mathbb{P}(\ell_n^1)\mathbb{P}(S_{n-1}^1\vee S_{n-1}^2)\qquad\text{(19)}
$$


### Two complementary mixed $2e\_\mu/2\mu\_e/e\_e\_\mu/\mu\_\mu\_e$ triggers, $n_e+n_\mu$ leptons

Complementary = two electrons+1 muon for one trigger, and two muons+1 electron for the other. 

$$
\mathbb{P}(T_{n_e+n_\mu}) = \mathbb{P}((S_e\wedge D_\mu) \vee (S_\mu\wedge D_e))\\
= \mathbb{P}(S_e)\mathbb{P}(D_\mu) + \mathbb{P}(S_\mu)\mathbb{P}(D_e) 
+ [\mathbb{P}(D_e\vee S_e) - \mathbb{P}(D_e)-\mathbb{P}(S_e)]
[\mathbb{P}(D_\mu)+\mathbb{P}(S_\mu)-\mathbb{P}(D_\mu\vee S_\mu)]\qquad\text{(20)}
$$
with $\mathbb{P}(S)$ given by (1), $\mathbb{P}(D)$ by either (6) or (7)
and $\mathbb{P}(D\vee S)$ by (8) or (9).

## Tetralepton triggers

### Fully symmetric $4e/4\mu$ trigger, $n$ leptons
Similarly as for the trilepton case:
$$
\mathbb{P}(T_n) = \mathbb{P}(T_{n-1})(1-\mathbb{P}(\ell_n)) + \mathbb{P}(\ell_n)\mathbb{P}(M_{n-1})\qquad\text{(21)}
$$
with $\mathbb{P}(M_{n-1})$ given by (16). 