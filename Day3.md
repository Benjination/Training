Picking up where I left off yesterday.
1. After I left last night I realized that I had used scp to move the installer over to the remote nodes, but I never ran the installer. 
2. This morning I used ssh to access each node, and used chmod +x to make the shell scripts executable, then ran the installers. 
3. After running the installers and setting the configuration, I ran dis_diag. There is a warning that one of the nodes is only running x8 instead of x16. They should both be running x16. 
4. I've used dis_diag and  dis_firmwaretool hexpert pci:00:03.1 pfx -i to try and diagnose the health of the connection. I've found that when I log into either location, dis_diag can only find one adapter card at address 08:00.1 ... Also, there is a warning that the lane width is set to x8 instead of x16. 
5. I tried to use dis_config to set the width to x16. This was allowed with no warnings, but the warnings persists. 
6. I have discovered the problem, and it is now fixed. There was a cable unplugged. I did not have access to the hardware, so I would not have seen that it was detached. I've only been accessing the system remotely. 
7. Roy came by to check my progress. I told him I had reinstalled the Dolphin software, configured the nodes, tried setting the width using dis_config, and was in the process of upgrading the eeprom, but that the warning still persists. He told me the system might be missing a cable. We went together to go look at the hardware and it did in fact have a cable that was not connected. 
8. I connected the cable.
9. I returned to my laptop to access the nodes remotely again. I upgraded the eeprom on both nodes, and used  dis_firmwaretool hexpert pci:08:00.1 pfx -R to reset the adapters.
10. I reboot both computers. 
11. I updated both computers with dis_config, then checked with dis_diag and made sure that the systems were running in the active partitions instead of the inactive partitions. 
12. The system was running the inactive partitions, so I reset the cards with  dis_firmwaretool hexpert pci:00:03.1 pfx -R, and then reboot again. 
13. After reboot, the system is running the active partitions at full x16. Everything seems to be working again. 

NOTE: SUCCESS

Questions: 
1. I only found one address at 08:00.1, and it seems like since there are two nodes, they should have two addresses, unless this address represents the connection between the two nodes, and not the nodes themselves. 
2. 