<p align="center">
    <img src='./preview.png' alt='Kaldereta Preview'/>
</p>

<p align="center">
    Unsigned Kernel Mode Driver that does memory modifications
</p>

## Features

<ul>
    <li>get process id</li>
    <li>get base address of an image</li>
    <li>change protection of a memory region</li>
    <li>allocate memory region</li>
    <li>free memory</li>
    <li>read/write from address</li>
    <li>read/write from address with offset</li>
    <li>read from memory to buffer</li>
    <li>write to memory from buffer</li>
    <li>simulate mouse events</li>
    <li>simulate keyboard events</li>
    <li>pattern scan</li>
    <li>manual map x64</li>
</ul>

## Setup

with Visual Studio

Install [Windows Driver Kit](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)

Create `Kernel Mode Drive, Empty (KMDF) project` with the following configuration properties:

|General|C/C++|Linker|Driver Settings|Inf2Cat|Driver Signing|
|-------|-----|------|---------------|-------|--------------|
|Configuration Type: Driver|Additional Include Directories: PATH_TO_WINDOW_KITS_FOLDER ex: `C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.19041.0\um`|Entry Point: Driver Entry|Target OS Version: Windows 10 or higher|Run Inf2Cat: No|Sign Mode: Off|
|Platform Toolset: WidnowsKernelModeDriver10.0|Security Check: Disable Security Check||Target Platform: Universal|
||Spectre Mitigation: Disabled||Type of driver: KMDF|
||Teat Warnings as Errors: No|

Once you've buit the driver, Load .sys file with [KDMapper](https://github.com/TheCruZ/kdmapper).

## Test

To see logs from the driver itself, download [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview) from microsoft.

## Credits

I was able to create this driver with the help of a bunch of sources. If you see your source-code in the codebase of this repository, contact me to properly credit you.

* https://www.youtube.com/c/NullTerminator
* https://j00ru.vexillium.org/syscalls/win32k/64/
* https://github.com/Zer0Mem0ry/ManualMap
* https://github.com/nbqofficial/norsefire

## License

[GNU General Public License v3.0](https://github.com/stephenasuncionDEV/kaldereta/blob/main/LICENSE)
