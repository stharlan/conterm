# conterm
console based terminal emulator with support for modem and telnet

A pandemic is a good time to dig out the old modem and dial up a BBS.

## The Beginning

A few days ago, somthing got me thinking about Bulletin Board Systems from the 1980s. I had two modems in storage and I decided to get them out and see if any BBSs were still in operation. Heres my experience.

There are some BBSs around. Google found this:

https://www.telnetbbsguide.com/connection/dial-up/

## The Modem

Time to get out the modems. Once of the modems is a softmodem (a winmdoem). Its a Zoom model 2990 from the 1990s, I think. I hooked it up to my Windows 10 machine and no drivers were found. All the drivers I found online are executables that wont run under Windows 10, or in XP mode on Windows 10. I knew it was a long-shot. 

I have a disk for Windows 2000 (supported) and I installed it in Virtual Box. After installing the driver, and some fiddling with Virtual Box, it was recognized on Win2K. I had to choose a driver on the host machine (Windows 10), and, I think I chose a generic USB device, just to get Virtual Box to see it. I was able to dial up one of the BBSs with HyperTerminal, but, it just wasnt working quite right.

I decided to try it on Linux, because, I knew some winmodems are recognizable on Linux. It didnt recognize it after plugging it in, so, I searched for some Linux drivers. The system is Ubuntu 18. All the web sites with drivers specify them by chip numbers, so, I had to open the modem case to find out what chips it had. Theyre all Conextant chips, but, none of the drivers I found supported the chips on the modem. The Zoom was not going to work. I was never a fan of the winmodems.

Fortunately, as some point many, many years ago, I bought a US Robotics modem 5686. They still go for around $100 online, so, it  has held its value well. But, I didnt have a serial port on my new modem laptop, nor a USB to Serial cable. So, I went to Amazon and bought a StarTech USB to RS232 DB9/DB23 Serial Adapter cable. Its got Prolific chips in it which are said to be widely compatible.

Two days later I got my cable and hooked it up to my laptop. It was recognized immediately. I hooked up my USR modem and looked for a terminal emulator that would support a modem. There are several out there. Most cost money and the free ones are quirky and/or buggy (or maybe I just dont know how to work them).

## The Terminal Emulator

AlphaCom works pretty well. I was able to dial up a BBS and get a login page. I wasnt prepared to go beyond that yet. The license is cheap, and, you get to try it free for 30 days (I think). However, Im not in a position to go buying new software licenses right now. Theres others, but, the licensing costs go up from AlphaCom (which is very cheap, by the way).

So, being a developer, I thought "I can develop something". And, thats where this started.

## The Project

### The Basics

First I had to find out how to control the modem. Having done some serial programming with a GPS device (back in the days), I knew the basics. Because I work in dotnet now, I tried to start with dotnet core. It does have serial port support, but, a little Googling indicates that developers are less than happy with the API. You can find the AT command set for this modem online. I tried some sample code, and, was able to get the modem to respond. But, I wanted tighter control over the communication with the modem, and, dotnet core wasnt going to work well.

So, I went to C++. I quickly found that DOTNET was hiding some of the initialization steps for me (it was only trying to help). After realizing I had to configure the serial port (baud rate, parity, etc), I was able to send some commands and get it to dial.

There is surprisingly little "on Google" that describes modem communications with BBSs. I found SynchroNet code, and, I was going to wade through it to see how it works, but, Im not that patient. I may have to eventually, but, for now, Im going to use trial and error. I did install it locally so I could use it as a testing mechanism via Telnet, but, Im not there yet.

So, I can communicate with the modem, and control it. Controlling hardware is an empowering thing. I send it commands to connect. But, once connected, how do I let it know what I send is a command, and, what needs to be forwarded on to the BBS? After some intense Googling, I found that the modem switches from command mode to data mode after it has connected. Brilliant! And, to get it back to command mode, theres a special string to send. Awsome.

Next question - do I send chunks of data, or, one character-at-a-time? When I was connected to one of the BBSs, I was able to determine that it was responding to single characters, so, the terminal program must be sending one-at-a-time. Seems inefficient, but, this technology was from the 80s. Ill start with the idea that it will send one-character-at-a-time.

### The Architecture

The API calls that control serial communication all work asynchronously, so, I decided to use IO Completion ports (IOCP) for the IO. I want to handle serial modem communication as well as telnet. The IO APIs for both can be handled by IOCP.

I started going directly for the modem connection, but, realized I didnt have a good test environment, unless I wanted to call a BBS and hang up 50 times a day.  So, I installed Synchronet BBS and started coding a telnet client first. Im hoping coding the modem client will be easier once I get that done. And, Im very close. I can connect, get the starting page and interact with the BBS. I had to learn about telnet command protocol. The BBS sends some telnet commands to the client, mostly just letting the client know what its going to do. It will default to a dumb terminal unless negotiated otherwise. It also sends ANSI escape codes which I have contemplated parsing and executing, but, seems like a bigger task than I want to handle right now.

There are no functions in the C runtime to get a character from the keyboard asynchronously (at least none I wanted to use). I found a couple of possible solutions on the internet, but, opted to code my own with the windows API. Using WaitForSingleObject on STDIN, GetNumberOfConsoleInputEvents and ReadConsoleInput, I was able to craft a thread that detects keydown events and sends the characters to my client class, which then sends across the "wire" to the server (telnet, serial modem, etc). Works very well. This is all console, of course - no windows.

