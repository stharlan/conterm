# conterm
console based terminal emulator with support for modem and telnet

A pandemic is a good time to dig out the old modem and dial up a BBS.

A few days ago, somthing got me thinking about Bulletin Board Systems from the 1980s. I had two modems in storage and I decided to get them out and see if any BBSs were still in operation. Heres my experience.

There are some BBSs around. Google found this:

https://www.telnetbbsguide.com/connection/dial-up/

Time to get out the modems. Once of the modems is a softmodem (a winmdoem). Its a Zoom model 2990 from the 1990s, I think. I hooked it up to my Windows 10 machine and no drivers were found. All the drivers I found online are executables that wont run under Windows 10, or in XP mode on Windows 10. I knew it was a long-shot. 

I have a disk for Windows 2000 (supported) and I installed it in Virtual Box. After installing the driver, and some fiddling with Virtual Box, it was recognized on Win2K. I had to choose a driver on the host machine (Windows 10), and, I think I chose a generic USB device, just to get Virtual Box to see it. I was able to dial up one of the BBSs with HyperTerminal, but, it just wasnt working quite right.

I decided to try it on Linux, because, I knew some winmodems are recognizable on Linux. It didnt recognize it after plugging it in, so, I searched for some Linux drivers. The system is Ubuntu 18. All the web sites with drivers specify them by chip numbers, so, I had to open the modem case to find out what chips it had. Theyre all Conextant chips, but, none of the drivers I found supported the chips on the modem. The Zoom was not going to work. I was never a fan of the winmodems.

Fortunately, as some point many, many years ago, I bought a US Robotics modem 5686. They still go for around $100 online, so, it  has held its value well. But, I didnt have a serial port on my new modem laptop, nor a USB to Serial cable. So, I went to Amazon and bought a StarTech USB to RS232 DB9/DB23 Serial Adapter cable. Its got Prolific chips in it which are said to be widely compatible.

Two days later I got my cable and hooked it up to my laptop. It was recognized immediately. I hooked up my USR modem and looked for a terminal emulator that would support a modem. There are several out there. Most cost money and the free ones are quirky and/or buggy (or maybe I just dont know how to work them).

AlphaCom works pretty well. I was able to dial up a BBS and get a login page. I wasnt prepared to go beyond that yet. The license is cheap, and, you get to try it free for 30 days (I think). However, Im not in a position to go buying new software licenses right now. Theres others, but, the licensing costs go up from AlphaCom (which is very cheap, by the way).

So, being a developer, I thought "I can develop something". And, thats where this started.
