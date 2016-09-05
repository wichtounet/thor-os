//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT_1_0.txt)
//=======================================================================

.intel_syntax noprefix

// Define the base ISRs

.global _arch_enable_sse

_arch_enable_sse:
    // Test if SSE is supported by the processor
    mov eax, 0x1
    cpuid
    test edx, 1<<25
    jz .no_sse

    // Enable SSE support
    xor rax, rax
    mov rax, cr0
    and ax, 0xFFFB  // clear coprocessor emulation CR0.EM
    or ax, 0x2      // set coprocessor monitoring  CR0.MP
    mov cr0, rax
    mov rax, cr4
    or ax, 3 << 9   // set CR4.OSFXSR and CR4.OSXMMEXCPT
    mov cr4, rax

    .no_sse:

    ret
