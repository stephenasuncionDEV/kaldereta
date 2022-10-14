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

Install [Windows Driver Kit](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)

Load .sys file with [KDMapper](https://github.com/TheCruZ/kdmapper)

## Credits

I was able to create this driver with the help of a bunch of sources. If you see your source-code in the codebase of this repository, contact me to properly credit you.

* https://www.youtube.com/c/NullTerminator
* https://j00ru.vexillium.org/syscalls/win32k/64/
* https://github.com/Zer0Mem0ry/ManualMap
* https://github.com/nbqofficial/norsefire

## License

[GNU General Public License v3.0](https://github.com/stephenasuncionDEV/kaldereta/blob/main/LICENSE)
