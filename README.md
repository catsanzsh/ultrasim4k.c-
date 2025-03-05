# ultrasim4k.c-
1.0
# UltraSim4K.c- 🚀
A low-level Nintendo 64 **RDP/RSP memory simulation** designed for debugging, reverse-engineering, and potential integration with UltraSim-based emulation projects.

## 🎮 What is UltraSim4K.c-?
UltraSim4K.c- is a **simulation of the N64's Reality Display Processor (RDP) and Reality Signal Processor (RSP) memory spaces**. This allows you to:
- Emulate memory interactions at the hardware level.
- Read and write to simulated RDP/RSP registers.
- Debug how RDP/RSP operations work in a controlled environment.
- Lay the foundation for deeper N64 hardware emulation.

## 🛠️ Features
- **Memory-mapped simulation** of RDP and RSP.
- **Read/write functions** to interact with RDP and RSP memory.
- **Bounds checking** to prevent invalid memory access.
- **Logging system** that prints memory operations for debugging.
- **Extensible** for integration with N64 emulator projects.

## 🖥️ How It Works
### Memory Layout:
| Component | Base Address |
|-----------|-------------|
| RDP (Reality Display Processor) | `0xA4000000` |
| RSP (Reality Signal Processor) | `0xA4080000` |

### Example Usage:
```c
write_rdp(RDP_BASE + 0x04, 0x12345678);
read_rdp(RDP_BASE + 0x04);

write_rsp(RSP_BASE + 0x08, 0x87654321);
read_rsp(RSP_BASE + 0x08);
