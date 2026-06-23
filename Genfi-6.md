# Genfi-6 Profile

## Snapshot

- Hostname seen in diagnostics: genfi-6
- Dates observed: 2026-06-03, 2026-06-04, and 2026-06-09
- Role: Dolphin node in 2-node direct topology

## OS and Kernel

- Previous kernel string observed:
	- Linux genfi-6 6.1.111-1.el9.elrepo.x86_64 #1 SMP PREEMPT_DYNAMIC Wed Sep 18 14:15:37 EDT 2024 x86_64 x86_64 x86_64 GNU/Linux
- Current kernel string observed after reverting to stock EL9 kernel:
	- Linux genfi-6 5.14.0-708.el9.x86_64 #1 SMP 
- OS family inference: EL9-based system

## Hardware Profile

- Architecture: x86_64
- CPU: AMD Ryzen 5 3600XT 6-Core Processor
- CPU count: 12 logical CPUs, 6 cores, 12 threads
- CPU frequency range: 2200.0000 MHz min, 3800.0000 MHz max
- Cache: 192 KiB L1d, 192 KiB L1i, 3 MiB L2, 32 MiB L3
- Motherboard: ASUSTeK COMPUTER INC. PRIME X570-P, Rev X.0x
- BIOS vendor/version: American Megatrends Inc. 5021
- BIOS date: 09/29/2024
- Virtualization: AMD-V
- NUMA: 1 node

## Dolphin Software Stack (observed)

- dis_diag version: 5.25.2
- Driver version: Dolphin IRM (GX) 5.25.2
- ExpressWare release line shown by driver:
	- DIS_RELEASE_5_25_2_APR_24_2026

## Adapter and Link State (from dis_diag)

- Number of configured local adapters found: 1
- Local adapter:
	- Type: MXH930
	- Mode: NTB
	- Local NodeId: 8
	- Serial number: MXH930-CF-0000-000089
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
	- Link required: x8 Gen3

## Hardware Match Assessment

- Same adapter family as genfi-5: MXH930
- Same observed link behavior as genfi-5: x16 Gen4 with the cable inserted
- Not exactly the same hardware unit: the adapter serial numbers differ
- Conclusion: the two nodes are hardware-matched at the model/link level, but not identical by serial number

## Partner Visibility and Session State

- Partner adapter type: MXH930
- Partner serial number: MXH930-002377
- Nodes detected in topology: 0004 and 0008
- Session status:
	- Node 4: Session valid
	- Node 8: Session valid

## Current Health Summary

- Local adapter and link are healthy at x16 Gen4.
- Host is no longer on the ELRepo kernel line and is back on the stock EL9 5.14 kernel family.
- Peer node visibility is present for both nodes (4 and 8).
- Session status is valid on both nodes.
- dis_diag reported 0 warnings and 0 errors, with overall TEST RESULT: PASSED.

## Compatibility Status vs Genfi-5

- genfi-5 and genfi-6 are now both on EL9 and both on the 5.14 kernel family.
- Current diagnostics show aligned 5.25.2 software stack, healthy link state, and valid sessions on both nodes.
