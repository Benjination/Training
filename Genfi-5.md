# Genfi-5 Profile

## Snapshot

- Hostname: genfi-5
- Date observed: 2026-06-04
- Role: Dolphin node in 2-node setup (peer with genfi-6)

## OS and Kernel

- Previous kernel string observed:
	- Linux genfi-5 4.18.0-553.6.1.el8.x86_64 #1 SMP Thu May 30 04:13:58 UTC 2024 x86_64 x86_64 x86_64 GNU/Linux
- Current kernel string observed after upgrade:
	- Linux genfi-5 5.14.0-710.el9.x86_64
- OS family inference:
	- EL9-based system

## Compatibility Baseline vs Genfi-6

- genfi-5 is now on EL9 kernel line (5.14.x).
- genfi-6 is now also on the stock EL9 kernel line (5.14.x).
- Nodes are now aligned on the same major OS and kernel family.
- Remaining compatibility work is primarily Dolphin package alignment, driver/module health, host config, and node/session discovery.

## Data Still Needed

- `dis_diag`
- `/etc/dis/dishosts.conf`
- Installed Dolphin package list
- `dis_firmwaretool --version`
