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


 // http://www.intel.com/content/dam/doc/application-note/processor-identification-cpuid-instruction-note.pdf

#include "cpuid.h"

static int  detect_cpuid(void);
static void get_leaf_00000000(hpc_cpuid_t *c);
static void get_leaf_00000001(hpc_cpuid_t *c);
static void get_leaf_00000002(hpc_cpuid_t *c);
static void get_leaf_00000003(hpc_cpuid_t *c);
static void get_leaf_00000004(hpc_cpuid_t *c);
static void get_leaf_00000005(hpc_cpuid_t *c);
static void get_leaf_00000006(hpc_cpuid_t *c);
static void get_leaf_00000007(hpc_cpuid_t *c);
static void get_leaf_00000008(hpc_cpuid_t *c);
static void get_leaf_00000009(hpc_cpuid_t *c);
static void get_leaf_0000000A(hpc_cpuid_t *c);
static void get_leaf_0000000B(hpc_cpuid_t *c);
static void get_leaf_0000000C(hpc_cpuid_t *c);
static void get_leaf_0000000D(hpc_cpuid_t *c);

static void get_leaf_80000000(hpc_cpuid_t *c);
static void get_leaf_80000001(hpc_cpuid_t *c);
static void get_leaf_80000002(hpc_cpuid_t *c);
static void get_leaf_80000003(hpc_cpuid_t *c);
static void get_leaf_80000004(hpc_cpuid_t *c);
static void get_leaf_80000005(hpc_cpuid_t *c);
static void get_leaf_80000006(hpc_cpuid_t *c);
static void get_leaf_80000007(hpc_cpuid_t *c);
static void get_leaf_80000008(hpc_cpuid_t *c);

static void print_leaf_00000000(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_00000001(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_00000002(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_00000003(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_00000004(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_00000005(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_00000006(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_00000007(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_00000008(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_00000009(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_0000000A(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_0000000B(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_0000000C(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_0000000D(hpc_cpuid_t *c, int (*print)(const char* format, ...));

static void print_leaf_80000000(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_80000001(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_80000002(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_80000003(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_80000004(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_80000005(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_80000006(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_80000007(hpc_cpuid_t *c, int (*print)(const char* format, ...));
static void print_leaf_80000008(hpc_cpuid_t *c, int (*print)(const char* format, ...));

static char* cache_descriptor_table[];


/*
 * See Intel publication 241618 or Intel Application Note 485.
 */

int get_cpuid(hpc_cpuid_t *c)
{
    if (c == NULL) return -1;

    if ((c->cpuid_enabled=detect_cpuid()) != 0) {
        get_leaf_00000000(c);
        get_leaf_00000001(c);
        get_leaf_00000002(c);
        get_leaf_00000003(c);
        get_leaf_00000004(c);
        get_leaf_00000005(c);
        get_leaf_00000006(c);
        get_leaf_00000007(c);
        get_leaf_00000008(c);
        get_leaf_00000009(c);
        get_leaf_0000000A(c);
        get_leaf_0000000B(c);
        get_leaf_0000000C(c);
        get_leaf_0000000D(c);

        get_leaf_80000000(c);
        if (0x80000001 <= c->max_leaf) get_leaf_80000001(c);
        if (0x80000002 <= c->max_leaf) get_leaf_80000002(c);
        if (0x80000003 <= c->max_leaf) get_leaf_80000003(c);
        if (0x80000004 <= c->max_leaf) get_leaf_80000004(c);
        if (0x80000005 <= c->max_leaf) get_leaf_80000005(c);
        if (0x80000006 <= c->max_leaf) get_leaf_80000006(c);
        if (0x80000007 <= c->max_leaf) get_leaf_80000007(c);
        if (0x80000008 <= c->max_leaf) get_leaf_80000008(c);

        return 0;
    }

    return -1;
}

static int detect_cpuid(void)
{
    /*
     * This tests whether the processor supports CPUID.
     * If the ID flag (bit 21) in the EFLAGS register can be
     * set and cleared by software, then the processor supports
     * CPUID. 
     *
     * A return value of TRUE means CPUID has been detected.
     */
     
#if defined(__i386__)
    return 1;
#elif defined(__x86_64__)
     int before=0, after=0;
     
    __asm__ __volatile__(
        "pushf ;"
        "pop %%rax;"
        "movl %%eax,%0;"
        "movl $0x00200000,%%ebx;"
        "xor %%ebx,%%eax ;"
        "push %%rax;"
        "popf ;"
        "pushf ;"
        "pop %%rax;"
        "movl %%eax,%1;"
        ""
        : "=r"(before), "=r"(after)
        :
        : "%eax", "%ebx"
    );

    return (before != after);
#endif
}

static void get_leaf_00000000(hpc_cpuid_t *c)
{
    int32_t eax=0x00000000;
    int32_t ebx=0x0;
    int32_t ecx=0x0;
    int32_t edx=0x0;

    asm("\n"
        "movl %4,%%eax;         \n\t"
        "movl $0,%%ebx;         \n\t"
        "movl %5,%%ecx;         \n\t"
        "movl $0,%%edx;         \n\t"
        "cpuid;                 \n\t"
        "movl %%eax,%0;         \n\t"
        "movl %%ebx,%1;         \n\t"
        "movl %%ecx,%2;         \n\t"
        "movl %%edx,%3;         \n\t"
        : "=m"(eax),"=m"(ebx),"=m"(ecx),"=m"(edx)
        : "m"(eax),"m"(ecx)
        : "%eax","%ebx","%ecx","%edx"
    );

    c->product_string[ 0] = ((char*)&ebx)[0];
    c->product_string[ 1] = ((char*)&ebx)[1];
    c->product_string[ 2] = ((char*)&ebx)[2];
    c->product_string[ 3] = ((char*)&ebx)[3];

    c->product_string[ 4] = ((char*)&edx)[0];
    c->product_string[ 5] = ((char*)&edx)[1];
    c->product_string[ 6] = ((char*)&edx)[2];
    c->product_string[ 7] = ((char*)&edx)[3];

    c->product_string[ 8] = ((char*)&ecx)[0];
    c->product_string[ 9] = ((char*)&ecx)[1];
    c->product_string[10] = ((char*)&ecx)[2];
    c->product_string[11] = ((char*)&ecx)[3];

    c->product_string[12] = '\0';
    c->product_string[13] = '\0';
    c->product_string[14] = '\0';
    c->product_string[15] = '\0';
}

static void get_leaf_00000001(hpc_cpuid_t *c)
{
    int32_t eax=0x00000001;
    int32_t ebx=0x0;
    int32_t ecx=0x0;
    int32_t edx=0x0;

    asm("\n"
        "movl %4,%%eax;         \n\t"
        "movl $0,%%ebx;         \n\t"
        "movl %5,%%ecx;         \n\t"
        "movl $0,%%edx;         \n\t"
        "cpuid;                 \n\t"
        "movl %%eax,%0;         \n\t"
        "movl %%ebx,%1;         \n\t"
        "movl %%ecx,%2;         \n\t"
        "movl %%edx,%3;         \n\t"
        : "=m"(eax),"=m"(ebx),"=m"(ecx),"=m"(edx)
        : "m"(eax),"m"(ecx)
        : "%eax","%ebx","%ecx","%edx"
    );

    c->extended_family = (eax >> 20) & 0xFF;
    c->extended_model  = (eax >> 16) & 0x0F;
    c->processor_type  = (eax >> 12) & 0x03;
    c->family_code     = (eax >>  8) & 0x0F;
    c->model_number    = (eax >>  4) & 0x0F;
    c->stepping_id     = (eax >>  0) & 0x0F;

    c->apic_id         = (ebx >> 24) & 0xFF;
    c->count           = (ebx >> 16) & 0xFF;
    c->chunks          = (ebx >>  8) & 0xFF;
    c->brand_id        = (ebx >>  0) & 0xFF;

    c->SSE3            = (ecx >>  0) & 0x01;
    c->PCLMULDQ        = (ecx >>  1) & 0x01;
    c->DTES64          = (ecx >>  2) & 0x01;
    c->MONITOR         = (ecx >>  3) & 0x01;
    c->DS_CPL          = (ecx >>  4) & 0x01;
    c->VMX             = (ecx >>  5) & 0x01;
    c->SMX             = (ecx >>  6) & 0x01;
    c->EIST            = (ecx >>  7) & 0x01;
    c->TM2             = (ecx >>  8) & 0x01;
    c->SSSE3           = (ecx >>  9) & 0x01;
    c->CNXT_ID         = (ecx >> 10) & 0x01;
                                // bit 11 reserved
    c->FMA             = (ecx >> 12) & 0x01;
    c->CX16            = (ecx >> 13) & 0x01;
    c->xTPR            = (ecx >> 14) & 0x01;
    c->PDCM            = (ecx >> 15) & 0x01;
                                // bit 16 reserved
    c->PCID            = (ecx >> 17) & 0x01;
    c->DCA             = (ecx >> 18) & 0x01;
    c->SSE4_1          = (ecx >> 19) & 0x01;
    c->SSE4_2          = (ecx >> 20) & 0x01;
    c->x2APIC          = (ecx >> 21) & 0x01;
    c->MOVBE           = (ecx >> 22) & 0x01;
    c->POPCT           = (ecx >> 23) & 0x01;
    c->TSC_DEADLINE    = (ecx >> 24) & 0x01;
    c->AES             = (ecx >> 25) & 0x01;
    c->XSAVE           = (ecx >> 26) & 0x01;
    c->OSXSAVE         = (ecx >> 27) & 0x01;
    c->AVX             = (ecx >> 28) & 0x01;
                                // bit 29 reserved
                                // bit 30 reserved
                                // bit 31 not used (always zero)

    c->FPU             = (edx >>  0) & 0x01;
    c->VME             = (edx >>  1) & 0x01;
    c->DE              = (edx >>  2) & 0x01;
    c->PSE             = (edx >>  3) & 0x01;
    c->TSC             = (edx >>  4) & 0x01;
    c->MSR             = (edx >>  5) & 0x01;
    c->PAE             = (edx >>  6) & 0x01;
    c->MCE             = (edx >>  7) & 0x01;
    c->CX8             = (edx >>  8) & 0x01;
    c->APIC            = (edx >>  9) & 0x01;
                                // bit 10 reserved
    c->SEP             = (edx >> 11) & 0x01;
    c->MTRR            = (edx >> 12) & 0x01;
    c->PGE             = (edx >> 13) & 0x01;
    c->MCA             = (edx >> 14) & 0x01;
    c->CMOV            = (edx >> 15) & 0x01;
    c->PAT             = (edx >> 16) & 0x01;
    c->PSE_36          = (edx >> 17) & 0x01;
    c->PSN             = (edx >> 18) & 0x01;
    c->CLFSH           = (edx >> 19) & 0x01;
                                // bit 20 reserved
    c->DS              = (edx >> 21) & 0x01;
    c->ACPI            = (edx >> 22) & 0x01;
    c->MMX             = (edx >> 23) & 0x01;
    c->FXSR            = (edx >> 24) & 0x01;
    c->SSE             = (edx >> 25) & 0x01;
    c->SSE2            = (edx >> 26) & 0x01;
    c->SS              = (edx >> 27) & 0x01;
    c->HTT             = (edx >> 28) & 0x01;
    c->HTT_Count       = (ebx >> 16) & 0xff;
    c->TM              = (edx >> 29) & 0x01;
                                // bit 30 reserved
    c->PBE             = (edx >> 31) & 0x01;
}

static void get_leaf_00000002(hpc_cpuid_t *c)
{
    int32_t eax=0x00000002;
    int32_t ebx=0x0;
    int32_t ecx=0x0;
    int32_t edx=0x0;
    int calls_to_cpuid;
    int32_t *rptr[4] = { &eax, &ebx, &ecx, &edx };
    int i, j, k;

    asm("\n"
        "movl %4,%%eax;         \n\t"
        "movl $0,%%ebx;         \n\t"
        "movl %5,%%ecx;         \n\t"
        "movl $0,%%edx;         \n\t"
        "cpuid;                 \n\t"
        "movl %%eax,%0;         \n\t"
        "movl %%ebx,%1;         \n\t"
        "movl %%ecx,%2;         \n\t"
        "movl %%edx,%3;         \n\t"
        : "=m"(eax),"=m"(ebx),"=m"(ecx),"=m"(edx)
        : "m"(eax),"m"(ecx)
        : "%eax","%ebx","%ecx","%edx"
    );

    calls_to_cpuid = eax & 0x000000ff;
    c->leaf_2_calls_to_cpuid = calls_to_cpuid;

    c->num_cache_descriptors = 0;
    for (k=0; k < MAX_CACHE_ITERS && k < calls_to_cpuid; k+=1) {
        for (j=0; j < 4; j+=1) {
            int32_t *ex = rptr[j];
            for (i=0; i < 32; i+=8) {
                if (k == 0 && j == 0 && i == 0) continue;
                if (((*ex >> i) & 0xff) != 0) {
                    c->cache_descriptor_text[c->num_cache_descriptors++] = cache_descriptor_table[(*ex >> i) & 0xff];
                }
            }
        }
    }
}

static void get_leaf_00000003(hpc_cpuid_t *c)
{
                                // processor serial number,
                                // not used after pentium iii
}

static void get_leaf_00000004(hpc_cpuid_t *c)
{
                                // deterministic cache parameters
    int32_t eax=0x00000004;
    int32_t ebx=0x0;
    int32_t ecx=0x0;
    int32_t edx=0x0;

    int32_t this_cache=0;       // which cache we're working on
    do {
        eax = 0x00000004;
        ecx = this_cache;
        asm("\n"
            "movl %4,%%eax;             \n\t"
            "movl $0,%%ebx;             \n\t"
            "movl %5,%%ecx;             \n\t"
            "movl $0,%%edx;             \n\t"
            "cpuid;                     \n\t"
            "movl %%eax,%0;             \n\t"
            "movl %%ebx,%1;             \n\t"
            "movl %%ecx,%2;             \n\t"
            "movl %%edx,%3;             \n\t"
            : "=m"(eax),"=m"(ebx),"=m"(ecx),"=m"(edx)
            : "m"(eax),"m"(ecx)
            : "%eax","%ebx","%ecx","%edx"
        );

        c->leaf_4[this_cache].cache_type                 = (eax >>  0) & 0x1f;
        if (c->leaf_4[this_cache].cache_type == 0) break;

        c->leaf_4[this_cache].reserved_apic_ids          =  (eax >> 26) & 0x3f;
        c->leaf_4[this_cache].max_threads_sharing_cache  = ((eax >> 14) & 0xfff) + 1;
        c->leaf_4[this_cache].fully_associative          =  (eax >>  9) & 0x1;
        c->leaf_4[this_cache].self_initializing          =  (eax >>  8) & 0x1;
        c->leaf_4[this_cache].cache_level                =  (eax >>  5) & 0x7;
        c->leaf_4[this_cache].associativity              = ((ebx >> 22) & 0x3ff) + 1;
        c->leaf_4[this_cache].physical_line_partitions   = ((ebx >> 12) & 0x3ff) + 1;
        c->leaf_4[this_cache].system_coherency_line_size = ((ebx >>  0) & 0xfff) + 1;
        c->leaf_4[this_cache].number_of_sets             =   ecx + 1;
        c->leaf_4[this_cache].complex_cache_indexing     =  (edx >>  2) & 0x1;
        c->leaf_4[this_cache].inclusive                  =  (edx >>  1) & 0x1;
        c->leaf_4[this_cache].inclusive_invalidate       =  (edx >>  0) & 0x1;

        c->leaf_4[this_cache].cache_size =
            c->leaf_4[this_cache].associativity * 
            c->leaf_4[this_cache].physical_line_partitions *
            c->leaf_4[this_cache].system_coherency_line_size * 
            c->leaf_4[this_cache].number_of_sets;

        this_cache += 1;
    } while(this_cache < MAX_CACHE_LEVELS);
    c->cache_levels = this_cache;
}

static void get_leaf_00000005(hpc_cpuid_t *c)
{
                                // MONITOR/MWAIT parameters
}

static void get_leaf_00000006(hpc_cpuid_t *c)
{
                                // digital thermal sensor and power management parameters
    int32_t eax=0x00000006;
    int32_t ebx=0x0;
    int32_t ecx=0x0;
    int32_t edx=0x0;

    asm("\n"
        "movl %4,%%eax;         \n\t"
        "movl $0,%%ebx;         \n\t"
        "movl %5,%%ecx;         \n\t"
        "movl $0,%%edx;         \n\t"
        "cpuid;                 \n\t"
        "movl %%eax,%0;         \n\t"
        "movl %%ebx,%1;         \n\t"
        "movl %%ecx,%2;         \n\t"
        "movl %%edx,%3;         \n\t"
        : "=m"(eax),"=m"(ebx),"=m"(ecx),"=m"(edx)
        : "m"(eax),"m"(ecx)
        : "%eax","%ebx","%ecx","%edx"
    );

    c->PTM                       = (eax >> 6) & 0x1;
    c->ECMD                      = (eax >> 5) & 0x1;
    c->PLN                       = (eax >> 4) & 0x1;
    c->ARAT                      = (eax >> 2) & 0x1;
    c->TurboBoost                = (eax >> 1) & 0x1;
    c->DTS                       = (eax >> 0) & 0x1;
    c->interrupt_threshholds     = (ebx >> 0) & 0xF;
    c->IA32_ENERGY_PERF_BIAS_MSR = (ecx >> 3) & 0x1;
    c->IA32_PERF_MSRs            = (ecx >> 0) & 0x1;
}

static void get_leaf_00000007(hpc_cpuid_t *c)
{
                                // reserved
}

static void get_leaf_00000008(hpc_cpuid_t *c)
{
                                // reserved
}

static void get_leaf_00000009(hpc_cpuid_t *c)
{
                                // Direct Cache Access (DCA) parameters
    int32_t eax=0x00000009;
    int32_t ebx=0x0;
    int32_t ecx=0x0;
    int32_t edx=0x0;

    asm("\n"
        "movl %4,%%eax;         \n\t"
        "movl $0,%%ebx;         \n\t"
        "movl %5,%%ecx;         \n\t"
        "movl $0,%%edx;         \n\t"
        "cpuid;                 \n\t"
        "movl %%eax,%0;         \n\t"
        "movl %%ebx,%1;         \n\t"
        "movl %%ecx,%2;         \n\t"
        "movl %%edx,%3;         \n\t"
        : "=m"(eax),"=m"(ebx),"=m"(ecx),"=m"(edx)
        : "m"(eax),"m"(ecx)
        : "%eax","%ebx","%ecx","%edx"
    );

    c->PLATFORM_DCA_CAP_MSR = (eax >> 0) & 0xFFFFFFFF;
}

static void get_leaf_0000000A(hpc_cpuid_t *c)
{
                                // architectural performance monitor features
    int32_t eax=0x0000000A;
    int32_t ebx=0x0;
    int32_t ecx=0x0;
    int32_t edx=0x0;

    asm("\n"
        "movl %4,%%eax;         \n\t"
        "movl $0,%%ebx;         \n\t"
        "movl %5,%%ecx;         \n\t"
        "movl $0,%%edx;         \n\t"
        "cpuid;                 \n\t"
        "movl %%eax,%0;         \n\t"
        "movl %%ebx,%1;         \n\t"
        "movl %%ecx,%2;         \n\t"
        "movl %%edx,%3;         \n\t"
        : "=m"(eax),"=m"(ebx),"=m"(ecx),"=m"(edx)
        : "m"(eax),"m"(ecx)
        : "%eax","%ebx","%ecx","%edx"
    );

    c->arch_events_per_core   = (eax >> 24) & 0xff;
    c->bits_per_counter       = (eax >> 16) & 0xff;
    c->counters_per_core      = (eax >>  8) & 0xff;
    c->arch_perfmon_version   = (eax >>  0) & 0xff;
    c->branch_miss_retired    = (ebx >>  6) & 0x1;
    c->branch_inst_retired    = (ebx >>  5) & 0x1;
    c->LLC_misses             = (ebx >>  4) & 0x1;
    c->LLC_refs               = (ebx >>  3) & 0x1;
    c->ref_cycles             = (ebx >>  2) & 0x1;
    c->inst_retired           = (ebx >>  1) & 0x1;
    c->core_cycles            = (ebx >>  0) & 0x1;
    c->bits_in_fixed_counters = (edx >>  5) & 0xff;
    c->fixed_counters         = (edx >>  0) & 0x1f;
}

static void get_leaf_0000000B(hpc_cpuid_t *c)
{
                                // x2APIC / processor topology
    int32_t eax=0x0000000B;
    int32_t ebx=0x0;
    int32_t ecx=0x0;
    int32_t edx=0x0;

    int32_t this_level=0;       // which cache we're working on
    do {
        eax = 0x0000000B;
        ecx = this_level;
        asm("\n"
            "movl %4,%%eax;             \n\t"
            "movl $0,%%ebx;             \n\t"
            "movl %5,%%ecx;             \n\t"
            "movl $0,%%edx;             \n\t"
            "cpuid;                     \n\t"
            "movl %%eax,%0;             \n\t"
            "movl %%ebx,%1;             \n\t"
            "movl %%ecx,%2;             \n\t"
            "movl %%edx,%3;             \n\t"
            : "=m"(eax),"=m"(ebx),"=m"(ecx),"=m"(edx)
            : "m"(eax),"m"(ecx)
            : "%eax","%ebx","%ecx","%edx"
        );

        if (eax == 0 && ebx == 0) break;

        c->leaf_b[this_level].apic_id_shift      = (eax >> 0) & 0x1f;
        c->leaf_b[this_level].logical_processors = (ebx >> 0) & 0xffff;
        c->leaf_b[this_level].level_type         = (ecx >> 8) & 0xff;
        c->leaf_b[this_level].level_number       = (ecx >> 0) & 0xff;
        c->leaf_b[this_level].extended_apic_id   =  edx;

        this_level += 1;
    } while(this_level < MAX_TOPOLOGY_LEVELS);
    c->topology_levels = this_level;
}

static void get_leaf_0000000C(hpc_cpuid_t *c)
{
                                // reserved
}

static void get_leaf_0000000D(hpc_cpuid_t *c)
{
                                // XSAVE features
}

static void get_leaf_80000000(hpc_cpuid_t *c)
{
    int32_t eax=0x80000000;
    int32_t ebx=0x0;
    int32_t ecx=0x0;
    int32_t edx=0x0;

    asm("\n"
        "movl %4,%%eax;         \n\t"
        "movl $0,%%ebx;         \n\t"
        "movl %5,%%ecx;         \n\t"
        "movl $0,%%edx;         \n\t"
        "cpuid;                 \n\t"
        "movl %%eax,%0;         \n\t"
        "movl %%ebx,%1;         \n\t"
        "movl %%ecx,%2;         \n\t"
        "movl %%edx,%3;         \n\t"
        : "=m"(eax),"=m"(ebx),"=m"(ecx),"=m"(edx)
        : "m"(eax),"m"(ecx)
        : "%eax","%ebx","%ecx","%edx"
    );

    c->max_leaf = eax;
    c->brand_string_supported = (0x80000004 <= eax);
}

static void get_leaf_80000001(hpc_cpuid_t *c)
{
    int32_t eax=0x80000001;
    int32_t ebx=0x0;
    int32_t ecx=0x0;
    int32_t edx=0x0;

    asm("\n"
        "movl %4,%%eax;         \n\t"
        "movl $0,%%ebx;         \n\t"
        "movl %5,%%ecx;         \n\t"
        "movl $0,%%edx;         \n\t"
        "cpuid;                 \n\t"
        "movl %%eax,%0;         \n\t"
        "movl %%ebx,%1;         \n\t"
        "movl %%ecx,%2;         \n\t"
        "movl %%edx,%3;         \n\t"
        : "=m"(eax),"=m"(ebx),"=m"(ecx),"=m"(edx)
        : "m"(eax),"m"(ecx)
        : "%eax","%ebx","%ecx","%edx"
    );

    c->LAHF_SAHF    = (ecx>> 0)&1;
    c->Intel64      = (edx>>29)&1;
    c->RDTSCP       = (edx>>27)&1;
    c->one_GB_pages = (edx>>26)&1;
    c->XD_bit       = (edx>>20)&1;
    c->SYSCALL      = (edx>>11)&1;
}

static void get_leaf_80000002(hpc_cpuid_t *c)
{
    int32_t eax=0x80000002;
    int32_t ebx=0x0;
    int32_t ecx=0x0;
    int32_t edx=0x0;

    asm("\n"
        "movl %4,%%eax;         \n\t"
        "movl $0,%%ebx;         \n\t"
        "movl %5,%%ecx;         \n\t"
        "movl $0,%%edx;         \n\t"
        "cpuid;                 \n\t"
        "movl %%eax,%0;         \n\t"
        "movl %%ebx,%1;         \n\t"
        "movl %%ecx,%2;         \n\t"
        "movl %%edx,%3;         \n\t"
        : "=m"(eax),"=m"(ebx),"=m"(ecx),"=m"(edx)
        : "m"(eax),"m"(ecx)
        : "%eax","%ebx","%ecx","%edx"
    );

    c->brand_string[ 0] = ((char*)&eax)[0];
    c->brand_string[ 1] = ((char*)&eax)[1];
    c->brand_string[ 2] = ((char*)&eax)[2];
    c->brand_string[ 3] = ((char*)&eax)[3];
    c->brand_string[ 4] = ((char*)&ebx)[0];
    c->brand_string[ 5] = ((char*)&ebx)[1];
    c->brand_string[ 6] = ((char*)&ebx)[2];
    c->brand_string[ 7] = ((char*)&ebx)[3];
    c->brand_string[ 8] = ((char*)&ecx)[0];
    c->brand_string[ 9] = ((char*)&ecx)[1];
    c->brand_string[10] = ((char*)&ecx)[2];
    c->brand_string[11] = ((char*)&ecx)[3];
    c->brand_string[12] = ((char*)&edx)[0];
    c->brand_string[13] = ((char*)&edx)[1];
    c->brand_string[14] = ((char*)&edx)[2];
    c->brand_string[15] = ((char*)&edx)[3];
}

static void get_leaf_80000003(hpc_cpuid_t *c)
{
    int32_t eax=0x80000003;
    int32_t ebx=0x0;
    int32_t ecx=0x0;
    int32_t edx=0x0;

    asm("\n"
        "movl %4,%%eax;         \n\t"
        "movl $0,%%ebx;         \n\t"
        "movl %5,%%ecx;         \n\t"
        "movl $0,%%edx;         \n\t"
        "cpuid;                 \n\t"
        "movl %%eax,%0;         \n\t"
        "movl %%ebx,%1;         \n\t"
        "movl %%ecx,%2;         \n\t"
        "movl %%edx,%3;         \n\t"
        : "=m"(eax),"=m"(ebx),"=m"(ecx),"=m"(edx)
        : "m"(eax),"m"(ecx)
        : "%eax","%ebx","%ecx","%edx"
    );

    c->brand_string[16] = ((char*)&eax)[0];
    c->brand_string[17] = ((char*)&eax)[1];
    c->brand_string[18] = ((char*)&eax)[2];
    c->brand_string[19] = ((char*)&eax)[3];
    c->brand_string[20] = ((char*)&ebx)[0];
    c->brand_string[21] = ((char*)&ebx)[1];
    c->brand_string[22] = ((char*)&ebx)[2];
    c->brand_string[23] = ((char*)&ebx)[3];
    c->brand_string[24] = ((char*)&ecx)[0];
    c->brand_string[25] = ((char*)&ecx)[1];
    c->brand_string[26] = ((char*)&ecx)[2];
    c->brand_string[27] = ((char*)&ecx)[3];
    c->brand_string[28] = ((char*)&edx)[0];
    c->brand_string[29] = ((char*)&edx)[1];
    c->brand_string[20] = ((char*)&edx)[2];
    c->brand_string[31] = ((char*)&edx)[3];
}

static void get_leaf_80000004(hpc_cpuid_t *c)
{
    int32_t eax=0x80000004;
    int32_t ebx=0x0;
    int32_t ecx=0x0;
    int32_t edx=0x0;
    int i;

    asm("\n"
        "movl %4,%%eax;         \n\t"
        "movl $0,%%ebx;         \n\t"
        "movl %5,%%ecx;         \n\t"
        "movl $0,%%edx;         \n\t"
        "cpuid;                 \n\t"
        "movl %%eax,%0;         \n\t"
        "movl %%ebx,%1;         \n\t"
        "movl %%ecx,%2;         \n\t"
        "movl %%edx,%3;         \n\t"
        : "=m"(eax),"=m"(ebx),"=m"(ecx),"=m"(edx)
        : "m"(eax),"m"(ecx)
        : "%eax","%ebx","%ecx","%edx"
    );

    c->brand_string[32] = ((char*)&eax)[0];
    c->brand_string[33] = ((char*)&eax)[1];
    c->brand_string[34] = ((char*)&eax)[2];
    c->brand_string[35] = ((char*)&eax)[3];
    c->brand_string[36] = ((char*)&ebx)[0];
    c->brand_string[37] = ((char*)&ebx)[1];
    c->brand_string[38] = ((char*)&ebx)[2];
    c->brand_string[39] = ((char*)&ebx)[3];
    c->brand_string[40] = ((char*)&ecx)[0];
    c->brand_string[41] = ((char*)&ecx)[1];
    c->brand_string[42] = ((char*)&ecx)[2];
    c->brand_string[43] = ((char*)&ecx)[3];
    c->brand_string[44] = ((char*)&edx)[0];
    c->brand_string[45] = ((char*)&edx)[1];
    c->brand_string[46] = ((char*)&edx)[2];
    c->brand_string[47] = ((char*)&edx)[3];

    c->brand_string_left = c->brand_string;
    for (i=0; i < sizeof c->brand_string; i++) {
        if (' ' < c->brand_string_left[i]) break;
        c->brand_string_left += 1;
    }
}

static void get_leaf_80000005(hpc_cpuid_t *c)
{
                                // reserved
}

static void get_leaf_80000006(hpc_cpuid_t *c)
{
    int32_t eax=0x80000006;
    int32_t ebx=0x0;
    int32_t ecx=0x0;
    int32_t edx=0x0;

    asm("\n"
        "movl %4,%%eax;         \n\t"
        "movl $0,%%ebx;         \n\t"
        "movl %5,%%ecx;         \n\t"
        "movl $0,%%edx;         \n\t"
        "cpuid;                 \n\t"
        "movl %%eax,%0;         \n\t"
        "movl %%ebx,%1;         \n\t"
        "movl %%ecx,%2;         \n\t"
        "movl %%edx,%3;         \n\t"
        : "=m"(eax),"=m"(ebx),"=m"(ecx),"=m"(edx)
        : "m"(eax),"m"(ecx)
        : "%eax","%ebx","%ecx","%edx"
    );

    c->l2_cache_size    = (ecx >> 16) & 0xffff;
    switch((ecx>>12)&0xf) {
    case 0x0:
        c->l2_associativity = L2_DISABLED;
        c->l2_assoc_string  = "Disabled";
        break;
    case 0x1:
        c->l2_associativity = L2_DIRECT_MAPPED;
        c->l2_assoc_string  = "Direct mapped";
        break;
    case 0x2:
        c->l2_associativity = L2_2_WAY_ASSOCIATIVE;
        c->l2_assoc_string  = "2-Way";
        break;
    case 0x4:
        c->l2_associativity = L2_4_WAY_ASSOCIATIVE;
        c->l2_assoc_string  = "4-Way";
        break;
    case 0x6:
        c->l2_associativity = L2_8_WAY_ASSOCIATIVE;
        c->l2_assoc_string  = "8-Way";
        break;
    case 0x8:
        c->l2_associativity = L2_16_WAY_ASSOCIATIVE;
        c->l2_assoc_string  = "16-Way";
        break;
    case 0xf:
        c->l2_associativity = L2_FULLY_ASSOCIATIVE;
        c->l2_assoc_string  = "Fully associative";
        break;
    default:
        c->l2_associativity = L2_INVALID;
        c->l2_assoc_string  = "invalid L2 associativity";
        break;
    }
    c->l2_line_size     = (ecx >>  0) & 0xff;
}

static void get_leaf_80000007(hpc_cpuid_t *c)
{
    int32_t eax=0x80000007;
    int32_t ebx=0x0;
    int32_t ecx=0x0;
    int32_t edx=0x0;

    asm("\n"
        "movl %4,%%eax;         \n\t"
        "movl $0,%%ebx;         \n\t"
        "movl %5,%%ecx;         \n\t"
        "movl $0,%%edx;         \n\t"
        "cpuid;                 \n\t"
        "movl %%eax,%0;         \n\t"
        "movl %%ebx,%1;         \n\t"
        "movl %%ecx,%2;         \n\t"
        "movl %%edx,%3;         \n\t"
        : "=m"(eax),"=m"(ebx),"=m"(ecx),"=m"(edx)
        : "m"(eax),"m"(ecx)
        : "%eax","%ebx","%ecx","%edx"
    );

    c->const_rate_TSC = (edx >> 8) & 1;
}

static void get_leaf_80000008(hpc_cpuid_t *c)
{
    if (0x80000008 <= c->max_leaf) {
        int32_t eax=0x80000008;
        int32_t ebx=0x0;
        int32_t ecx=0x0;
        int32_t edx=0x0;

        asm("\n"
            "movl %4,%%eax;             \n\t"
            "movl $0,%%ebx;             \n\t"
            "movl %5,%%ecx;             \n\t"
            "movl $0,%%edx;             \n\t"
            "cpuid;                     \n\t"
            "movl %%eax,%0;             \n\t"
            "movl %%ebx,%1;             \n\t"
            "movl %%ecx,%2;             \n\t"
            "movl %%edx,%3;             \n\t"
            : "=m"(eax),"=m"(ebx),"=m"(ecx),"=m"(edx)
            : "m"(eax),"m"(ecx)
            : "%eax","%ebx","%ecx","%edx"
        );

        c->physical_address_size = (eax >> 0) & 0xff;
        c->virtual_address_size  = (eax >> 8) & 0xff;
    }
}

#if defined(__KERNEL__)
#include <linux/jiffies.h>

                                // delay 200 jiffies or 1/2 second, whichever is less
#define MAX_DELAY_SEC   (HZ/2)
#define MAX_DELAY_JIF   (200)
#define MIN(x,y)        (((x)<(y))?(x):(y))
#define DELAY_JIFFIES   MIN(MAX_DELAY_SEC,MAX_DELAY_JIF)

static void calculate_tsc_per_sec(void);
static uint64_t _tsc_per_sec = 0;
static uint64_t _tsc_per_jiffy = 0;

uint64_t tsc_per_sec(void)
{
    if (_tsc_per_sec == 0) {
        calculate_tsc_per_sec();
    }

    return _tsc_per_sec;
} 

uint64_t tsc_per_jiffy(void)
{
    if (_tsc_per_sec == 0) {
        calculate_tsc_per_sec();
    }

    return _tsc_per_jiffy;
} 

static void calculate_tsc_per_sec(void)
{
    unsigned long b_jif;
    unsigned long e_jif;
    uint64_t b_tsc, e_tsc;
    int64_t j_delta, t_delta;
    unsigned int b_eax=0, b_edx=0;
    unsigned int e_eax=0, e_edx=0;

                                // wait until the start of the next jiffy
    b_jif = jiffies;
    while (time_before(jiffies, b_jif+1)) {
        ;
    }

                                // read the current jiffy
    b_jif = jiffies;
    e_jif = jiffies + DELAY_JIFFIES;

                                // read the time stamp counter
    __asm__ __volatile__(
        "rdtsc ;"
        "movl %%eax,%0;"
        "movl %%edx,%1;"
        ""
        : "=r"(b_eax), "=r"(b_edx)
        :
        : "%eax", "%edx"
    );

                                // spin wait for DELAY_JIFFIES
    while (time_before(jiffies, e_jif)) {
        ;
    }

                                // read the time stamp counter again
    __asm__ __volatile__(
        "rdtsc ;"
        "movl %%eax,%0;"
        "movl %%edx,%1;"
        ""
        : "=r"(e_eax), "=r"(e_edx)
        :
        : "%eax", "%edx"
    );

                                // read the current jiffy again
    e_jif = jiffies;

                                // calculate time stamps, deltas and translation factors
    b_tsc = ((int64_t) b_edx << 32) | (int64_t) b_eax;
    e_tsc = ((int64_t) e_edx << 32) | (int64_t) e_eax;

    t_delta = e_tsc - b_tsc;
    j_delta = e_jif - b_jif;

    _tsc_per_sec   = (t_delta * HZ) / j_delta;
    _tsc_per_jiffy = t_delta / j_delta;
}

#else

#include <sys/time.h>
#include <unistd.h>

uint64_t processor_clock_Hz(void)
{
    struct timeval btv,etv;
    int64_t b_usec, e_usec, btsc, etsc;
    int64_t usec, tsc;
    unsigned int eax=0, edx=0;

    gettimeofday(&btv,NULL);

    __asm__ __volatile__(
        "rdtsc ;"
        "movl %%eax,%0;"
        "movl %%edx,%1;"
        ""
        : "=r"(eax), "=r"(edx)
        :
        : "%eax", "%edx"
    );
    btsc = ((int64_t) edx << 32) | (int64_t) eax;

    sleep(1);

    __asm__ __volatile__(
        "rdtsc ;"
        "movl %%eax,%0;"
        "movl %%edx,%1;"
        ""
        : "=r"(eax), "=r"(edx)
        :
        : "%eax", "%edx"
    );
    etsc = ((int64_t) edx << 32) | (int64_t) eax;

    gettimeofday(&etv,NULL);

    b_usec = btv.tv_sec*1000000 + btv.tv_usec;
    e_usec = etv.tv_sec*1000000 + etv.tv_usec;

    usec = e_usec - b_usec;
    tsc  = etsc   - btsc;

    return (uint64_t) (((1000000L * tsc)/usec));
}
#endif


void
print_cpuid(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
    if (c == NULL || print == NULL) return;

    print("CPUID enabled: %d\n", c->cpuid_enabled);
    if (! c->cpuid_enabled) return;

    print_leaf_00000000(c, print);      // 0x00000000
    print_leaf_00000001(c, print);      // 0x00000001
    print_leaf_00000002(c, print);      // 0x00000002
    print_leaf_00000003(c, print);      // 0x00000003
    print_leaf_00000004(c, print);      // 0x00000004
    print_leaf_00000005(c, print);      // 0x00000005
    print_leaf_00000006(c, print);      // 0x00000006
    print_leaf_00000007(c, print);      // 0x00000007
    print_leaf_00000008(c, print);      // 0x00000008
    print_leaf_00000009(c, print);      // 0x00000009
    print_leaf_0000000A(c, print);      // 0x0000000A
    print_leaf_0000000B(c, print);      // 0x0000000B
    print_leaf_0000000C(c, print);      // 0x0000000C
    print_leaf_0000000D(c, print);      // 0x0000000D

    print_leaf_80000000(c, print);      // 0x80000000
    print_leaf_80000001(c, print);      // 0x80000001
    print_leaf_80000002(c, print);      // 0x80000002
    print_leaf_80000003(c, print);      // 0x80000003
    print_leaf_80000004(c, print);      // 0x80000004
    print_leaf_80000005(c, print);      // 0x80000005
    print_leaf_80000006(c, print);      // 0x80000006
    print_leaf_80000007(c, print);      // 0x80000007
    print_leaf_80000008(c, print);      // 0x80000008
}

static void print_leaf_00000000(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
    print("Product string: %-16s\n", c->product_string);
}

static char* brand_name_table[] = {
    NULL,                                                                                       // 0x00
    "Intel Celeron processor",                                                                  // 0x01
    "Intel Pentium III processor",                                                              // 0x02
    "Intel Pentium III Xeon processor"
    "If processor signature = 0x000006B1, then Intel Celeron processor",                        // 0x03
    "Intel Pentium III processor",                                                              // 0x04
    NULL,                                                                                       // 0x05
    "Mobile Intel Pentium III processor-M",                                                     // 0x06
    "Mobile Intel Celeron Processor",                                                           // 0x07
    "Intel Pentium 4 processor",                                                                // 0x08
    "Intel Pentium 4 processor",                                                                // 0x09
    "Mobile Intel Celeron Processor",                                                           // 0x0A
    "Intel Xeon processor"
    "If processor signature = 0x00000F13, then Intel Xeon processor",                           // 0x0B
    "Intel Xeon processor MP",                                                                  // 0x0C
    NULL,                                                                                       // 0x0D
    "Mobile Intel Pentium 4 processor-M"
    "If processor signature = 0x00000F13, then Intel Xeon processor",                           // 0x0E
    "Mobile Intel Celeron Processor",                                                           // 0x0F
    NULL,                                                                                       // 0x10
    "Mobile Genuine Intel Processor",                                                           // 0x11
    "Intel Celeron M processor",                                                                // 0x12
    "Mobile Intel Celeron processor",                                                           // 0x13
    "Intel Celeron processor",                                                                  // 0x14
    "Mobile Genuine Intel processor",                                                           // 0x15
    "Intel Pentium M processor",                                                                // 0x16
    "Mobile Intel Celeron processor",                                                           // 0x17
};

static void print_leaf_00000001(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
    printf("Stepping ID:     0x%x\n", c->stepping_id);
    printf("Model Number:    0x%x\n", c->model_number);
    printf("Family Code:     0x%x\n", c->family_code);
    printf("Processor Type:  0x%x\n", c->processor_type);
    printf("Extended Model:  0x%x\n", c->extended_model);
    printf("Extended Family: 0x%x\n", c->extended_family);

    printf("Brand ID:        0x%x\n", c->brand_id);
    printf("Chuncks:         0x%x\n", c->chunks);
    printf("HTT Thread Count 0x%x\n", c->count);
    printf("APIC ID:         0x%x\n", c->apic_id);

    printf("brand_id          0x%x\n", c->brand_id);
    printf("sizeof BNT        0x%x\n", sizeof brand_name_table);
    printf("sizeof BNT[0]     0x%x\n", sizeof brand_name_table[0]);
    printf("sizeof BNT/BNT[0] 0x%x\n", (sizeof brand_name_table)/(sizeof brand_name_table[0]));

    if (c->brand_id < (sizeof brand_name_table)/(sizeof brand_name_table[0]) && brand_name_table[c->brand_id] != NULL) {
        printf("%s\n", brand_name_table[c->brand_id]);
    }

    printf("\n");
    if (c->SSE3        ) printf("SSE3 ");
    if (c->PCLMULDQ    ) printf("PCLMULDQ ");
    if (c->DTES64      ) printf("DTES64 ");
    if (c->MONITOR     ) printf("MONITOR ");
    if (c->DS_CPL      ) printf("DS-CPL ");
    if (c->VMX         ) printf("VMX ");
    if (c->SMX         ) printf("SMX ");
    if (c->EIST        ) printf("EIST ");
    if (c->TM2         ) printf("TM2 ");
    if (c->SSSE3       ) printf("SSSE3 ");
    if (c->CNXT_ID     ) printf("CNXT-ID ");
    if (false) ;	// reserved
    if (c->FMA         ) printf("FMA ");
    if (c->CX16        ) printf("CX16 ");
    if (c->xTPR        ) printf("xTPR ");
    if (c->PDCM        ) printf("PDCM ");
    if (false) ;	// reserved
    if (c->PDCM        ) printf("PDCM ");
    if (c->DCA         ) printf("DCA ");
    if (c->SSE4_1      ) printf("SSE4.1 ");
    if (c->SSE4_2      ) printf("SSE4.2 ");
    if (c->x2APIC      ) printf("x2APIC ");
    if (c->MOVBE       ) printf("MOVBE ");
    if (c->POPCT       ) printf("POPCNT ");
    if (c->TSC_DEADLINE) printf("TSC-DEADLINE ");
    if (c->AES         ) printf("AES ");
    if (c->XSAVE       ) printf("XSAVE ");
    if (c->OSXSAVE     ) printf("OSXSAVE ");
    if (c->AVX         ) printf("AVX ");
    if (false) ;	// reserved
    if (false) ;	// reserved
    if (false) ;	// reserved

    if (c->FPU         ) printf("FPU ");
    if (c->VME         ) printf("VME ");
    if (c->DE          ) printf("DE ");
    if (c->PSE         ) printf("PSE ");
    if (c->TSC         ) printf("TSC ");
    if (c->MSR         ) printf("MSR ");
    if (c->PAE         ) printf("PAE ");
    if (c->MCE         ) printf("MCE ");
    if (c->CX8         ) printf("CX8 ");
    if (c->APIC        ) printf("APIC ");
    if (false) ;	// reserved
    if (c->SEP         ) printf("SEP ");
    if (c->MTRR        ) printf("MTRR ");
    if (c->PGE         ) printf("PGE ");
    if (c->MCA         ) printf("MCA ");
    if (c->CMOV        ) printf("CMOV ");
    if (c->PAT         ) printf("PAT ");
    if (c->PSE_36      ) printf("PSE-36 ");
    if (c->PSN         ) printf("PSN ");
    if (c->CLFSH       ) printf("CLFSH ");
    if (false) ;        // reserved
    if (c->DS          ) printf("DS ");
    if (c->ACPI        ) printf("ACPI ");
    if (c->MMX         ) printf("MMX ");
    if (c->FXSR        ) printf("FXSR ");
    if (c->SSE         ) printf("SSE ");
    if (c->SSE2        ) printf("SSE2 ");
    if (c->SS          ) printf("SS ");
    if (c->HTT         ) {
        printf("HTT ");
        if (c->HTT_Count > 0) {
            printf("MT=YES ");
        } else {
            printf("MT=NO ");
        }
    }
    if (c->TM          ) printf("TM ");
    if (false) ;	// reserved
    if (c->PBE         ) printf("PBE ");
    printf("\n");
}

static void print_leaf_00000002(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
    int i; 

    for (i=0; i < c->num_cache_descriptors; i++) {
        print("%s\n", c->cache_descriptor_text[i]);
    }
}

static void print_leaf_00000003(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
}

static void print_leaf_00000004(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
#if defined(UNDEFINED)
    int i;

    for (i=0; i < c->cache_levels; i++) {
        switch (c->leaf_4[i].cache_type) {
        case 0:
            print("Cache %d: Null, ", i);
            break;
        case 1:
            print("Cache %d: Data Cache, ", i);
            break;
        case 2:
            print("Cache %d: Instruction Cache, ", i);
            break;
        case 3:
            print("Cache %d: Unified Cache, ", i);
            break;
        default:
            print("Cache %d: Reserved (%d), ", i, c->leaf_4[i].cache_type);
            break;
        }

        print("APIC IDs=%d, ",                   c->leaf_4[i].reserved_apic_ids);
        print("Max threads sharing cache=%d, ",  c->leaf_4[i].max_threads_sharing_cache);
        print("%s assoc, ",                      c->leaf_4[i].fully_associative ? "fully" : "NOT fully");
        print("%s-initializing, ",               c->leaf_4[i].self_initializing ? "self" : "NOT self");
        print("Level=%d, ",                      c->leaf_4[i].cache_level);
        print("%d-way set assoc, ",              c->leaf_4[i].associativity);
        print("Physical line partitions=%d, ",   c->leaf_4[i].physical_line_partitions);
        print("System Coherency Line Size=%d, ", c->leaf_4[i].system_coherency_line_size);
        print("Sets=%d, ",                       c->leaf_4[i].number_of_sets);
        print("%s Mapped Cache, ",               c->leaf_4[i].complex_cache_indexing ? "Direct" : "NOT Direct");
        print("%s inclusive, ",                  c->leaf_4[i].inclusive              ? "IS" : "NOT");
        print("WBINVD/INVD=%s, ",                !c->leaf_4[i].inclusive_invalidate ? "YES" : "NO");

        print("Size=%d KiB\n",                   c->leaf_4[i].cache_size/1024);
    }
#endif
    int i;

    print("Cache, Cache Type, APIC IDs, Threads Sharing, Fully Associative, Self-Initializing, Level, Associativity, Physical Line Partitions, Coherency Line Size, Sets, Direct Mapped, Inclusive, WBINDV/INVD, Size (KiB)\n");
    for (i=0; i < c->cache_levels; i++) {
        print("%d, ", i);
        switch (c->leaf_4[i].cache_type) {
        case 0: print("   \"null\", "); break;
        case 1: print("   \"data\", "); break;
        case 2: print("   \"inst\", "); break;
        case 3: print("\"unified\", "); break;
        default: print("\"reserved (%d)\", ", c->leaf_4[i].cache_type); break;
        }
        print("%d, ",     c->leaf_4[i].reserved_apic_ids);
        print("%d, ",     c->leaf_4[i].max_threads_sharing_cache);
        print("\"%s\", ", c->leaf_4[i].fully_associative ? "yes" : "no");
        print("\"%s\", ", c->leaf_4[i].self_initializing ? "yes" : "no");
        print("%d, ",     c->leaf_4[i].cache_level);
        print("%d, ",     c->leaf_4[i].associativity);
        print("%d, ",     c->leaf_4[i].physical_line_partitions);
        print("%d, ",     c->leaf_4[i].system_coherency_line_size);
        print("%d, ",     c->leaf_4[i].number_of_sets);
        print("\"%s\", ", c->leaf_4[i].complex_cache_indexing ? "yes" : "no");
        print("\"%s\", ", c->leaf_4[i].inclusive              ? "yes" : "no");
        print("\"%s\", ", !c->leaf_4[i].inclusive_invalidate  ? "yes" : "no");

        print("%d\n",     c->leaf_4[i].cache_size/1024);
    }
}

static void print_leaf_00000005(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
}

static void print_leaf_00000006(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
    print("Processor %s have Package Thermal Management (PTM) capability\n",      c->PTM        ? "DOES" : "DOES NOT");
    print("Processor %s have Extended Clock Modulation Duty (ECMD) capability\n", c->ECMD       ? "DOES" : "DOES NOT");
    print("Processor %s have Power Limit Notification (PLN) capability\n",        c->PLN        ? "DOES" : "DOES NOT");
    print("Processor %s have Always Running APIC Timer (ARAT) capability\n",      c->ARAT       ? "DOES" : "DOES NOT");
    print("Processor %s have Intel Turbo Boost Technology capability\n",          c->TurboBoost ? "DOES" : "DOES NOT");
    print("Processor %s have Digital Thermal Sensor (DTS) capability\n",          c->DTS        ? "DOES" : "DOES NOT");
    print("Number of Interrupt Thresholds is %d\n",                               c->interrupt_threshholds);
    print("Processor %s have Performance-Energy Bias capability (presence of IA32_ENERGY_PERF_IBAS MSR) capability\n",
        c->IA32_ENERGY_PERF_BIAS_MSR ? "DOES" : "DOES NOT");
    print("Processor %s have Hardware Coordination Feedback capability (presence of IA32_APERF, IA32_MPERF MSRs)\n",
        c->IA32_PERF_MSRs ? "DOES" : "DOES NOT");
}

static void print_leaf_00000007(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
}

static void print_leaf_00000008(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
}

static void print_leaf_00000009(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
    print("Value of PLATFORM_DCA_CAP MSR Bits[31:0](Offset 1F8h)\n", c->PLATFORM_DCA_CAP_MSR);
}

static void print_leaf_0000000A(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
    printf("Architectural PerfMon Version                         : %d\n", c->arch_perfmon_version);
    printf("Number of counters per logical processor              : %d\n", c->counters_per_core);
    printf("Number of bits per programmable counter (width)       : %d\n", c->bits_per_counter);
    printf("Number of arch events supported per logical processor : %d\n", c->arch_events_per_core);

    printf("Branch Mispredicts Retired                            : %s\n", c->branch_miss_retired ? "NOT Supported" : "Supported" );
    printf("Branch Instructions Retired                           : %s\n", c->branch_inst_retired ? "NOT Supported" : "Supported" );
    printf("Last Level Cach Misses                                : %s\n", c->LLC_misses          ? "NOT Supported" : "Supported" );
    printf("Last Level Cach References                            : %s\n", c->LLC_refs            ? "NOT Supported" : "Supported" );
    printf("Reference Cycles                                      : %s\n", c->ref_cycles          ? "NOT Supported" : "Supported" );
    printf("Instructions Retired                                  : %s\n", c->inst_retired        ? "NOT Supported" : "Supported" );
    printf("Core Cycles                                           : %s\n", c->core_cycles         ? "NOT Supported" : "Supported" );

    printf("Number of Bits in the Fixed Counters (width)          : %d\n", c->bits_in_fixed_counters);
    printf("Number of Fixed Counters                              : %d\n", c->fixed_counters);
}

static void print_leaf_0000000B(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
}

static void print_leaf_0000000C(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
}

static void print_leaf_0000000D(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
}

static void print_leaf_80000000(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
    print("Max leaf: 0x%-8x\n", c->max_leaf);
}

static void print_leaf_80000001(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
    if (0x80000001 <= c->max_leaf) {
        int before = false;
        if (c->LAHF_SAHF   ) {
            print("LAHF/SAHF");
            before = true;
        }
        if (c->SYSCALL     ) {
            if (before) print(", ");
            print("SYSCALL");
            before = true;
        }
        if (c->XD_bit      ) {
            if (before) print(", ");
            print("XD bit");
            before = true;
        }
        if (c->one_GB_pages) {
            if (before) print(", ");
            print("1 GB Pages");
            before = true;
        }
        if (c->RDTSCP      ) {
            if (before) print(", ");
            print("RDTSCP");
            before = true;
        }
        if (c->Intel64     ) {
            if (before) print(", ");
            print("Intel 64");
            before = true;
        }
        print("\n");
    }
}

static void print_leaf_80000002(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
    if (0x80000002 <= c->max_leaf && c->brand_string_supported) {
        print("Brand string: %s\n", c->brand_string_left);
    }
}

static void print_leaf_80000003(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
    if (0x80000003 <= c->max_leaf) {
    }
}

static void print_leaf_80000004(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
    if (0x80000004 <= c->max_leaf) {
    }
}

static void print_leaf_80000005(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
    if (0x80000005 <= c->max_leaf) {
    }
}

static void print_leaf_80000006(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
    if (0x80000006 <= c->max_leaf) {
    }
}

static void print_leaf_80000007(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
    if (0x80000007 <= c->max_leaf) {
        printf("TSC %s run at a constant rate in all ACPI P-states, C-states and T-states.\n", c->const_rate_TSC ? "WILL" : "WILL NOT");
    }
}

static void print_leaf_80000008(hpc_cpuid_t *c, int (*print)(const char* format, ...))
{
    if (0x80000008 <= c->max_leaf) {
        printf("Physical Address Size: %d bits\n", c->physical_address_size);
        printf("Virtual  Address Size: %d bits\n", c->virtual_address_size);
    }
}

static char* cache_descriptor_table[] = {
    "Null Descriptor. This byte contains no information.",                                      // 0x00
    "Instruction TLB: 4-KB Pages, 4-way set associative, 32 entries",                           // 0x01
    "Instruction TLB: 4-MB Pages, fully associative, 2 entries",                                // 0x02
    "Data TLB: 4-KB Pages, 4-way set associative, 64 entries",                                  // 0x03
    "Data TLB: 4-MB Pages, 4-way set associative, 8 entries",                                   // 0x04
    "Data TLB: 4-MB Pages, 4-way set associative, 32 entries",                                  // 0x05
    "1st-level instruction cache: 8-KB, 4-way set associative, 32-byte line size",              // 0x06
    NULL,                                                                                       // 0x07
    "1st-level instruction cache: 16-KB, 4-way set associative, 32-byte line size",             // 0x08
    "1st-level instruction cache: 32-KB, 4-way set associative, 64-byte line size",             // 0x09
    "1st-level instruction cache: 8-KB, 2-way set associative, 32-byte line size",              // 0x0A
    "Instruction TLB: 4-MB Pages, 4-way set associative, 4 entries",                            // 0x0B
    "1st-level data cache: 16-KB, 4-way set associative, 32-byte line size",                    // 0x0C
    "1st-level data cache: 16-KB, 4-way set associative, 64-byte line size, ECC",               // 0x0D
    "1st-level data cache: 24-KB, 6-way set associative, 64-byte line size, ECC",               // 0x0E
    NULL,                                                                                       // 0x0F
    NULL,                                                                                       // 0x10
    NULL,                                                                                       // 0x11
    NULL,                                                                                       // 0x12
    NULL,                                                                                       // 0x13
    NULL,                                                                                       // 0x14
    NULL,                                                                                       // 0x15
    NULL,                                                                                       // 0x16
    NULL,                                                                                       // 0x17
    NULL,                                                                                       // 0x18
    NULL,                                                                                       // 0x19
    NULL,                                                                                       // 0x1A
    NULL,                                                                                       // 0x1B
    NULL,                                                                                       // 0x1C
    NULL,                                                                                       // 0x1D
    NULL,                                                                                       // 0x1E
    NULL,                                                                                       // 0x1F
    NULL,                                                                                       // 0x20
    "2nd-level cache: 256-KB, 8-way set associative, 64-byte line size",                        // 0x21
    "3rd-level cache: 512-KB, 4-way set associative, sectored cache, 64-byte line size",        // 0x22
    "3rd-level cache: 1-MB, 8-way set associative, sectored cache, 64-byte line size",          // 0x23
    NULL,                                                                                       // 0x24
    "3rd-level cache: 2-MB, 8-way set associative, sectored cache, 64-byte line size",          // 0x25
    NULL,                                                                                       // 0x26
    NULL,                                                                                       // 0x27
    NULL,                                                                                       // 0x28
    "3rd-level cache: 4-MB, 8-way set associative, sectored cache, 64-byte line size",          // 0x29
    NULL,                                                                                       // 0x2A
    NULL,                                                                                       // 0x2B
    "1st-level data cache: 32-KB, 8-way set associative, 64-byte line size",                    // 0x2C
    NULL,                                                                                       // 0x2D
    NULL,                                                                                       // 0x2E
    NULL,                                                                                       // 0x2F
    "1st-level instruction cache: 32-KB, 8-way set associative, 64-byte line size",             // 0x30
    NULL,                                                                                       // 0x31
    NULL,                                                                                       // 0x32
    NULL,                                                                                       // 0x33
    NULL,                                                                                       // 0x34
    NULL,                                                                                       // 0x35
    NULL,                                                                                       // 0x36
    NULL,                                                                                       // 0x37
    NULL,                                                                                       // 0x38
    NULL,                                                                                       // 0x39
    NULL,                                                                                       // 0x3A
    NULL,                                                                                       // 0x3B
    NULL,                                                                                       // 0x3C
    NULL,                                                                                       // 0x3D
    NULL,                                                                                       // 0x3E
    NULL,                                                                                       // 0x3F
    "No 2nd-level cache or, if processor contains a valid 2nd-level cache, no 3rd-level cache", // 0x40
    "2nd-level cache: 128-KB, 4-way set associative, 32-byte line size",                        // 0x41
    "2nd-level cache: 256-KB, 4-way set associative, 32-byte line size",                        // 0x42
    "2nd-level cache: 512-KB, 4-way set associative, 32-byte line size",                        // 0x43
    "2nd-level cache: 1-MB, 4-way set associative, 32-byte line size",                          // 0x44
    "2nd-level cache: 2-MB, 4-way set associative, 32-byte line size",                          // 0x45
    "3rd-level cache: 4-MB, 4-way set associative, 64-byte line size",                          // 0x46
    "3rd-level cache: 8-MB, 8-way set associative, 64-byte line size",                          // 0x47
    "2nd-level cache: 3-MB, 12-way set associative, 64-byte line size, unified on-die",         // 0x48
    "3rd-level cache: 4-MB, 16-way set associative, 64-byte line size "
    "(Intel Xeon processor MP, Family 0Fh, Model 06h)\n"
    "2nd-level cache: 4-MB, 16-way set associative, 64-byte line size",                         // 0x49
    "3rd-level cache: 6-MB, 12-way set associative, 64-byte line size",                         // 0x4A
    "3rd-level cache: 8-MB, 16-way set associative, 64-byte line size",                         // 0x4B
    "3rd-level cache: 12-MB, 12-way set associative, 64-byte line size",                        // 0x4C
    "3rd-level cache: 16-MB, 16-way set associative, 64-byte line size",                        // 0x4D
    "2nd-level cache: 6-MB, 24-way set associative, 64-byte line size",                         // 0x4E
    "Instruction TLB: 4-KB pages, 32 entries",                                                  // 0x4F
    "Instruction TLB: 4-KB, 2-MB or 4-MB pages, fully associative, 64 entries",                 // 0x50
    "Instruction TLB: 4-KB, 2-MB or 4-MB pages, fully associative, 128 entries",                // 0x51
    "Instruction TLB: 4-KB, 2-MB or 4-MB pages, fully associative, 256 entries",                // 0x52
    NULL,                                                                                       // 0x53
    NULL,                                                                                       // 0x54
    "Instruction TLB: 2-MB or 4-MB pages, fully associative, 7 entries",                        // 0x55
    "L1 Data TLB: 4-MB pages, 4-way set associative, 16 entries",                               // 0x56
    "L1 Data TLB: 4-KB pages, 4-way set associative, 16 entries",                               // 0x57
    NULL,                                                                                       // 0x58
    "Data TLB0: 4-KB pages, fully associative, 16 entries",                                     // 0x59
    "Data TLB0: 2-MB or 4-MB pages, 4-way set associative, 32 entries",                         // 0x5A
    "Data TLB: 4-KB or 4-MB pages, fully associative, 64 entries",                              // 0x5B
    "Data TLB: 4-KB or 4-MB pages, fully associative, 128 entries",                             // 0x5C
    "Data TLB: 4-KB or 4-MB pages, fully associative, 256 entries",                             // 0x5D
    NULL,                                                                                       // 0x5E
    NULL,                                                                                       // 0x5F
    "1st-level data cache: 16-KB, 8-way set associative, sectored cache, 64-byte line size",    // 0x60
    NULL,                                                                                       // 0x61
    NULL,                                                                                       // 0x62
    NULL,                                                                                       // 0x63
    NULL,                                                                                       // 0x64
    NULL,                                                                                       // 0x65
    "1st-level data cache: 8-KB, 4-way set associative, sectored cache, 64-byte line size",     // 0x66
    "1st-level data cache: 16-KB, 4-way set associative, sectored cache, 64-byte line size",    // 0x67
    "1st-level data cache: 32-KB, 4-way set associative, sectored cache, 64-byte line size",    // 0x68
    NULL,                                                                                       // 0x69
    NULL,                                                                                       // 0x6A
    NULL,                                                                                       // 0x6B
    NULL,                                                                                       // 0x6C
    NULL,                                                                                       // 0x6D
    NULL,                                                                                       // 0x6E
    NULL,                                                                                       // 0x6F
    "Trace cache: 12K-uops, 8-way set associative",                                             // 0x70
    "Trace cache: 16K-uops, 8-way set associative",                                             // 0x71
    "Trace cache: 32K-uops, 8-way set associative",                                             // 0x72
    NULL,                                                                                       // 0x73
    NULL,                                                                                       // 0x74
    NULL,                                                                                       // 0x75
    "2nd-level cache: 1-MB, 4-way set associative, 64-byte line size",                          // 0x76
    NULL,                                                                                       // 0x77
    "2nd-level cache: 1-MB, 4-way set associative, 64-byte line size",                          // 0x78
    "2nd-level cache: 128-KB, 8-way set associative, sectored cache, 64-byte line size",        // 0x79
    "2nd-level cache: 256-KB, 8-way set associative, sectored cache, 64-byte line size",        // 0x7A
    "2nd-level cache: 512-KB, 8-way set associative, sectored cache, 64-byte line size",        // 0x7B
    "2nd-level cache: 1-MB, 8-way set associative, sectored cache, 64-byte line size",          // 0x7C
    "2nd-level cache: 2-MB, 8-way set associative, sectored cache, 64-byte line size",          // 0x7D
    NULL,                                                                                       // 0x7E
    "2nd-level cache: 512-KB, 2-way set associative, 64-byte line size",                        // 0x7F
    "2nd-level cache: 512-KB, 8-way set associative, 64-byte line size",                        // 0x80
    NULL,                                                                                       // 0x81
    "2nd-level cache: 256-KB, 8-way set associative, 32-byte line size",                        // 0x82
    "2nd-level cache: 512-KB, 8-way set associative, 32-byte line size",                        // 0x83
    "2nd-level cache: 1-MB, 8-way set associative, 32-byte line size",                          // 0x84
    "2nd-level cache: 2-MB, 8-way set associative, 32-byte line size",                          // 0x85
    "2nd-level cache: 512-KB, 4-way set associative, 64-byte line size",                        // 0x86
    "2nd-level cache: 1-MB, 4-way set associative, 64-byte line size",                          // 0x87
    NULL,                                                                                       // 0x88
    NULL,                                                                                       // 0x89
    NULL,                                                                                       // 0x8A
    NULL,                                                                                       // 0x8B
    NULL,                                                                                       // 0x8C
    NULL,                                                                                       // 0x8D
    NULL,                                                                                       // 0x8E
    NULL,                                                                                       // 0x8F
    NULL,                                                                                       // 0x90
    NULL,                                                                                       // 0x91
    NULL,                                                                                       // 0x92
    NULL,                                                                                       // 0x93
    NULL,                                                                                       // 0x94
    NULL,                                                                                       // 0x95
    NULL,                                                                                       // 0x96
    NULL,                                                                                       // 0x97
    NULL,                                                                                       // 0x98
    NULL,                                                                                       // 0x99
    NULL,                                                                                       // 0x9A
    NULL,                                                                                       // 0x9B
    NULL,                                                                                       // 0x9C
    NULL,                                                                                       // 0x9D
    NULL,                                                                                       // 0x9E
    NULL,                                                                                       // 0x9F
    NULL,                                                                                       // 0xA0
    NULL,                                                                                       // 0xA1
    NULL,                                                                                       // 0xA2
    NULL,                                                                                       // 0xA3
    NULL,                                                                                       // 0xA4
    NULL,                                                                                       // 0xA5
    NULL,                                                                                       // 0xA6
    NULL,                                                                                       // 0xA7
    NULL,                                                                                       // 0xA8
    NULL,                                                                                       // 0xA9
    NULL,                                                                                       // 0xAA
    NULL,                                                                                       // 0xAB
    NULL,                                                                                       // 0xAC
    NULL,                                                                                       // 0xAD
    NULL,                                                                                       // 0xAE
    NULL,                                                                                       // 0xAF
    "Instruction TLB: 4-KB pages, 4-way set associative, 128 entries",                          // 0xB0
    "Instruction TLB: 2-MB pages, 4-way, 8 entries or 4M pages, 4-way, 4 entries",              // 0xB1
    "Instruction TLB: 4-KB pages, 4-way set associative, 64 entries",                           // 0xB2
    "Data TLB: 4-KB pages, 4-way set associative, 128 entries",                                 // 0xB3
    "Data TLB: 4-KB pages, 4-way set associative, 256 entries",                                 // 0xB4
    NULL,                                                                                       // 0xB5
    NULL,                                                                                       // 0xB6
    NULL,                                                                                       // 0xB7
    NULL,                                                                                       // 0xB8
    NULL,                                                                                       // 0xB9
    "Data TLB: 4-KB pages, 4-way set associative, 64 entries",                                  // 0xBA
    NULL,                                                                                       // 0xBB
    NULL,                                                                                       // 0xBC
    NULL,                                                                                       // 0xBD
    NULL,                                                                                       // 0xBE
    NULL,                                                                                       // 0xBF
    "Data TLB: 4-KB or 4-MB Pages, 4-way set associative, 8 entries",                           // 0xC0
    NULL,                                                                                       // 0xC1
    NULL,                                                                                       // 0xC2
    NULL,                                                                                       // 0xC3
    NULL,                                                                                       // 0xC4
    NULL,                                                                                       // 0xC5
    NULL,                                                                                       // 0xC6
    NULL,                                                                                       // 0xC7
    NULL,                                                                                       // 0xC8
    NULL,                                                                                       // 0xC9
    "Shared 2nd-level TLB: 4 KB pages, 4-way set associative, 512 entries",                     // 0xCA
    NULL,                                                                                       // 0xCB
    NULL,                                                                                       // 0xCC
    NULL,                                                                                       // 0xCD
    NULL,                                                                                       // 0xCE
    NULL,                                                                                       // 0xCF
    "3rd-level cache: 512-kB, 4-way set associative, 64-byte line size",                        // 0xD0
    "3rd-level cache: 1-MB, 4-way set associative, 64-byte line size",                          // 0xD1
    "3rd-level cache: 2-MB, 4-way set associative, 64-byte line size",                          // 0xD2
    NULL,                                                                                       // 0xD3
    NULL,                                                                                       // 0xD4
    NULL,                                                                                       // 0xD5
    "3rd-level cache: 1-MB, 8-way set associative, 64-byte line size",                          // 0xD6
    "3rd-level cache: 2-MB, 8-way set associative, 64-byte line size",                          // 0xD7
    "3rd-level cache: 4-MB, 8-way set associative, 64-byte line size",                          // 0xD8
    NULL,                                                                                       // 0xD9
    NULL,                                                                                       // 0xDA
    NULL,                                                                                       // 0xDB
    "3rd-level cache: 1.5-MB, 12-way set associative, 64-byte line size",                       // 0xDC
    "3rd-level cache: 3-MB, 12-way set associative, 64-byte line size",                         // 0xDD
    "3rd-level cache: 6-MB, 12-way set associative, 64-byte line size",                         // 0xDE
    NULL,                                                                                       // 0xDF
    NULL,                                                                                       // 0xE0
    NULL,                                                                                       // 0xE1
    "3rd-level cache: 2-MB, 16-way set associative, 64-byte line size",                         // 0xE2
    "3rd-level cache: 4-MB, 16-way set associative, 64-byte line size",                         // 0xE3
    "3rd-level cache: 8-MB, 16-way set associative, 64-byte line size",                         // 0xE4
    NULL,                                                                                       // 0xE5
    NULL,                                                                                       // 0xE6
    NULL,                                                                                       // 0xE7
    NULL,                                                                                       // 0xE8
    NULL,                                                                                       // 0xE9
    "3rd-level cache: 12-MB, 24-way set associative, 64-byte line size",                        // 0xEA
    "3rd-level cache: 18-MB, 24-way set associative, 64-byte line size",                        // 0xEB
    "3rd-level cache: 24-MB, 24-way set associative, 64-byte line size",                        // 0xEC
    NULL,                                                                                       // 0xED
    NULL,                                                                                       // 0xEE
    NULL,                                                                                       // 0xEF
    "64-byte Prefetching",                                                                      // 0xF0
    "128-byte Prefetching",                                                                     // 0xF1
    NULL,                                                                                       // 0xF2
    NULL,                                                                                       // 0xF3
    NULL,                                                                                       // 0xF4
    NULL,                                                                                       // 0xF5
    NULL,                                                                                       // 0xF6
    NULL,                                                                                       // 0xF7
    NULL,                                                                                       // 0xF8
    NULL,                                                                                       // 0xF9
    NULL,                                                                                       // 0xFA
    NULL,                                                                                       // 0xFB
    NULL,                                                                                       // 0xFC
    NULL,                                                                                       // 0xFD
    NULL,                                                                                       // 0xFE
    "CPUID Leaf 2 does not report cache descriptor information; "
    "use CPUID Leaf 4 to query cache parameters",                                               // 0xFF
};


#if defined(MAIN)
#include <stdio.h>
#include <unistd.h>
int
main(int ac, char* av[])
{
    hpc_cpuid_t c;

    get_cpuid(&c);
    print_cpuid(&c, printf);
    printf("CPU core frequency is %.2lf MHz\n", (double)(processor_clock_Hz()/1000000.));
    printf("System page size is %d bytes.\n", getpagesize());
}
#endif

#if defined(ID)
const char* cpuid_c = "\0@ID " ID;
#endif
