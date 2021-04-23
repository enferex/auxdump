#include <assert.h>
#include <elf.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __x86_64__
typedef Elf64_auxv_t AuxType;
#elif __i386__
typedef Elf32_auxv_t AuxType;
#else
#error "This is tool is only supported on 32 and 64 bit X86 targets."
#endif

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
#define CASE(_x)                                       \
  case _x:                                             \
    printf("%-16s: 0x%016lx\n", #_x, aux->a_un.a_val); \
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
      CASE(AT_PLATFORM);
      CASE(AT_HWCAP);
      CASE(AT_FPUCW);
      CASE(AT_DCACHEBSIZE);
      CASE(AT_ICACHEBSIZE);
      CASE(AT_UCACHEBSIZE);
      CASE(AT_IGNOREPPC);
      CASE(AT_SECURE);
      CASE(AT_BASE_PLATFORM);
      CASE(AT_HWCAP2);
      CASE(AT_EXECFN);
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
      case AT_RANDOM: {
        const uint8_t *c = (uint8_t *)aux->a_un.a_val;
        printf("%-16s: (%p) -> [", "AT_RANDOM", (void *)aux->a_un.a_val);
        for (int i = 0; i < 16; ++i) printf("0x%x ", c[i]);
        printf("\b]\n");
        break;
      }
      default:
        printf("Unknown AT_ type %lu (see elf.h)\n", aux->a_type);
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
