# CapriceESP32
(A version for the Odroid GO Hardware)
# Introduction
At the time the Amstrad CPC came out in the 80th, I was fascinated on what lies within the creativity of computer games. It was my first computer where me and my brothers spent days and nights playing games. What a perfect time looking back from now. It was also the time I got fascinated by how a computer works, I have started to learn digital science and computer programming, finding hacks and game cheats in first place. Well, there is always a way to start with :-)

Now, sever centuries later, I came in touch with the Odroid GO by searching for ESP32 programming and evaluation board references. I liked already the ESP8266 for doing some nice IOT projects on my own for home automation purpose. Then, after purchasing an Odroid GO, I have started to upload some nice EMUs checking some games.

Well, then I was looking for a Amstrad CPC EMU for the Odroid GO. As far as I know, there is no public version available. I was caught by the idea playing my favorite games I was addicted to in my youth 30 years later on this tiny ESP32. How could such a big machine fit to half a square inch of space?

Then I started with the end of mind. Searching for a simulator platform saving me some development time capable to develop the wrapper on a decent PC. On the Amstrad CPC, I searched for a good starting point to develope from. I decided to go for the Caprice Palm OS version. It was the most flexible and most decent version of the original Caprice32 version. Still a mystery to me how to develop an EMU. Well done Ulrich Doewich and Frederic Coste! Every thing else is history.

Here we go, a first pre-release of the ESP32, Odroid GO version of the Caprice CPC engine is made available here. What needs to be done to get going is described below.

# Preparing the SD Card

whatever_sd_card_name_you_take/
- /cpc/dsk  :: Search path for DISK images
- /cpc/cps  :: Default snapshot path (not used yet)
- /cpc/scr  :: Default screenshot path (not used yet)
- /cpc/cmf  :: Holding the cheat (cheat mode format) files
- /cpc/kmf  :: Default keymap file path

It is very important to setup the SD card accordingly. Without that it won’t work. This configuration is uses to be standard compliant to Odroid GO Emulator setup. In general used to keep multiple EMUs organized on one single SD card. The /cpc/dsk path may contain sub-directories. Please not that sub-directories cannot be stepped into at the moment. So please put all your *.dsk disk images into the root directory of: /cpc/dsk.

# Compile from source

Normally, there is no need to compile from source. The repository contains two executable versions.  One to use with :>make flash (monitor), the other as a FW image which can be loaded through the Odroid GO Firmware process. As platform, the esp_idf version 3.2 is uses. It is basically the standard Odroid GO development platform. I have added also Cmake support for the actual esp_idf version 4.x for testing purpose. It should compile, however, I have stopped development on 4.x because no performance improvements determined.

# Running the Emulator

After compile and flash, or FW upload, you should see the Amstrad/Schneider CPC 6128 start screen. How to navigate is described in the usermanual.pdf document.

Have Fun!
