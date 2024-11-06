    <h1>Nucleus ESP: An Affordable Hacking Tool</h1>
    <p>Nucleus ESP is a budget-friendly, yellow gadget for various hacking applications. It can transmit in the Sub-GHz RF range and has additional potential, which I plan to test further.</p>

    <h2>Project Overview</h2>
    <p>I modified the device by desoldering the RGB diode to use it as GPIO, and repurposed the LCD backlight pin for the CC1101 chip select (CS) function. This is my first project in C, which I began in early summer with some background in PHP and a bit of FPGA tinkering. I'm learning as I go, figuring things out by experimenting.</p>
    <p><strong>Status</strong>: Work in Progress (WIP)<br>
    Please go easy on it; it's still evolving!</p>

    <h2>Features Implemented</h2>
    <p>With the help of my Software-Defined Radio (SDR), I implemented a RAW protocol for replay through a simple bit-bang method, upgrading it to more precise timing later (previously, I sampled every 430 microseconds).</p>
    <p>Now, I’m planning to reintegrate RCswitch and the <code>.sub</code> (Flipper) file player from the old GUI, as they haven’t been added to the new interface yet.</p>
    <p><strong>Last Stable Version</strong>: Compiled into bin files. Occasionally, things may break in new versions, so for consistent testing, use this stable version.</p>

    <h3>Last commit status</h3>
    <ul>
        <li><strong>Updated to new LVGL</strong>: This was not so easy</li>
        <li><strong>Touch drivers</strong>: I remembered why i use bitbang drivers while moving to new LVGL - drivers included, should compile from source</li>
        <li><strong>New filebrowser</strong>: Unlike previous one, this one is usable, even with bare hand.</li>
        <li><strong>SubGhz files transmission</strong>: When system start transmiting, it dont show it ended but most raw timings files will transmit</li>
    </ul>

    <h3>Priority list</h3>
    <ul>
        <li>Saving os subGhz files</li>
        <li>Parsing of custom protocol data from subGhz files, and writing them to CC1101 registers to make those work</li>
    </ul>

    <h3>Current Functionalities</h3>
    <ul>
        <li><strong>CC1101 RAW Replay</strong>: Now operates similarly to the Flipper. - saving not work, willl repair have hight prioritz</li>
        <li><strong><code>.sub</code> Files Player</strong>: Supports RAW files, tested with AM270 and AM650.</li>
        <li><strong>Tesla Charger Opener</strong>:- now deactivated, will come back, not important</li>
        <li><strong>Large <code>.sub</code> File Support</strong>: Handles virtually unlimited file sizes, tested with a 3MB "Came_top_brute.sub" file.</li>
        <li><strong>Brute Force Attack on 8-bit RF Codes</strong>: Experimental feature for the M1E IC by MOSDESIGN SEMICONDUCTOR CORP. <a href="https://www.cika.com/soporte/Information/Semiconductores/CIencoder-decoder/M1E-MOSDESIGN.pdf">M1E Datasheet</a></li>
    </ul>

    <h3>Planned Features</h3>
    <ul>
        <li>Bluetooth Spam</li>
        <li>Sour Apple</li>
        <li>Wi-Fi Deauther</li>
        <li>Bad USB</li>
        <li>2.4GHz Support</li>
        <li>MouseJack Attack</li>
    </ul>

    <h2>Timing and Transmission Quality</h2>
    <p>The timing tolerance is around 1-5 microseconds. Transmission quality from <code>.sub</code> files is as follows:</p>

    <h3>AM270</h3>
    <img src="https://github.com/GthiN89/NucleusESP32/blob/main/images/AM270.PNG" alt="AM270 Transmission">

    <h3>AM650</h3>
    <img src="https://github.com/GthiN89/NucleusESP32/blob/main/images/AM650.PNG" alt="AM650 Transmission">

    <h3>Capture Times</h3>
    <img src="https://github.com/GthiN89/NucleusESP32/blob/main/images/capture%20times.bmp" alt="Capture Times">

    <h2>Hardware</h2>
    <ul>
        <li>Cheap Yellow Display (CYD)</li>
        <li>ESP32-2432S028R</li>
        <li>ESP32-2432S028Rv2</li>
    </ul>

    <p>Happy hacking!</p>
