Nucleus ESP is cheap yellow hack toy.</br>
It can ring some SubGhz RF bells, and maybe more, i just need to test it.</br>
I desoldered RGB diod, to make it use as GPIO, and i use LCD Backlight pin as CC1101 CS...</br>
It is my first project in c, which i started at the start of the summer, with some experience with PHP, and little of FPGA tindering.</br>
I learn as i go, just by doing things.</br>
Its WIP so please dont be hars to it.</br>
I got SDR, so i managed to implement RAW protocol for replay by simle bitBang, and upgrated it later to acctual timing (i was kinda sampling every 430 micros before).</br>
Now i can put back RCswitch and .sub (flipper) file player, which are on old GUI, and are not implemented in to the new one.</br>
Happy Hacking!</br>
</br>
have:</br>
-C1101 raw replay - now work more similar to flipper</br>
-*.sub files player-raw files does work, tested for AM270 and AM650</br>
-Tesla charger open - this works great</br>
</br>
planned:
-BL spam</br>
-Sour apple</br>
-Deauther</br>
-Bad USB</br>
-2.4ghz</br>
-mousejack</br>
</br>
Timing tollerance is in about 1-5 micros, "quality" of transmision from .sub is as shown:</br>
</br>
AM270:</br>
<img src="https://github.com/GthiN89/NucleusESP32/blob/main/images/AM270.PNG"></br>
</br>
AM650:</br>
<img src="https://github.com/GthiN89/NucleusESP32/blob/main/images/AM650.PNG">
</br>
<img src="https://github.com/GthiN89/NucleusESP32/blob/main/images/capture times.bmp"></br>
</br>
Cheap yellow display, CYD, ESP32-2432S028R, ESP32-2432S028Rv2</br>
</br>