Yesterday before leaving we got ssh working on genfi-5. This morning I'm starting with the software install
    1. ssh is still working. That's good. I had to reset the relationships between the computers using keygen commands:
        ssh-keygen -R genfi-5.lab.dolphinics.no
        ssh-keygen -R genfi-5
        ssh-keygen -R <genfi-5-ip>
    2. I used scp to send the Dolphin installer I used on genfi-6 over to genfi-5. 
    3. After attempting the install several times, the installer asked if all the leds were green on the card. I already knew they weren't, but I went and looked. The leds on the card in genfi-5 have been off since early on in this project, and they were still off. 
    4. I changed the card for a different MXH930 card. The LEDs were all green, but the install still does not work. 
    5. I'm going to try to find someone to do the install while I watch so that I can make sure I'm not doing it wrong. 
    6. I captured a clean dis_diag result on genfi-5 after reinstall/recovery work. The local adapter is NodeId 4, link state is x16 Gen4, and topology detects both nodes 0004 and 0008 with valid sessions. Test result passed with 0 warnings and 0 errors.
    7. I then confirmed node 8 on genfi-6. dis_diag now shows version 5.25.2, local NodeId 8, x16 Gen4 link, both nodes 0004 and 0008 visible, and valid sessions for both. Test result also passed with 0 warnings and 0 errors.
    8. I was able to run the DMA_Bench test successfully. 
    9. I just went over the different tests that the devs use to optomize a system and find which settings get them optimal performance. This is truly exciting!!! I think I finally understand my job. 
    10. I ran the DMA_Bench -rn <Node> -client/server optional -size ((1024*1024*2))
    11. SCIBench to measure latency
    12. SCIPP for a different look at latency
    13. SCIMemcopyBench uses PIO
    14. Intr_bench uses interrupts. 
    15. I'm pretty sure my job will be to diagnose customer systems using these tools to find exactly which settings allow their unique clusters the best performance. I can use this to DIAGOSE!!!
    16. 