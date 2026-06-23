# Week 4 Tuesday – Follow-up Response to Fujitsu Inquiry

**From:** Sharath Krishna Karanam  
**Date:** 2026-06-23  
**Ticket:** DCS-1540

---

## Summary Response

Thank you for the follow-up questions. Based on technical guidance from our infrastructure team, here are clarifications on the key concerns:

---

## 1. PCIe Host Identification Without IP/Network

**Question:** Is it possible to identify connected hosts using only direct PCIe cables?

**Response:** 
Our system matches hosts via IP-addresses. For network-less deployments:

- **Hosts with NICs:** Assign static IP addresses
- **Hosts without NICs:** Configure dummy IP-interfaces (required for SuperSockets compatibility, as it replaces network calls via existing IP-interfaces)
We also heard that a proprietary port is used. Could you please clarify why SSH is still needed and how this port is involved?
**Fallback Options** (if IP-interfaces unavailable):
- Custom `dishosts.conf` files per node using `DIS_LOCAL` hostname identifiers
- Alternative: Remove `dishosts.conf` + Node Manager, call `dis_config` in `/etc/rc.local`
  - **Trade-off:** Breaks SuperSockets, but NodeId configures post-boot

---

## 2. Installation Prerequisites

**Question:** Is pre-cable installation supported (ExpressWare installed on both nodes before PCIe connection)?

**Response:** Yes, this scenario is supported. Post-connection steps are detailed in Section 4.

---

## 3. Post-Cable Connection Steps

After physically connecting PCIe cables:
1. Verify IP-interfaces are operational (static IPs or dummy interfaces)
2. Run connection validation checks (see verification tests below)
3. Execute cluster formation with `--install-frontend` (see Section 5)

---

## 4. Kernel-ExpressWare Version Mismatch Errors

**Question:** What error does dis_diag report on kernel mismatch?

**Response:**
`dis_diag` connects to the IRM (`dis_Irm`) kernel module, which depends on lower-level interconnect drivers (`dis_mx.conf`). If version mismatch exists:
- Neither module will load against the running kernel
- **Error message:** `dis_diag` reports that drivers are not loaded

**Action:** Rebuild modules against the current kernel version.

---

## 5. Cluster Formation & SSH Dependency (--install-frontend)

**Question:** Why is SSH still required despite "passwordless" claims? How does the proprietary port work?

**Response:**
SSH is required for `--install-frontend` (not optional). The cluster formation uses:

- **Network Manager → Node Manager:** TCP port **3443** (proprietary cluster protocol)
- **Admin/GUI tool → Network Manager:** TCP port **3444**

While the installation process initiates via SSH, the actual cluster communication uses these proprietary ports after setup.

---

## Verification Tests

Run these tests to confirm assumptions:

### Test 1: IP-Interface Availability
```bash
# On each node, verify IP-interfaces exist
ip addr show
# Expected: At least one active interface (real or dummy)
# Dummy interface example: ip link add dummy0 type dummy
```

### Test 2: Kernel Module Load Status
```bash
# Check if kernel modules match ExpressWare build
modinfo dis_mx
modinfo dis_Irm
# Expected: No "No such file or directory" errors
```

### Test 3: Proprietary Port Connectivity
```bash
# Verify Network Manager → Node Manager communication
netstat -tlnp | grep -E '3443|3444'
# Expected: Listening services on ports 3443 and 3444 post-deployment
```

### Test 4: Direct PCIe Link Detection
```bash
# After cable connection, check if interface is recognized
lspci | grep -i express  # Or search for your NIC/adapter
ethtool -i <interface>  # Verify driver loaded
```

### Test 5: dis_diag Module Verification
```bash
# Test dis_diag driver status
dis_diag
# Expected: Clean output (no "drivers not loaded" errors)
# If error occurs: Check kernel version matches ExpressWare build
```

### Test 6: Super Sockets Validation (Optional)
```bash
# If SuperSockets in use, verify IP-interface dependency
# (Confirms that dummy IPs work as documented)
# Run application communication test post-cluster-formation
```

---

## Original Inquiry


Sharath Krishna Karanam
Today 07:08
Thanks Ben for the response. We have a few additional queries:


Is it possible to identify the connected host using only a direct PCIe cable connection (without IP/network)?

Is the following scenario supported (hosts up, no cables connected)?

Install ExpressWare on both nodes.

After connecting the PCIe cables what steps or configurations are required to establish connectivity?

What error is reported in dis_diag when there is a mismatch between the upgraded kernel and the currently installed ExpressWare version?

Could you explain the technical mechanism behind cluster formation using --install-frontend? Specifically, as discussed in the meeting, it was mentioned that some proprietary port is used—could you clarify how this works?

 


Sharath Krishna Karanam
Today 11:44
We were informed earlier (including in the biweekly sync) that password less SSH is not required for the installation. However, during execution, it still prompts for SSH (frontend), indicating SSH dependency. (--install frontend)

image-20260623-114256.png
We also heard that a proprietary port is used. Could you please clarify why SSH is still needed and how this port is involved?

Information provided by Simen:
Hi Ben,

More fun on your ticket (https://dolphinics.atlassian.net/browse/DCS-1540)

Is it possible to identify the connected host using only a direct PCIe cable connection (without IP/network)?

Our mechanism here is to match interface IP-addresses. For network-less hosts that have NICs we've suggested that these are assigned static IP addresses  and for hosts that do not have NICs we've suggested that these are set up with 'dummy' IP-interfaces so that they can still be configued. Havin a working IP-interface is also required for SuperSockets, as it relies on an existing IP-interface to replace the network calls issued.

If no IP-interfaces at all are possible then there are options which may or may not have acceptable downsides;
Custom dishosts.conf files for each node that identifies the node with the DIS_LOCAL etc hostname, or to drop the dishosts.conf file and the Node Manager and to instead call dis_config in /etc/rc.local or similar. Both of these break SuperSockets, but will have the node with NodeId configured after booting.

What error is reported in dis_diag when there is a mismatch between the upgraded kernel and the currently installed ExpressWare version?

Dis_diag relies on connecting to the IRM (dis_Irm) kernel module. This module depends on the lower-level interconnect driver modules (dis_mx.conf) - neither of these modules will load if they are not built to fit the running kernel, and dis_diag will complain that the drivers are not loaded.

Could you explain the technical mechanism behind cluster formation using --install-frontend? Specifically, as discussed in the meeting, it was mentioned that some proprietary port is used—could you clarify how this works?

Suggest you confirm with netstat, but - Network Manager connects to Node Manager on tcp port 3443. I think the Network Manager takes connections from admin/gui tool on tcp port 3444. (edited)