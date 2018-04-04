//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <string.hpp>

#include <tlib/print.hpp>
#include <tlib/system.hpp>

namespace {

inline void native_cpuid(uint32_t key, uint32_t& eax, uint32_t& ebx, uint32_t& ecx, uint32_t& edx){
    eax = key;

    /* ecx is often an input as well as an output. */
    asm volatile("cpuid"
        : "=a" (eax),
          "=b" (ebx),
          "=c" (ecx),
          "=d" (edx)
        : "a" (eax), "c" (ecx));
}

void get_base_info(){
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    native_cpuid(1, eax, ebx, ecx, edx);

    tlib::printf("Stepping: %u\n", eax & 0xF);
    tlib::printf("Model: %u\n", (eax >> 4) & 0xF);
    tlib::printf("Family: %u\n", (eax >> 8) & 0xF);
    tlib::printf("Processor Type: %u\n", (eax >> 12) & 0x3);
    tlib::printf("Extended Model: %u\n", (eax >> 16) & 0xF);
    tlib::printf("Extended Family: %u\n", (eax >> 20) & 0xFF);
}

void get_vendor_id(){
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    native_cpuid(0, eax, ebx, ecx, edx);

    uint32_t vendor_id[3];
    vendor_id[0] = ebx;
    vendor_id[1] = edx;
    vendor_id[2] = ecx;

    tlib::printf("Vendor ID: %s\n", reinterpret_cast<const char*>(vendor_id));
}

void get_brand_string(){
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    uint32_t brand_string[12];

    native_cpuid(0x80000002, eax, ebx, ecx, edx);
    brand_string[0] = eax;
    brand_string[1] = ebx;
    brand_string[2] = ecx;
    brand_string[3] = edx;

    native_cpuid(0x80000003, eax, ebx, ecx, edx);
    brand_string[4] = eax;
    brand_string[5] = ebx;
    brand_string[6] = ecx;
    brand_string[7] = edx;

    native_cpuid(0x80000004, eax, ebx, ecx, edx);
    brand_string[8] = eax;
    brand_string[9] = ebx;
    brand_string[10] = ecx;
    brand_string[11] = edx;

    tlib::printf("Brand String: %s\n", reinterpret_cast<const char*>(brand_string));
}

// EDX Features
const int FPU = 1 << 0;
const int APIC = 1 << 9;
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
        tlib::print(' ');
        tlib::print(s);
    }
}

void get_features(){
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    native_cpuid(1, eax, ebx, ecx, edx);

    tlib::print("Features:");

    test_feature(edx, FPU, "fpu");
    test_feature(edx, APIC, "apic");
    test_feature(edx, MMX, "mmx");
    test_feature(edx, SSE, "sse");
    test_feature(edx, SSE2, "sse2");
    test_feature(edx, HT, "ht");

    test_feature(ecx, SSE3, "sse3");
    test_feature(ecx, SSE41, "sse4_1");
    test_feature(ecx, SSE42, "sse4_2");
    test_feature(ecx, AES, "aes");
    test_feature(ecx, AVX, "avx");

    tlib::print_line();
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

    tlib::print_line("Cache and TLB:");

    native_cpuid(0, eax, ebx, ecx, edx);

    if (eax < 2){
        tlib::print_line("   CPUID(2) not supported");
        return;
    }

    native_cpuid(2, eax, ebx, ecx, edx);

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
        native_cpuid(2, eax, ebx, ecx, edx);
        desc += 16;
    }

    for (i=0; i< next; i++)     {
        if ( descriptor[i] ==  0x00){
            // NULL descriptor, legal value but no info
            continue;
        }

        if ( descriptor[i] ==  0x01){
            tlib::print_line("  Instruction TLB ...   4 kb pages, 4-way associative, 32 entries");
        }
        if ( descriptor[i] ==  0x02){
            tlib::print_line("  Instruction TLB ...   4 Mb pages, 4-way associative, 2 entries");
        }
        if ( descriptor[i] ==  0x03){
            tlib::print_line("  Data TLB ..........   4 kb pages, 4-way associative, 64 entries");
        }
        if ( descriptor[i] ==  0x04){
            tlib::print_line("  Data TLB ..........   4 Mb pages, 4-way associative, 8 entries");
        }
        if ( descriptor[i] ==  0x06){
            tlib::print_line("  L1 instruction cache  8 kb, 4-way associative, 32 byte line size");
        }
        if ( descriptor[i] ==  0x08){
            tlib::print_line("  L1 instruction cache 16 kb, 4-way associative, 32 byte line size");
        }
        if ( descriptor[i] ==  0x0A){
            tlib::print_line("  L1 data cache .....   8 kb, 2-way associative, 32 byte line size");
        }
        if ( descriptor[i] ==  0x0B){
            tlib::print_line("  Instruction TLB ...   4 Mb pages, 4-way associative, 4 entries");
        }
        if ( descriptor[i] ==  0x0C){
            tlib::print_line("  L1 data cache .....  16 kb, 4-way associative, 32 byte line size");
        }
        if ( descriptor[i] ==  0x22){
            tlib::print_line("  L3 cache:     512K Bytes, 4-way associative, 64 byte line size, ");
            tlib::print_line("                       128 byte sector size" );
        }
        if ( descriptor[i] ==  0x23){
            tlib::print_line("  L3 cache:     1M Bytes, 8-way associative, 64 byte line size, ");
            tlib::print_line("                       128 byte sector size" );
        }
        if ( descriptor[i] ==  0x25){
            tlib::print_line("  L3 cache:     2M Bytes, 8-way associative, 64 byte line size, ");
            tlib::print_line("                       128 byte sector size" );
        }
        if ( descriptor[i] ==  0x29){
            tlib::print_line("  L3 cache:     4M Bytes, 8-way associative, 64 byte line size, ");
            tlib::print_line("                       128 byte sector size" );
        }
        if ( descriptor[i] ==  0x2C){
            tlib::print_line("  1st-level D-cache:   32K Bytes, 8-way associative, 64 byte line size");
        }
        if ( descriptor[i] ==  0x30){
            tlib::print_line("  1st-level I-cache:   32K Bytes, 8-way associative, 64 byte line size");
        }
        if ( descriptor[i] ==  0x40){
            tlib::print_line("  No L2 cache OR if there is an L2 cache, then no L3 cache");
        }
        if ( descriptor[i] ==  0x41){
            tlib::print_line("  L2 cache .......... 128 kb, 4-way associative, 32 byte line size");
        }
        if ( descriptor[i] ==  0x42){
            tlib::print_line("  L2 cache .......... 256 kb, 4-way associative, 32 byte line size");
        }
        if ( descriptor[i] ==  0x43){
            tlib::print_line("  L2 cache .......... 512 kb, 4-way associative, 32 byte line size");
        }
        if ( descriptor[i] ==  0x44){
            tlib::print_line("  L2 cache ..........   1 Mb, 4-way associative, 32 byte line size");
        }
        if ( descriptor[i] ==  0x45){
            tlib::print_line("  L2 cache ..........   2 Mb, 4-way associative, 32 byte line size");
        }
        if ( descriptor[i] ==  0x46){
            tlib::print_line("  L3 cache ..........   4 Mb, 4-way associative, 64 byte line size");
        }
        if ( descriptor[i] ==  0x47){
            tlib::print_line("  L3 cache ..........   8 Mb, 8-way associative, 64 byte line size");
        }
        if ( descriptor[i] ==  0x49){
            tlib::print_line("  L2 cache ..........   4 Mb, 16-way associative, 64 byte line size");
        }
        if ( descriptor[i] ==  0x50){
            tlib::print_line("  Instruction TLB ...   4 kb and 2 Mb or 4 Mb pages, 64 entries");
        }
        if ( descriptor[i] ==  0x51){
            tlib::print_line("  Instruction TLB ...   4 kb and 2 Mb or 4 Mb pages, 128 entries");
        }
        if ( descriptor[i] ==  0x52){
            tlib::print_line("  Instruction TLB ...   4 kb and 2 Mb or 4 Mb pages, 256 entries");
        }
        if ( descriptor[i] ==  0x56){
            tlib::print_line("  Data TLB ..........   4 Mb pages, 4-way associative,  16 entries");
        }
        if ( descriptor[i] ==  0x57){
            tlib::print_line("  Data TLB ..........   4 Kb pages, 4-way associative,  16 entries");
        }
        if ( descriptor[i] ==  0x5B){
            tlib::print_line("  Data TLB ..........   4 kb and 4 Mb pages,  64 entries");
        }
        if ( descriptor[i] ==  0x5C){
            tlib::print_line("  Data TLB ..........   4 kb and 4 Mb pages, 128 entries");
        }
        if ( descriptor[i] ==  0x5D){
            tlib::print_line("  Data TLB ..........   4 kb and 4 Mb pages, 256 entries");
        }
        if ( descriptor[i] ==  0x60){
            tlib::print_line("  L1 data cache .....  16 kb, 8-way associative, 64 byte line size");
        }
        if ( descriptor[i] ==  0x66){
            tlib::print_line("  L1 data cache .....   8 kb, 4-way associative, 64 byte line size");
        }
        if ( descriptor[i] ==  0x67){
            tlib::print_line("  L1 data cache .....  16 kb, 4-way associative, 64 byte line size");
        }
        if ( descriptor[i] ==  0x68){
            tlib::print_line("  L1 data cache .....  32 kb, 4-way associative, 64 byte line size");
        }
        if ( descriptor[i] ==  0x70){
            tlib::print_line("  Trace cache ......  12k uop, 8-way associative");
        }
        if ( descriptor[i] ==  0x71){
            tlib::print_line("  Trace cache ......  16k uop, 8-way associative");
        }
        if ( descriptor[i] ==  0x72){
            tlib::print_line("  Trace cache ......  32k uop, 8-way associative");
        }
        if ( descriptor[i] ==  0x78){
            tlib::print_line("  L2 cache .......... 1 MB   , 8-way associative, 64byte line size");
        }
        if ( descriptor[i] ==  0x79){
            tlib::print_line("  L2 cache .......... 128 kb, 8-way associative, sectored, 64 byte line size");
        }
        if ( descriptor[i] ==  0x7A){
            tlib::print_line("  L2 cache .......... 256 kb, 8-way associative, sectored, 64 byte line size");
        }
        if ( descriptor[i] ==  0x7B){
            tlib::print_line("  L2 cache .......... 512 kb, 8-way associative, sectored, 64 byte line size");
        }
        if ( descriptor[i] ==  0x7C){
            tlib::print_line("  L2 cache .......... 1M Byte, 8-way associative, sectored, 64 byte line size");
        }
        if ( descriptor[i] ==  0x7D){
            tlib::print_line("  L2 cache .......... 2M Byte, 8-way associative, 64 byte line size");
        }
        if ( descriptor[i] ==  0x7F){
            tlib::print_line("  L2 cache .........512K Byte, 2-way associative, 64 byte line size");
        }
        if ( descriptor[i] ==  0x82){
            tlib::print_line("  L2 cache .......... 256 kb, 8-way associative, 32 byte line size");
        }
        if ( descriptor[i] ==  0x83){
            tlib::print_line("  L2 cache .......... 512K Byte, 8-way associative, 32 byte line size");
        }
        if ( descriptor[i] ==  0x84){
            tlib::print_line("  L2 cache ..........   1 Mb, 8-way associative, 32 byte line size");
        }
        if ( descriptor[i] ==  0x85){
            tlib::print_line("  L2 cache ..........   2 Mb, 8-way associative, 32 byte line size");
        }
        if ( descriptor[i] ==  0x86){
            tlib::print_line("  L2 cache ..........   512K Byte, 4-way associative, 64 byte line size");
        }
        if ( descriptor[i] ==  0x87){
            tlib::print_line("  L2 cache ..........   1M Byte, 8-way associative, 64 byte line size");
        }
        if ( descriptor[i] ==  0xB0){
            tlib::print_line("  Instruction TLB       4K-Byte Pages, 4-way associative, 128 entries");
        }
        if ( descriptor[i] ==  0xB3){
            tlib::print_line("  Data TLB               4K-Byte Pages, 4-way associative, 128 entries");
        }
        if ( descriptor[i] ==  0xB4){
            tlib::print_line("  Data TLB               4K-Byte Pages, 4-way associative, 256 entries");
        }
        if ( descriptor[i] ==  0xF0){
            tlib::print_line("  64-byte prefetching");
        }
        if ( descriptor[i] ==  0xF1){
            tlib::print_line("  128-byte prefetching");
        }
    }
}

void get_deterministic_cache_parameters(){
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    native_cpuid(0, eax, ebx, ecx, edx);

    if(eax < 4){
        //Not supported on this processor
        return;
    }

    size_t caches = 0;

    while(caches < 1000){
        native_cpuid(4, eax, ebx, ecx, edx);

        if ( (eax & 0x1F) == 0 ) {
            // No more caches
            break;
        }

        if ((eax & 0x1F) == 1){
            tlib::print("Data Cache:        ");
        }

        if ((eax & 0x1F) == 2){
            tlib::print("Instruction Cache: ");
        }

        if ((eax & 0x1F) == 3){
            tlib::print("Unified Cache:     ");
        }

        tlib::printf( "Level %u: ", static_cast<size_t>((eax & 0xE0)/32));
        tlib::printf( "Max Threads %u: ", static_cast<size_t>(((eax & 0x03FFC000)/(8192))+1));
        tlib::printf( "Max Procs. %u: " ,  static_cast<size_t>(((eax & 0xFC000000)/(4*256*65536))+1));
        tlib::printf( "Line Size = %u: ", static_cast<size_t>((ebx & 0xFFF ) + 1));
        tlib::printf( "Associativity = %u: ", (static_cast<size_t>((ebx & 0xFFC00000)/4*16*65536) + 1));
        tlib::printf( "Sets = %u:\n", static_cast<size_t>(ecx + 1));

        ++caches;
    }
}

} //end of anonymous namespace

int main(){
    get_base_info();
    get_vendor_id();
    get_brand_string();
    get_features();
    get_cache_info();
    get_deterministic_cache_parameters();

    return 0;
}
