/*******************************************************************************
 * Copyright (c) 2011, Douglas M. Pase                                         *
 * All rights reserved.                                                        *
 * Redistribution and use in source and binary forms, with or without          *
 * modification, are permitted provided that the following conditions          *
 * are met:                                                                    *
 * o       Redistributions of source code must retain the above copyright      *
 *         notice, this list of conditions and the following disclaimer.       *
 * o       Redistributions in binary form must reproduce the above copyright   *
 *         notice, this list of conditions and the following disclaimer in     *
 *         the documentation and/or other materials provided with the          *
 *         distribution.                                                       *
 * o       Neither the name of the copyright holder nor the names of its       *
 *         contributors may be used to endorse or promote products derived     *
 *         from this software without specific prior written permission.       *
 *                                                                             *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" *
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   *
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE   *
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         *
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF        *
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    *
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN     *
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)     *
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF      *
 * THE POSSIBILITY OF SUCH DAMAGE.                                             *
 *******************************************************************************/

#if !defined(CPUID_H)
#define CPUID_H


#if defined(__KERNEL__)
#include <linux/types.h>
#else
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#endif

#define MAX_CACHE_ITERS         4
#define MAX_CACHE_DESCRIPTORS   (16*MAX_CACHE_ITERS)

#define L2_DISABLED             0x00000000
#define L2_DIRECT_MAPPED        0x00000001
#define L2_1_WAY_ASSOCIATIVE    0x00000001
#define L2_2_WAY_ASSOCIATIVE    0x00000002
#define L2_4_WAY_ASSOCIATIVE    0x00000004
#define L2_8_WAY_ASSOCIATIVE    0x00000008
#define L2_16_WAY_ASSOCIATIVE   0x00000010
#define L2_FULLY_ASSOCIATIVE    0x7fffffff
#define L2_INVALID              0xffffffff

typedef struct {
    bool cpuid_enabled;

                                // leaf 0x00000000
    char product_string[16];

                                // leaf 0x00000001
    uint8_t extended_family;
    uint8_t extended_model;
    uint8_t processor_type;
    uint8_t family_code;
    uint8_t model_number;
    uint8_t stepping_id;

    uint8_t apic_id;
    uint8_t count;                              // HTT thread count
    uint8_t chunks;
    uint8_t brand_id;

    bool SSE3;                                  // Streaming SIMD Extensions 3
    bool PCLMULDQ;                              // PCLMULDQ Instruction
    bool DTES64;                                // 64-Bit Debug Store
    bool MONITOR;                               // MONITOR/MWAIT
    bool DS_CPL;                                // CPL Qualified Debug Store
    bool VMX;                                   // Virtual Machine Extensions
    bool SMX;                                   // Safer Mode Extensions
    bool EIST;                                  // Enhanced Intel SpeedStep Technology
    bool TM2;                                   // Thermal Monitor 2
    bool SSSE3;                                 // Supplemental Streaming SIMD Extensions 3
    bool CNXT_ID;                               // L1 Context ID
    bool FMA;                                   // Fused Multiply Add
    bool CX16;                                  // CMPXCHG16B
    bool xTPR;                                  // xTPR Update Control
    bool PDCM;                                  // Perfmon and Debug Capability
    bool PCID;                                  // Process Context Identifiers
    bool DCA;                                   // Direct Cache Access
    bool SSE4_1;                                // Streaming SIMD Extensions 4.1
    bool SSE4_2;                                // Streaming SIMD Extensions 4.2
    bool x2APIC;                                // Extended xAPIC Support
    bool MOVBE;                                 // MOVBE Instruction
    bool POPCT;                                 // POPCNT Instruction
    bool TSC_DEADLINE;                          // Time Stamp Counter Deadline
    bool AES;                                   // AES Instruction Extensions
    bool XSAVE;                                 // XSAVE/XSTOR States
    bool OSXSAVE;                               // OS-Enabled Extended State Management
    bool AVX;                                   // Advanced Vector Extensions

    bool FPU;                                   // Floating-point Unit On-Chip
    bool VME;                                   // Virtual Mode Extension
    bool DE;                                    // Debugging Extension
    bool PSE;                                   // Page Size Extension
    bool TSC;                                   // Time Stamp Counter
    bool MSR;                                   // Model Specific Registers
    bool PAE;                                   // Physical Address Extension
    bool MCE;                                   // Machine-Check Exception
    bool CX8;                                   // CMPXCHG8 Instruction
    bool APIC;                                  // On-chip APIC Hardware
    bool SEP;                                   // Fast System Call
    bool MTRR;                                  // Memory Type Range Registers
    bool PGE;                                   // Page Global Enable
    bool MCA;                                   // Machine-Check Architecture
    bool CMOV;                                  // Conditional Move Instruction
    bool PAT;                                   // Page Attribute Table
    bool PSE_36;                                // 36-bit Page Size Extension
    bool PSN;                                   // Processor serial number is present and enabled
    bool CLFSH;                                 // CLFLUSH Instruction
    bool DS;                                    // Debug Store
    bool ACPI;                                  // Thermal Monitor and Software Controlled Clock Facilities
    bool MMX;                                   // MMX technology
    bool FXSR;                                  // FXSAVE and FXSTOR Instructions
    bool SSE;                                   // Streaming SIMD Extensions
    bool SSE2;                                  // Streaming SIMD Extensions 2
    bool SS;                                    // Self-Snoop
    bool HTT;                                   // Multi-Threading
    uint8_t HTT_Count;                          // Number of HyperThreads enabled (same as Count)
    bool TM;                                    // Thermal Monitor
    bool PBE;                                   // Pending Break Enable

                                // leaf 0x00000002
    char *cache_descriptor_text[16*MAX_CACHE_DESCRIPTORS];
    int   num_cache_descriptors;
    uint8_t leaf_2_calls_to_cpuid;

                                // leaf 0x00000003
                                // leaf 0x00000004
#define MAX_CACHE_LEVELS        8
    struct leaf_4_t {
        uint8_t  cache_type;                    // Cache type { NULL, Data, Instruction, Unified }
        uint8_t  reserved_apic_ids;             // Number of APIC IDs reserved for the package
        uint16_t max_threads_sharing_cache;     // Maximum number of threads sharing this cache
        bool     fully_associative;             // Fully Associative Cache
        bool     self_initializing;             // Self Initializing cache level
        uint8_t  cache_level;                   // Cache Level (Starts at 1)
        uint16_t associativity;                 // Ways of Associativity
        uint16_t physical_line_partitions;      // Physical Line partitions
        uint16_t system_coherency_line_size;    // System Coherency Line Size
        uint32_t number_of_sets;                // Number of Sets
        bool     complex_cache_indexing;        // Complex cache indexing { direct mapped == 0 }
        bool     inclusive;                     // Cache is inclusive to lower cache levels
        bool     inclusive_invalidate;          // 0 means WBINVD/INVD acts upon all lower levels of cache
        uint64_t cache_size;                    // associativity x partitions x line size x sets
    } leaf_4[MAX_CACHE_LEVELS];
    int cache_levels;

                                // leaf 0x00000005
                                // leaf 0x00000006
    bool    PTM;                                // Package Thermal Management (PTM) capability
    bool    ECMD;                               // Extended Clock Modulation Duty (ECMD) capability
    bool    PLN;                                // Power Limit Notification (PLN) capability
    bool    ARAT;                               // Always Running APIC Timer (ARAT) capability
    bool    TurboBoost;                         // Intel® Turbo Boost Technology capability
    bool    DTS;                                // Digital Thermal Sensor (DTS) capability
    uint8_t interrupt_threshholds:4;            // Number of Interrupt Thresholds
    bool    IA32_ENERGY_PERF_BIAS_MSR;          // Performance-Energy Bias capability (presence of IA32_ENERGY_PERF_BIAS MSR)
    bool    IA32_PERF_MSRs;                     // Hardware Coordination Feedback capability (presence of IA32_APERF, IA32_MPERF MSRs)

                                // leaf 0x00000007
                                // leaf 0x00000008
                                // leaf 0x00000009
    uint32_t PLATFORM_DCA_CAP_MSR;

                                // leaf 0x0000000A
    uint8_t arch_events_per_core;               // Number of arch events supported per logical processor
    uint8_t bits_per_counter;                   // Number of bits per programmable counter (width)
    uint8_t counters_per_core;                  // Number of counters per logical processor
    uint8_t arch_perfmon_version;               // Architectural PerfMon Version
    bool    branch_miss_retired;                // Branch Mispredicts Retired; 0 = supported
    bool    branch_inst_retired;                // Branch Instructions Retired; 0 = supported
    bool    LLC_misses;                         // Last Level Cache Misses; 0 = supported
    bool    LLC_refs;                           // Last Level Cache References; 0 = supported
    bool    ref_cycles;                         // Reference Cycles; 0 = supported
    bool    inst_retired;                       // Instructions Retired; 0 = supported
    bool    core_cycles;                        // Core Cycles; 0 = supported
    uint8_t bits_in_fixed_counters;             // Number of Bits in the Fixed Counters (width)
    uint8_t fixed_counters;                     // Number of Fixed Counters

                                // leaf 0x0000000B
#define MAX_TOPOLOGY_LEVELS        8
    struct leaf_b_t {
        uint8_t  apic_id_shift;                 // Number of bits to shift right APIC ID to get next level APIC ID.
        uint16_t logical_processors;            // Number of factory-configured logical processors at this level.
        uint8_t  level_type;                    // Level Type (0=Invalid, 1=Thread, 2=Core, 3-255=Reserved)
        uint8_t  level_number;                  // Level Number (same as ECX input)
        uint32_t extended_apic_id;              // Extended APIC ID -- Lower 8 bits identical to the legacy APIC ID
    } leaf_b[MAX_TOPOLOGY_LEVELS];
    int topology_levels;

                                // leaf 0x0000000C
                                // leaf 0x0000000D
                                // leaf 0x80000000
    uint32_t max_leaf;                          // Largest extended function number supported

                                // leaf 0x80000001
    bool LAHF_SAHF;                             // LAHF / SAHF
    bool Intel64;                               // Intel 64 Instruction Set Architecture
    bool RDTSCP;                                // RDTSCP and IA32_TSC_AUX
    bool one_GB_pages;                          // 1 GB Pages
    bool XD_bit;                                // Execution Disable Bit
    bool SYSCALL;                               // SYSCALL/SYSRET

                                // leaf 0x80000002
                                // leaf 0x80000003
                                // leaf 0x80000004
    bool brand_string_supported;
    char brand_string[48];                      // brand string, output of 0x80000002, 0x80000003 and 0x80000004
    char *brand_string_left;                    // pointer to left-justified brand string text

                                // leaf 0x80000005
                                // leaf 0x80000006
    uint32_t l2_cache_size;                     // L2 Cache size described in 1-KB units
    int32_t  l2_associativity;                  // L2 Cache Associativity (0xffffffff == fully associative)
    char    *l2_assoc_string;                   // text version of L2 Cache Associativity
    uint32_t l2_line_size;                      // L2 Cache Line Size in bytes

                                // leaf 0x80000007
    bool const_rate_TSC;                        // TSC Invariance (1 = Available, 0 = Not available) TSC will run 
                                                // at a constant rate in all ACPI P-states, C-states and T-states.
                                                // (invariant = 1)

                                // leaf 0x80000008
    uint16_t virtual_address_size;              // Number of address bits supported by the processor for a virtual address.
    uint16_t physical_address_size;             // Number of address bits supported by the processor for a physical address.
} hpc_cpuid_t;


int      get_cpuid(hpc_cpuid_t *c);
uint64_t tsc_per_sec(void);
#if defined(__KERNEL__)
uint64_t tsc_per_jiffy(void);
#else
uint64_t processor_clock_Hz(void);
#endif
void     print_cpuid(hpc_cpuid_t *c, int (*print)(const char* format, ...));

#endif
