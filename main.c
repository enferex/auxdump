#include <asm/hwcap2.h>
#include <assert.h>
#include <elf.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __x86_64__
typedef Elf64_auxv_t AuxType;
#define FMT_HEX "0x%016lx"
#define FMT_DEC "%lu"
#elif __i386__
typedef Elf32_auxv_t AuxType;
#define FMT_HEX "0x%08x"
#define FMT_DEC "%u"
#else
#error "This tool is only supported on 32 and 64 bit X86 targets."
#endif

// The encoded CPUID values (represented in AT_HWCAP) are from the Intel manual
// Figure 3-8:
// https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-instruction-set-reference-manual-325383.pdf
static const char *CPUID_1_EDX[] = {
    "FPU",  "VME",  "DE",       "PSE",   "TSC",      "MSR", "PAE",      "MCE",
    "CX8",  "APIC", "Reserved", "SEP",   "MTRR",     "PGE", "MCA",      "CMOV",
    "PAT",  "PSE",  "PSN",      "CLFSH", "Reserved", "DS",  "ACPI",     "MMX",
    "FXSR", "SSE",  "SSE2",     "SS",    "HTT",      "TM",  "Reserved", "PBE"};

void dump_x86_capabilities(uint32_t hwcap) {
  // The linux kernel populates AT_HWCAP for x86 processors using the data
  // from the cpuid instruction.  Specifically the values encoded in the EDX
  // register. See:
  // https://github.com/torvalds/linux/blob/f40ddce8/arch/x86/include/asm/elf.h#L260
  for (int i = 0; i < 32; ++i)
    if ((hwcap >> i) & 1) printf("%s ", CPUID_1_EDX[i]);
}

static void dump_aux(const AuxType *auxp) {
  assert(auxp && "Invalid input.");
  // The aux vector is read in dl_main (glibc:rtld.c).  The code below is
  // largely from that. See:
  // https://sourceware.org/git/?p=glibc.git;a=blob;f=elf/rtld.c;h=34879016ad595ee94206988ce52839e1f2865473;#l1416
  for (const AuxType *aux = auxp; aux->a_type != AT_NULL; ++aux) {
    // Since the kernel is creating the AUX vector, looking in
    // linux/fs/binfmt_elf.c is the best place to understand what data is going
    // where. See:
    // https://github.com/torvalds/linux/blob/f40ddce88593482919761f74910f42f4b84c004b/fs/binfmt_elf.c#L256
#define CASE(_x)                                          \
  case _x:                                                \
    printf("%-16s: " FMT_HEX "\n", #_x, aux->a_un.a_val); \
    break
    switch (aux->a_type) {
      CASE(AT_IGNORE);
      CASE(AT_EXECFD);
      CASE(AT_PHDR);
      CASE(AT_PHENT);
      CASE(AT_PHNUM);
      CASE(AT_PAGESZ);
      CASE(AT_BASE);
      CASE(AT_FLAGS);
      CASE(AT_ENTRY);
      CASE(AT_NOTELF);
      CASE(AT_UID);
      CASE(AT_EUID);
      CASE(AT_GID);
      CASE(AT_EGID);
      CASE(AT_CLKTCK);
      CASE(AT_FPUCW);
      CASE(AT_DCACHEBSIZE);
      CASE(AT_ICACHEBSIZE);
      CASE(AT_UCACHEBSIZE);
      CASE(AT_IGNOREPPC);
      CASE(AT_SECURE);
      CASE(AT_BASE_PLATFORM);
      CASE(AT_SYSINFO);
      CASE(AT_SYSINFO_EHDR);
      CASE(AT_L1I_CACHESHAPE);
      CASE(AT_L1D_CACHESHAPE);
      CASE(AT_L2_CACHESHAPE);
      CASE(AT_L3_CACHESHAPE);
      CASE(AT_L1I_CACHESIZE);
      CASE(AT_L1I_CACHEGEOMETRY);
      CASE(AT_L1D_CACHESIZE);
      CASE(AT_L1D_CACHEGEOMETRY);
      CASE(AT_L2_CACHESIZE);
      CASE(AT_L2_CACHEGEOMETRY);
      CASE(AT_L3_CACHESIZE);
      CASE(AT_L3_CACHEGEOMETRY);
      CASE(AT_MINSIGSTKSZ);
      case AT_EXECFN: {
        printf("%-16s: (" FMT_HEX ") -> %s\n", "AT_EXECFN", aux->a_un.a_val,
               (aux->a_un.a_val) ? (char *)aux->a_un.a_val : "N/A");
        break;
      }
      case AT_HWCAP: {
        printf("%-16s: (0x%-8x) -> [", "AT_HWCAP", (uint32_t)aux->a_un.a_val);
        dump_x86_capabilities((uint32_t)aux->a_un.a_val);
        printf("]\n");
        break;
      }
      case AT_HWCAP2: {
        // See:
        // https://github.com/torvalds/linux/blob/2a1d7946fa53cea2083e5981ff55a8176ab2be6b/arch/x86/include/uapi/asm/hwcap2.h
        const uint32_t cap = aux->a_un.a_val;
        printf("%-16s: (0x%-8x) -> [", "AT_HWCAP2", cap);
#define BIT(_x) ((_x)&1)
        if (cap & HWCAP2_RING3MWAIT) printf("RING3MWAIT ");
        if (cap & HWCAP2_FSGSBASE) printf("FSGSBASE ");
        printf("]\n");
        break;
      }
      case AT_PLATFORM: {
        printf("%-16s: (" FMT_HEX ") -> %s\n", "AT_PLATFORM", aux->a_un.a_val,
               (aux->a_un.a_val) ? (char *)aux->a_un.a_val : "N/A");
        break;
      }
      case AT_RANDOM: {
        const uint8_t *c = (uint8_t *)aux->a_un.a_val;
        printf("%-16s: (%p) -> [", "AT_RANDOM", (void *)aux->a_un.a_val);
        for (int i = 0; i < 16; ++i) printf("0x%x ", c[i]);
        printf("\b]\n");
        break;
      }
      default:
        printf("Unknown AT_ type " FMT_DEC " (see elf.h)\n", aux->a_type);
    }
  }
}

int main(int argc, char **argv, char **envp) {
  // The initialization for the aux vector occurs in dl-sysdep.c
  // (_dl_sysdep_start). The auxv pointer is located after the NULL terminator
  // to the environment pointer (envp). The following loop is based on glibc.
  // _dl_sysdep_start will call dl_main and pass it the auxv pointer.
  // See:
  // https://sourceware.org/git/?p=glibc.git;a=blob;f=elf/dl-sysdep.c;h=bd5066fe3b7dbf1f215bf22fc06afe0697123922;#l73
  //
  // Similarly, LIBC_START_MAIN in libc-start.c also finds the auxv pointer in
  // the same spot.
  // See:
  // https://sourceware.org/git/?p=glibc.git;a=blob;f=csu/libc-start.c;h=8688cba76d1511207f79e97e5b7eeaea683b819c;#l261
  char **env_item;
  for (env_item = envp; *env_item; ++env_item) continue;
  ++env_item;
  dump_aux((AuxType *)env_item);
  return 0;
}
