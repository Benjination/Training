Picking up where I left off yesterday.

1. After I left last night, I realized that I had used `scp` to move the installer to the remote nodes, but I never ran the installer.
2. This morning I used `ssh` to access each node, then used `chmod +x` to make the shell scripts executable, and ran the installers.
3. After running the installers and setting the configuration, I ran `dis_diag`. There was a warning that one node was only running x8 instead of x16. They should both be running x16.
4. I used `dis_diag` and `dis_firmwaretool hexpert pci:00:03.1 pfx -i` to diagnose the connection health. I found that when I log into either location, `dis_diag` can only find one adapter card at address `08:00.1`. There was also a warning that the lane width was set to x8 instead of x16.
5. I tried using `dis_config` to set the width to x16. This was allowed with no warnings, but the warning persisted.
6. I discovered the problem, and it is now fixed. A cable was unplugged. I did not have access to the hardware, so I would not have seen that it was detached because I had only been accessing the system remotely.
7. Roy came by to check my progress. I told him I had reinstalled the Dolphin software, configured the nodes, tried setting the width using `dis_config`, and was in the process of upgrading the EEPROM, but the warning still persisted. He said the system might be missing a cable. We went together to check the hardware and found a cable that was not connected.
8. I connected the cable.
9. I returned to my laptop to access the nodes remotely again. I upgraded the EEPROM on both nodes and used `dis_firmwaretool hexpert pci:08:00.1 pfx -R` to reset the adapters.
10. I rebooted both computers.
11. I updated both computers with `dis_config`, then checked with `dis_diag` and made sure the systems were running in the active partitions instead of the inactive partitions.
12. The systems were running in inactive partitions, so I reset the cards with `dis_firmwaretool hexpert pci:00:03.1 pfx -R`, then rebooted again.
13. After reboot, the systems were running in active partitions at full x16. Everything seems to be working again.

Note: Success.

Questions:
1. I only found one address at `08:00.1`, and it seems like since there are two nodes, they should have two addresses, unless this address represents the connection between the two nodes and not the nodes themselves.

14. Seems like the driver that installs with update_eeprom does not match what it is supposed to match. The newest expressware is 5.25.2, but the driver listed in dis_diag is 5.26. Then the eeprom version is also not right. It's 14, but it should be 15. 
15. Roy showed me a few steps to take on remote node 5, so I am repeating the steps on remote node 6. 
16. First step, running dis_services status. This lists all services and whether they are running. 
17. There is a service called nodemgr that will create a dishosts file for me. I turn this service on with the command "nodemgr-setup -i --start"
18. The service is confirmed running in dis_services status. 
19. Before running the nodemgr, dishosts lists localhost as node 4, and there are three physical adapters connected. a0, a1, a2... Link width is 4, link speed is 3. 
20. Running the nodemgr service feature that creates a dishost file: dis_services restart (This stops each dis_service and brings them back up in the correct order)
21. The nodemgr is not creating or modifying the dishosts file like it should be, so I'm going to adjust my plan. 

The new plan:
22. Update the Linux versions on both nodes. 
23. Find the Dolphin installer that will work for the new matching Linux distro.

24. Test and configure. 