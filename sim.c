#include <stdio.h>
#include <stdint.h>

#define RDP_BASE 0xA4000000
#define RSP_BASE 0xA4080000

// Simulating RDP and RSP memory spaces
uint32_t RDP_MEMORY[0x1000];
uint32_t RSP_MEMORY[0x1000];

void write_rdp(uint32_t address, uint32_t value) {
    uint32_t index = (address - RDP_BASE) / 4;
    if (index < 0x1000) {
        RDP_MEMORY[index] = value;
        printf("RDP Write: Address 0x%08X, Value 0x%08X\n", address, value);
    } else {
        printf("RDP Write Out of Bounds!\n");
    }
}

void write_rsp(uint32_t address, uint32_t value) {
    uint32_t index = (address - RSP_BASE) / 4;
    if (index < 0x1000) {
        RSP_MEMORY[index] = value;
        printf("RSP Write: Address 0x%08X, Value 0x%08X\n", address, value);
    } else {
        printf("RSP Write Out of Bounds!\n");
    }
}

uint32_t read_rdp(uint32_t address) {
    uint32_t index = (address - RDP_BASE) / 4;
    if (index < 0x1000) {
        printf("RDP Read: Address 0x%08X, Value 0x%08X\n", address, RDP_MEMORY[index]);
        return RDP_MEMORY[index];
    }
    printf("RDP Read Out of Bounds!\n");
    return 0xFFFFFFFF;
}

uint32_t read_rsp(uint32_t address) {
    uint32_t index = (address - RSP_BASE) / 4;
    if (index < 0x1000) {
        printf("RSP Read: Address 0x%08X, Value 0x%08X\n", address, RSP_MEMORY[index]);
        return RSP_MEMORY[index];
    }
    printf("RSP Read Out of Bounds!\n");
    return 0xFFFFFFFF;
}

int main() {
    // Example simulation of writing and reading from RDP and RSP
    write_rdp(RDP_BASE + 0x04, 0x12345678);
    read_rdp(RDP_BASE + 0x04);

    write_rsp(RSP_BASE + 0x08, 0x87654321);
    read_rsp(RSP_BASE + 0x08);

    return 0;
}
// [By ChatGPT 
