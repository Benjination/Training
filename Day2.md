This was my first assignment from Roy. He asked that I ssh into a cluster and upgrade the SISCI software that they have. This is what i was able to do. 

1. SSH into the remote computer
2. Use uname -a to identify the Linux version
3. Download the correct version of SISCI installer to Work Laptop
4. scp installer over to remote node
NOTE: scp <filename> <destinationAddress:/root/>
5. run "upgrade_eeprom.sh --update"
6. check status with "dis_firmwaretool hexpert pci:08:00.1 pfx -i"
NOTE: find node address using dis_diag
7. Reset card with "dis_firmwaretool hexpert pci:08:00.1 pfx -R"
8. Reboot remote node "reboot"
9. SSH back into remote node
10. Check status again. 
NOTE: Systems should be running in Active partition, not inactive. 
11. Check configuration with "dis_diag -V 9"
12. Configure node with dis_config
NOTE: Adapter set to 0, node ID set to 004

Special NOTE: Upon checking the second node, I found they have two different versions of Linux

13. Downloaded compatible version, scp to remote node, ssh to remote, upgrade_eeprom.sh --update, Reset, reboot, configure, test

QUESTIONS: 
1. Did it actually run the installer, or did it run the .sh script from the previous installation?
    Answer: No, it had not. I needed to chmod +x the installer and run it on each node. 
2. Are the two nodes connected and able to transfer data between them?
    Answer: Yes, they were transmitting at x8, but have since been repaired to run at the full x16.
3. Should I have upgraded the linux version to use newer versions of Dolphin and Linux?
    Answer: 
