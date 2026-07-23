

<div align="center">
	<h1>Cclicker</h1>
</div>

<div align="center">
	<i>A minimal cross-platform autoclicker engine</i>
</div>


**Cclicker** was made to serve as an engine for your autoclicker GUI on multiple systems such as Windows and Linux using the performance and portability of the C language.

---

## Supported Platforms

| Platform | Status         |
| -------- | :------------- |
| Windows  | ✅ Supported   |
| Linux    | ✅ Supported   |
| macOS    | ❌ Unsupported |

---

## Dependencies

To build **Cclicker**, you need:

### Required

- **C compiler**
    - GCC or Clang (Linux)
    - MinGW / MSYS2 / MSVC (Windows)
- **CMake** (>= 3.10)
- **Git**

---

### Linux (Debian / Ubuntu)

```bash
sudo apt update
sudo apt install build-essential cmake git
```

---

### Windows (PowerShell)

```bash
winget install --id Git.Git -e
winget install --id Kitware.CMake -e
winget install --id MSYS2.MSYS2 -e
```

---

## Build & Run


### Linux

```bash
git clone git@github.com:gabk9/Cclicker.git
cd Cclicker

mkdir build
cd build

cmake ..
cmake --build .

./Cclicker
```

---

### Windows (PowerShell - MSVC)

```bash
git clone git@github.com:gabk9/Cclicker.git
cd Cclicker

mkdir build
cd build

cmake ..
cmake --build .

.\Cclicker
```

---

## Usage

```bash
./Cclicker [OPTIONS...]
```


### Options

| Option | Description |
| -------------- | ------------------------------- |
| `--duration=x` | Total execution time in seconds |
| `--delay=x` | Delay between clicks in seconds |
| `--cps=x` | Target clicks per second |

`--delay` and `--cps` are mutually exclusive.
### Examples

```bash
./Cclicker --duration=10 --delay=0.1
./Cclicker --duration=10 --cps=100
```

---

## Architecture

Cclicker was designed as an input simulation engine, keeping the platform-specific implementation separated from the core logic.

```
Cclicker
│
├── Core
│   ├── Timing
│   ├── Clicker scheduler
│   └── Statistics
│
├── Windows Backend
│   └── SendInput API
│
└── Linux Backend
    └── uinput subsystem
```

This design allows a graphical interface to be built on top of Cclicker without needing to handle operating-system-specific input APIs.

---

## Linux Notes

On Linux, Cclicker uses the kernel `uinput` interface to create a virtual mouse device.

Configure your system permissions to allow access to `/dev/uinput`.

If permission is denied, add your user to the appropriate group:
 
```
sudo usermod -aG input $USER
```

Then log out and log in again.

---

## Performance

Cclicker uses native operating system APIs for input generation.

Performance depends on:

- Operating system scheduler
- Timer resolution
- Desktop environment/compositor
- System load

The engine is capable of generating hundreds or thousands of clicks per second depending on the system configuration.

---

## Future Plans

- [ ] Multiple mouse buttons
- [ ] macOS support

---
## License

This project is licensed under the MIT License.

---

## Author

Developed by **gabk9**.
