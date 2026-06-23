Monday, monday, monday... I think I went a little too hard on Sunday Funday, but I did have a really fun day. A coworker invited me for dinner and I got to meet his family. We had many beers.. Anyway, back to work. 

1. This morning first thing I started working with the Dolphin Demo  that I made before. I got it working with the remote nodes and adapted it to work with two nodes instead of three. This does limit the data output though because it only shows one link instead of two. 
2. I set up meetings on Thur and Fri with Marius to try and work out my laptop issues. If I can't ssh into the remote nodes, then that is going to make my life a lot more difficult. I will need to set up a two or three node cluster in dallas just in case. 
3. Fujitsu responded this morning. They are having an issue where both nodes are beign assigned to node 4. It's most likely a mismatch between the dishosts file on each node. I told him to scp the file from node 1 to node 2 so that they would be the same. I hope that fixes it. 
4. I found a switch and connected it to my two node system. Roy showed me how to configure the switch. We use a website: https://mxs824-541.lab.dolphinics.no/login.html. The model of the switch is the first part, and the second part is the ID number. From that website you can adjust the settings of the switch. Unfortunatly, I won\t be able to see some of the visual indicators on the switch remotely. That might be a hassle. The settings for my two node system is going to be NTB, single, 6 x16... That should allow the two nodes to find each other as if they were connected directly. 
5. I responded to Fujitsu. They were still trying to install frontend after I gave them an easier path to install. 
6. I refined the Fujitsu response so we only request the minimum required diagnostic info first: dishosts.conf from both nodes, then dis_diag if needed. I also corrected the path guidance so the file target is /etc/dis/dishosts.conf and not an install-directory path.
7. I confirmed that dis_config-only changes are not persistent for this issue, and that dis_mkconf/dishosts.conf remains the persistent source for node identity mapping.
8. I troubleshot switch access for mxs824-541.lab.dolphinics.no and confirmed the hostname does not resolve from my network. Public resolver check also showed NXDOMAIN, so access appears internal-only.
9. I scanned local neighbor/ARP data for switch MAC 00:12:a5:01:02:1d and did a subnet sweep on 192.168.41.0/24, but did not find that MAC on my local segment.
10. I verified that 192.168.1.210 is not reachable from my current network context (I am on 192.168.41.x), so I likely need VPN, the correct VLAN, or direct static-IP access to reach switch management.
11. I documented what is needed to use the VPN profile screen: remote gateway, auth method, optional cert requirements, and route access to the management subnet.

End of Day wrap-up:
1. Customer status: Awaiting Fujitsu response after sending the minimal-data troubleshooting request.
2. Current blocker: No confirmed network path to switch management from my current segment (likely requires VPN, correct VLAN, or route updates).
3. Evidence captured today:
	- Hostname for switch management page did not resolve from current network context.
	- Target switch MAC was not visible on my local segment after neighbor/ARP checks.
	- 192.168.1.210 was not reachable from my current subnet.
4. First tasks for Day 12:
	- Get VPN gateway/auth details and confirm whether cert is required.
	- Confirm route/access policy to 192.168.1.0/24 from my laptop.
	- Retry switch UI access and capture final result/logs.
12. I was able to access the website for the switch that I added to the system. The throughput was halved when I connected the two nodes through the switch and I couldn't figure out why. I noticed that the switch wsa defaulting to Gen 3, and there wasn't an option to change that setting. I realized that the switch itself is Gen 3. So even though the adapaters are both gen 4, the switch throttles them down to gen 3 speeds, so an old switch won't work with new adapters. At least not at the new adapters speeds. 
13. Roy just brought to my attention that when he ran dmesg on my cluster, there are a lot of errors. The problem is that I had not updated the dishosts file after changing the configuration and adding the switch. The issue is that the dishosts file has the standard set to gen 4, but since the switch is only gen 3, the system will continuosly try to repair the speed. It can't so it repeats errors. I need to clear the dmesg each time I make a change, and run dis_mkconf to recreate the dishosts file. 
14. ps -aux will show all processes being run on the fabric. 
