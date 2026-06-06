# Genfi-6 Profile

## Snapshot

- Hostname seen in diagnostics: genfi-6
- Dates observed: 2026-06-03 and 2026-06-04
- Role: Dolphin node in 2-node direct topology

## OS and Kernel

- Previous kernel string observed:
	- Linux genfi-6 6.1.111-1.el9.elrepo.x86_64 #1 SMP PREEMPT_DYNAMIC Wed Sep 18 14:15:37 EDT 2024 x86_64 x86_64 x86_64 GNU/Linux
- Current kernel string observed after reverting to stock EL9 kernel:
  - Linux genfi-6 5.14.0-134.el9.x86_64 #1 SMP PREEMPT_DYNAMIC Thu Jul 21 12:57:06 UTC 2022 x86_64 x86_64 x86_64 GNU/Linux
- OS family inference: EL9-based system

## Dolphin Software Stack (observed)

- dis_diag version: 5.26.0
- Driver version: Dolphin IRM (GX) 5.26.0-d
- dishosts file header reports created by:
	- Dishostseditor 5.25.0-d
- Reported during troubleshooting:
	- ExpressWare package line being used: 5.25.2

Note: This indicates a likely mixed-version stack between tools, driver, and generated config metadata.

## Adapter and Link State (from dis_diag)

- Number of configured local adapters found: 1
- Local adapter:
	- Type: MXH930
	- Mode: NTB
	- Local NodeId: 8
	- BDF: 08:00.1
	- Topology type: Direct 2 nodes
	- Enabled links: 1
- PCIe upstream:
	- State: x16 Gen4 (16 GT/s)
	- Capabilities: x16 Gen4
- Link 0:
	- State: ENABLED
	- Active: 1
	- Cable inserted: 1
	- Link width/speed state: x16 Gen4

## Partner Visibility (from dis_diag)

- Partner adapter type: MXH930
- Nodes detected in topology:
	- 2026-06-03 run: 0004 and 0008
	- 2026-06-04 run: 0008 only

Interpretation of delta:

- The local adapter and physical link still look healthy (x16 Gen4, link enabled, cable inserted).
- Peer node visibility changed from 2 nodes to local-only, which points to a session/discovery/config mismatch rather than a local PCIe link failure.

## dishosts.conf State Captured

Observed local block contained:

- HOSTNAME: LOCALHOST
- ADAPTER lines for LOCALHOST_a0, LOCALHOST_a1, LOCALHOST_a2 with node id 4
- STRIPE defined across LOCALHOST_a0 and LOCALHOST_a1
- SOCKET mapped to LOCALHOST_a0

Warning seen in dis_diag:

- dishosts.conf does not specify the local host

## Known Mismatches and Risks

- Local NodeId seen live is 8, but dishosts local adapter entries were using node id 4.
- dishosts local hostname entry LOCALHOST did not match local host identity expected by tools.
- dishosts modeled 3 local adapters, while dis_diag reported 1 configured local adapter.
- Version skew likely present across ExpressWare, driver, and host/config tools.

## Current Health Summary

- Hardware link health on genfi-6 appears good at x16 Gen4.
- Host is no longer on the ELRepo kernel line and is back on the stock EL9 5.14 kernel family.
- Main issue area appears to be software/config consistency and cross-node discovery/session alignment, not physical link stability.

## Compatibility Status vs Genfi-5

- genfi-5 and genfi-6 are now both on EL9 and both on the 5.14 kernel family.
- The remaining work is to align Dolphin packages, ensure matching kernel modules/drivers are loaded, and correct host/node discovery configuration.
