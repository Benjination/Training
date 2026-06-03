# Training Notes: Dolphin ICS Environment

This workspace contains my daily training notes while working in an environment that uses Dolphin Interconnect Solutions (Dolphin ICS) technology.

## About Dolphin ICS

Dolphin ICS builds high-performance interconnect solutions used in technical computing, real-time systems, and other low-latency data transfer environments.

## Hardware Context

The systems I am working with use Dolphin PCIe-based adapter hardware to connect nodes for high-bandwidth, low-latency communication. In my notes, this includes checking link state, lane width (x8 vs x16), and adapter firmware/partition status.

## Software Features

The software stack in these notes includes Dolphin diagnostic and configuration utilities such as:

- `dis_diag`
- `dis_config`
- `dis_firmwaretool`
- EEPROM upgrade scripts

These tools are used to verify node health, inspect firmware state, reset adapters, and apply configuration changes.

## SISCI API

SISCI is Dolphin's software interface for shared-memory-style communication over the interconnect. It is commonly used to build applications that need predictable, low-latency communication between systems.

---

The files in this repository are a chronological record of troubleshooting, upgrades, and validation steps performed during training.
