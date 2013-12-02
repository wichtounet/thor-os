//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "sysinfo.hpp"
#include "console.hpp"

namespace {

void native_cpuid(uint32_t key, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx){
    *eax = key;

    /* ecx is often an input as well as an output. */
    asm volatile("cpuid"
        : "=a" (*eax),
          "=b" (*ebx),
          "=c" (*ecx),
          "=d" (*edx)
        : "0" (*eax), "2" (*ecx));
}

void decode_bytes (int data, int descriptor[16], int *next){
    int i;

    i = *next;
    if (!(data & 0x80000000)) {
        descriptor[i++] =  data & 0x000000FF;
        descriptor[i++] = (data & 0x0000FF00) /      256; // 1 bytes R
        descriptor[i++] = (data & 0x00FF0000) /    65536; // 2 bytes R
        descriptor[i++] = (data & 0xFF000000) / 16777216; // 3 bytes R
        *next = i;
    }
}

void get_cache_info() {
    int next = 0, i = 0;
    int descriptor[256];
    int mem_count;

    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    k_print_line("Cache and TLB:");

    native_cpuid(0, &eax, &ebx, &ecx, &edx);

    if (eax < 2){
        k_print_line("   CPUID(2) not supported");
        return;
    }

    native_cpuid(2, &eax, &ebx, &ecx, &edx);

    mem_count = eax & 0x000000FF;        // 1st byte is the count
    eax &= 0xFFFFFF00;                   // mask off the count

    int* desc = descriptor;

    while ( i < mem_count) {
        decode_bytes(eax, desc, &next);
        decode_bytes(ebx, desc, &next);
        decode_bytes(ecx, desc, &next);
        decode_bytes(edx, desc, &next);

        ++i;

        ecx = i;
        native_cpuid(2, &eax, &ebx, &ecx, &edx);
        desc += 16;
    }

    for (i=0; i< next; i++)     {
        if ( descriptor[i] ==  0x00){
            // NULL descriptor, legal value but no info
            continue;
        }

        if ( descriptor[i] ==  0x01)
            k_print_line("  Instruction TLB ...   4 kb pages, 4-way associative, 32 entries");
        if ( descriptor[i] ==  0x02)
            k_print_line("  Instruction TLB ...   4 Mb pages, 4-way associative, 2 entries");
        if ( descriptor[i] ==  0x03)
            k_print_line("  Data TLB ..........   4 kb pages, 4-way associative, 64 entries");
        if ( descriptor[i] ==  0x04)
            k_print_line("  Data TLB ..........   4 Mb pages, 4-way associative, 8 entries");
        if ( descriptor[i] ==  0x06)
            k_print_line("  L1 instruction cache  8 kb, 4-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x08)
            k_print_line("  L1 instruction cache 16 kb, 4-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x0A)
            k_print_line("  L1 data cache .....   8 kb, 2-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x0B)
            k_print_line("  Instruction TLB ...   4 Mb pages, 4-way associative, 4 entries");
        if ( descriptor[i] ==  0x0C)
            k_print_line("  L1 data cache .....  16 kb, 4-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x22){
            k_print_line("  L3 cache:     512K Bytes, 4-way associative, 64 byte line size, ");
            k_print_line("                       128 byte sector size" );
        }
        if ( descriptor[i] ==  0x23){
            k_print_line("  L3 cache:     1M Bytes, 8-way associative, 64 byte line size, ");
            k_print_line("                       128 byte sector size" );
        }
        if ( descriptor[i] ==  0x25){
            k_print_line("  L3 cache:     2M Bytes, 8-way associative, 64 byte line size, ");
            k_print_line("                       128 byte sector size" );
        }
        if ( descriptor[i] ==  0x29){
            k_print_line("  L3 cache:     4M Bytes, 8-way associative, 64 byte line size, ");
            k_print_line("                       128 byte sector size" );
        }
        if ( descriptor[i] ==  0x2C)
            k_print_line("  1st-level D-cache:   32K Bytes, 8-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x30)
            k_print_line("  1st-level I-cache:   32K Bytes, 8-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x40)
            k_print_line("  No L2 cache OR if there is an L2 cache, then no L3 cache");
        if ( descriptor[i] ==  0x41)
            k_print_line("  L2 cache .......... 128 kb, 4-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x42)
            k_print_line("  L2 cache .......... 256 kb, 4-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x43)
            k_print_line("  L2 cache .......... 512 kb, 4-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x44)
            k_print_line("  L2 cache ..........   1 Mb, 4-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x45)
            k_print_line("  L2 cache ..........   2 Mb, 4-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x46)
            k_print_line("  L3 cache ..........   4 Mb, 4-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x47)
            k_print_line("  L3 cache ..........   8 Mb, 8-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x49)
            k_print_line("  L2 cache ..........   4 Mb, 16-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x50)
            k_print_line("  Instruction TLB ...   4 kb and 2 Mb or 4 Mb pages, 64 entries");
        if ( descriptor[i] ==  0x51)
            k_print_line("  Instruction TLB ...   4 kb and 2 Mb or 4 Mb pages, 128 entries");
        if ( descriptor[i] ==  0x52)
            k_print_line("  Instruction TLB ...   4 kb and 2 Mb or 4 Mb pages, 256 entries");
        if ( descriptor[i] ==  0x56)
            k_print_line("  Data TLB ..........   4 Mb pages, 4-way associative,  16 entries");
        if ( descriptor[i] ==  0x57)
            k_print_line("  Data TLB ..........   4 Kb pages, 4-way associative,  16 entries");
        if ( descriptor[i] ==  0x5B)
            k_print_line("  Data TLB ..........   4 kb and 4 Mb pages,  64 entries");
        if ( descriptor[i] ==  0x5C)
            k_print_line("  Data TLB ..........   4 kb and 4 Mb pages, 128 entries");
        if ( descriptor[i] ==  0x5D)
            k_print_line("  Data TLB ..........   4 kb and 4 Mb pages, 256 entries");
        if ( descriptor[i] ==  0x60)
            k_print_line("  L1 data cache .....  16 kb, 8-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x66)
            k_print_line("  L1 data cache .....   8 kb, 4-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x67)
            k_print_line("  L1 data cache .....  16 kb, 4-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x68)
            k_print_line("  L1 data cache .....  32 kb, 4-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x70)
            k_print_line("  Trace cache ......  12k uop, 8-way associative");
        if ( descriptor[i] ==  0x71)
            k_print_line("  Trace cache ......  16k uop, 8-way associative");
        if ( descriptor[i] ==  0x72)
            k_print_line("  Trace cache ......  32k uop, 8-way associative");
        if ( descriptor[i] ==  0x78)
            k_print_line("  L2 cache .......... 1 MB   , 8-way associative, 64byte line size");
        if ( descriptor[i] ==  0x79)
            k_print_line("  L2 cache .......... 128 kb, 8-way associative, sectored, 64 byte line size");
        if ( descriptor[i] ==  0x7A)
            k_print_line("  L2 cache .......... 256 kb, 8-way associative, sectored, 64 byte line size");
        if ( descriptor[i] ==  0x7B)
            k_print_line("  L2 cache .......... 512 kb, 8-way associative, sectored, 64 byte line size");
        if ( descriptor[i] ==  0x7C)
            k_print_line("  L2 cache .......... 1M Byte, 8-way associative, sectored, 64 byte line size");
        if ( descriptor[i] ==  0x7D)
            k_print_line("  L2 cache .......... 2M Byte, 8-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x7F)
            k_print_line("  L2 cache .........512K Byte, 2-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x82)
            k_print_line("  L2 cache .......... 256 kb, 8-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x83)
            k_print_line("  L2 cache .......... 512K Byte, 8-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x84)
            k_print_line("  L2 cache ..........   1 Mb, 8-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x85)
            k_print_line("  L2 cache ..........   2 Mb, 8-way associative, 32 byte line size");
        if ( descriptor[i] ==  0x86)
            k_print_line("  L2 cache ..........   512K Byte, 4-way associative, 64 byte line size");
        if ( descriptor[i] ==  0x87)
            k_print_line("  L2 cache ..........   1M Byte, 8-way associative, 64 byte line size");
        if ( descriptor[i] ==  0xB0)
            k_print_line("  Instruction TLB       4K-Byte Pages, 4-way associative, 128 entries");
        if ( descriptor[i] ==  0xB3)
            k_print_line("  Data TLB               4K-Byte Pages, 4-way associative, 128 entries");
        if ( descriptor[i] ==  0xB4)
            k_print_line("  Data TLB               4K-Byte Pages, 4-way associative, 256 entries");
        if ( descriptor[i] ==  0xF0)
            k_print_line("  64-byte prefetching");
        if ( descriptor[i] ==  0xF1)
            k_print_line("  128-byte prefetching");
    }
}

// EDX Features
const int FPU = 1 << 0;
const int MMX = 1 << 23;
const int SSE = 1 << 25;
const int SSE2 = 1 << 26;
const int HT = 1 << 28;

//EAX Features
const int SSE3 = 1 << 9;
const int SSE41 = 1 << 19;
const int SSE42 = 1 << 20;
const int AES = 1 << 25;
const int AVX = 1 << 28;

void test_feature(uint32_t reg, int mask, const char* s){
    if(reg & mask){
        k_print(' ');
        k_print(s);
    }
}

void get_features(){
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    native_cpuid(1, &eax, &ebx, &ecx, &edx);

    k_print("Features:");

    test_feature(edx, FPU, "fpu");
    test_feature(edx, MMX, "mmx");
    test_feature(edx, SSE, "sse");
    test_feature(edx, SSE2, "sse2");
    test_feature(edx, HT, "ht");

    test_feature(ecx, SSE3, "sse3");
    test_feature(ecx, SSE41, "sse4_1");
    test_feature(ecx, SSE42, "sse4_2");
    test_feature(ecx, AES, "aes");
    test_feature(ecx, AVX, "avx");

    k_print_line();
}

void get_deterministic_cache_parameters(){
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    native_cpuid(0, &eax, &ebx, &ecx, &edx);

    if(eax < 4){
        //Not supported on this processor
        return;
    }

    size_t caches = 0;

    while(caches < 1000){
        native_cpuid(4, &eax, &ebx, &ecx, &edx);

        if ( (eax & 0x1F) == 0 ) {
            // No more caches
            break;
        }

        if ((eax & 0x1F) == 1){
            k_print("Data Cache:        ");
        }

        if ((eax & 0x1F) == 2){
            k_print("Instruction Cache: ");
        }

        if ((eax & 0x1F) == 3){
            k_print("Unified Cache:     ");
        }

        k_printf( "Level %d: ", (eax & 0xE0)/32 );
        k_printf( "Max Threads %d: ", ((eax & 0x03FFC000)/(8192))+1 );
        k_printf( "Max Procs. %d: " ,  ((eax & 0xFC000000)/(4*256*65536))+1 );
        k_printf( "Line Size = %d: ", (ebx & 0xFFF ) + 1 );
        k_printf( "Associativity = %d: ", ((ebx & 0xFFC00000)/4*16*65536) + 1 );
        k_printf( "Sets = %d:\n", ecx + 1 );

        ++caches;
    }
}

void get_brand_string(){
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    char brand_string[49];

    native_cpuid(0x80000002, &eax, &ebx, &ecx, &edx);
    *(reinterpret_cast<uint32_t*>(brand_string)+0) = eax;
    *(reinterpret_cast<uint32_t*>(brand_string)+1) = ebx;
    *(reinterpret_cast<uint32_t*>(brand_string)+2) = ecx;
    *(reinterpret_cast<uint32_t*>(brand_string)+3) = edx;

    native_cpuid(0x80000003, &eax, &ebx, &ecx, &edx);
    *(reinterpret_cast<uint32_t*>(brand_string)+4) = eax;
    *(reinterpret_cast<uint32_t*>(brand_string)+5) = ebx;
    *(reinterpret_cast<uint32_t*>(brand_string)+6) = ecx;
    *(reinterpret_cast<uint32_t*>(brand_string)+7) = edx;

    native_cpuid(0x80000004, &eax, &ebx, &ecx, &edx);
    *(reinterpret_cast<uint32_t*>(brand_string)+8) = eax;
    *(reinterpret_cast<uint32_t*>(brand_string)+9) = ebx;
    *(reinterpret_cast<uint32_t*>(brand_string)+10) = ecx;
    *(reinterpret_cast<uint32_t*>(brand_string)+11) = edx;

    brand_string[48] = '\0';

    k_printf("Brand String: %s\n", brand_string);
}

void get_vendor_id(){
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    native_cpuid(0, &eax, &ebx, &ecx, &edx);

    char vendor_id[13];
    *(reinterpret_cast<uint32_t*>(vendor_id)+0) = ebx;
    *(reinterpret_cast<uint32_t*>(vendor_id)+1) = edx;
    *(reinterpret_cast<uint32_t*>(vendor_id)+2) = ecx;
    vendor_id[12] = '\0';

    k_printf("Vendor ID: %s\n", vendor_id);
}

void get_base_info(){
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    native_cpuid(1, &eax, &ebx, &ecx, &edx);

    k_printf("Stepping: %d\n", eax & 0xF);
    k_printf("Model: %d\n", (eax >> 4) & 0xF);
    k_printf("Family: %d\n", (eax >> 8) & 0xF);
    k_printf("Processor Type: %d\n", (eax >> 12) & 0x3);
    k_printf("Extended Model: %d\n", (eax >> 16) & 0xF);
    k_printf("Extended Family: %d\n", (eax >> 20) & 0xFF);
}

} //end of anonymous namespace

void sysinfo_command(const vector<string>&){
    get_base_info();
    get_vendor_id();
    get_brand_string();
    get_features();
    get_cache_info();
    get_deterministic_cache_parameters();
}
