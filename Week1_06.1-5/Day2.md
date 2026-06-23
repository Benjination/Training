This was my first assignment from Roy. He asked me to SSH into a cluster and upgrade the SISCI software. This is what I was able to do.

1. SSH into the remote computer.
2. Use `uname -a` to identify the Linux version.
3. Download the correct version of the SISCI installer to my work laptop.
4. Use `scp` to copy the installer to the remote node.
    Note: `scp <filename> <destinationAddress:/root/>`
5. Run `upgrade_eeprom.sh --update`.
6. Check status with `dis_firmwaretool hexpert pci:08:00.1 pfx -i`.
    Note: Find the node address using `dis_diag`.
7. Reset the card with `dis_firmwaretool hexpert pci:08:00.1 pfx -R`.
8. Reboot the remote node with `reboot`.
9. SSH back into the remote node.
10. Check status again.
     Note: Systems should be running in the active partition, not inactive.
11. Check configuration with `dis_diag -V 9`.
12. Configure the node with `dis_config`.
     Note: Adapter set to 0, node ID set to 004.

Special note: Upon checking the second node, I found they have two different Linux versions.

13. Download a compatible version, copy it to the remote node with `scp`, SSH to the remote node, run `upgrade_eeprom.sh --update`, reset, reboot, configure, and test.

Questions:
1. Did it actually run the installer, or did it run the `.sh` script from the previous installation?
    Answer: No, it had not. I needed to run `chmod +x` on the installer and run it on each node.
2. Are the two nodes connected and able to transfer data between them?
    Answer: Yes. They were transmitting at x8, but have since been repaired to run at the full x16.
3. Should I have upgraded the Linux version to use newer versions of Dolphin and Linux?
    Answer:
