# CHIP-8 Emulator

My take on a Chip8 emulator / interpreter written in C, using SDL2. CHIP-8 is an interpreted programming language, developed by Joseph Weisbecker. It was initially used on the COSMAC VIP and Telmac 1800 8-bit microcomputers in the mid-1970s. CHIP-8 programs are run on a CHIP-8 virtual machine. It was made to allow video games to be more easily programmed for said computers.

### Technical Specifications
Chip8 had 4kB memory, however, the interpreter occuppied the first 512 bytes, so all programs written for the system started on the 512th memory position.

It also features 16 1-Byte registers, named V0 to VF and a 2-Byte special register named I, which is an address register.

### Usage
Compile the project using the Makefile.
Then to run a game:
```
./CHIP-8 <filepath>
```

### Input
The Chip8 features a hex keyboard as shown below.

|   	|   	|  |  |
|---	|---	|---	|	---	|
|   1	|  2 	| 3 | C |
|   4	|  5 	| 6 | D |
|   7 |  8 	| 9 | E |
|   A	|  0 	| B | F |

### Features

- [x] Compatible with most ROMS
- [x] Audio fully implemented
- [ ] User graphical interface
- [ ] Operations / registers debugging
- [ ] Save / load state system

### Testing

- [BC_test](https://github.com/stianeklund/chip8/blob/master/roms/BC_test.ch8)
<p>
  <img src="https://i.imgur.com/xizNJUt.png" />
</p>

- [Corax89_test](https://github.com/corax89/chip8-test-rom)
<p>
  <img src="https://i.imgur.com/14VtAAg.png" />
</p>

- [Skosulor_test](https://github.com/Skosulor/c8int/tree/master/test)
<p>
  <img src="https://i.imgur.com/09J9vCX.png" />
</p>

### Credits

- [http://en.wikipedia.org/wiki/CHIP-8](http://en.wikipedia.org/wiki/CHIP-8)
- [http://devernay.free.fr/hacks/chip8/C8TECH10.HTM](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [http://www.codeslinger.co.uk/pages/projects/chip8.html](http://www.codeslinger.co.uk/pages/projects/chip8.html)
