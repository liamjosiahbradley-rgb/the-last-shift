# THE LAST SHIFT: MALL AFTER DARK
## Prototype 0.1 — development kit

This is the first source prototype for the RG36 Pro / ArkOS target.

### What is in this prototype
- A low-resolution, low-poly **3D** mall corridor and security office.
- First-person camera.
- Left stick moves; right stick turns the camera.
- A simple Night Guard that begins pursuing the player when they get close.
- Keyboard controls are included for desktop testing.

### Important status
This folder contains **source code**, not a ready-to-run RG36 Pro binary.
The RG36 Pro target still needs an ARM build and a real-device test. Do not copy
this source ZIP into the PSX folder. The intended destination after a successful
PortMaster build is the device's `ports` folder.

### Build requirements
- SDL2 development headers and library
- ARM hard-float Linux cross compiler for the ArkOS target
- `make`

### Desktop build
On a Linux computer with SDL2 development files:
```sh
gcc -O2 -Wall src/main.c -o lastshift $(sdl2-config --cflags --libs) -lm
./lastshift
```

Keyboard:
- W / Up: forward
- S / Down: backward
- A / D: strafe
- Left / Right: turn
- Escape: quit

Handheld:
- Left analog stick: move / strafe
- Right analog stick: turn
- Start: quit prototype

### Next development milestones
1. Build for the exact ArkOS ARM ABI and test launch on the RG36 Pro.
2. Add proper mall rooms, store entrances, and a real security-office opening.
3. Improve guard navigation, line-of-sight, hiding, flashlight, and sound.
4. Add title screen, game-over/restart, and objective/key progression.
5. Package as a polished PortMaster install.

The renderer is deliberately small and low-resolution to suit the handheld.


## Updated ArkOS build notes
The GitHub Actions workflow now attempts an ARM hard-float build inside
Ubuntu 20.04, which PortMaster documents as the safer glibc baseline for
older firmware such as ArkOS. This is a build attempt, not a verified
RG36 Pro binary. The device model/firmware ABI and runtime libraries still
need confirmation, and the result must be tested on the actual handheld.

PortMaster's documentation says ports can be installed offline by copying
the port ZIP into the firmware's `autoinstall` folder, but this prototype
is not yet a complete PortMaster release package.
