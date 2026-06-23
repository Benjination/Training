# Genfi-5 Profile

## Snapshot

- Hostname: genfi-5
- Dates observed: 2026-06-04 and 2026-06-09
- Role: Dolphin node in 2-node setup (peer with genfi-6)

## OS and Kernel

- Previous kernel string observed:
	- Linux genfi-5 4.18.0-553.6.1.el8.x86_64 #1 SMP Thu May 30 04:13:58 UTC 2024 x86_64 x86_64 x86_64 GNU/Linux
- Current kernel string observed after upgrade:
	- Linux genfi-5 5.14.0-687.12.1.el9_8.x86_64
- OS family inference:
	- EL9-based system

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

## Compatibility Baseline vs Genfi-6

- genfi-5 is now on EL9 kernel line (5.14.x).
- genfi-6 is now also on the stock EL9 kernel line (5.14.x).
- Nodes are now aligned on the same major OS and kernel family.
- Remaining compatibility work is primarily Dolphin package alignment, driver/module health, host config, and node/session discovery.

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
	- Local NodeId: 4
	- Serial number: MXH930-DE-0000-002377
	- BDF: 09:00.1
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

## Hardware Match Assessment

- Same adapter family as genfi-6: MXH930
- Same observed link behavior as genfi-6: x16 Gen4 with the cable inserted
- Not exactly the same hardware unit: the adapter serial numbers differ
- Conclusion: the two nodes are hardware-matched at the model/link level, but not identical by serial number

## Partner Visibility and Session State

- Partner adapter type: MXH930
- Partner serial number: MXH930-000089
- Nodes detected in topology: 0004 and 0008
- Session status:
	- Node 4: Session valid
	- Node 8: Session valid

## Current Health Summary

- Local adapter and link are healthy at x16 Gen4.
- Peer node visibility is present for both nodes (4 and 8).
- Session status is valid on both nodes.
- dis_diag reported 0 warnings and 0 errors, with overall TEST RESULT: PASSED.

## Data Still Needed

- `/etc/dis/dishosts.conf`
- Installed Dolphin package list
- `dis_firmwaretool --version`
