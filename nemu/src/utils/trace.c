#include <common.h>
#include <elf.h>

#define MAX_INST 15
#define MAX_INST_SIZE 100

// iringbuf
char iringbuf[MAX_INST][MAX_INST_SIZE];
int begin, cur = 0;
void store_inst(char *buf)
{
    strncpy(iringbuf[cur], buf, MAX_INST_SIZE - 1);
    // assert(0);
    iringbuf[cur][MAX_INST_SIZE - 1] = 0;
    cur = (cur + 1) % MAX_INST;
    if (cur == begin)
        begin = (begin + 1) % MAX_INST;
}

void display_inst()
{
    int index = begin;
    while (index != cur)
    {
        printf(((index + 1) % MAX_INST == cur) ? " --> %s\n" : "     %s\n", iringbuf[index]);
        index = (index + 1) % MAX_INST;
    }
}

// mtrace
void mtrace_read(paddr_t addr, int len)
{
    printf("r: addr = " FMT_PADDR ","
           " len = %d\n",
           addr, len);
}

void mtrace_write(paddr_t addr, int len, word_t data)
{
    printf("w: addr = " FMT_PADDR ","
           " len = %d, data = " FMT_WORD "\n",
           addr, len, data);
}

// ftrace
static FILE* elf_fp = NULL;
static Elf32_Ehdr ehdr;
static Elf32_Shdr *shdr;
static Elf32_Sym *symtab;
static char *strtab;
static int calls = 0;
typedef struct func_map
{
    char name[256];
    paddr_t start;
    word_t size;
    struct func_map* next; 
} FM;
static FM* head = NULL;

void gen_fm(word_t count);
void init_elf(const char *elf_file)
{
    if (elf_file != NULL)
    {
        FILE *fp = fopen(elf_file, "rb");
        Assert(fp, "Can not open '%s'", elf_file);
        elf_fp = fp;
    } else return;
    Log("Elf file at %s", elf_file);
    // read elf header
    Assert(fread(&ehdr, sizeof(Elf32_Ehdr), 1, elf_fp) == 1, "Cannot read elf header");

    // read all section headers to shdr
    shdr = malloc(ehdr.e_shnum * sizeof(Elf32_Shdr));
    // set fp to section headers
    fseek(elf_fp, ehdr.e_shoff, SEEK_SET);
    Assert(fread(shdr, sizeof(Elf32_Shdr), ehdr.e_shnum, elf_fp) == ehdr.e_shnum, "Cannot read seciton header");

    // get shstrtab_hdr whose section(section header string table) stores the name of other sections
    Elf32_Shdr shstrtab_hdr = shdr[ehdr.e_shstrndx];
    char *shstrtab = malloc(shstrtab_hdr.sh_size);
    // set fp to section header string table
    fseek(elf_fp, shstrtab_hdr.sh_offset, SEEK_SET);
    Assert(fread(shstrtab, shstrtab_hdr.sh_size, 1, elf_fp) == 1, "Cannot read seciton header string table");

    // find .strtab and .symtab
    for (size_t i = 0; i < ehdr.e_shnum; i++)
    {
        char *name = &shstrtab[shdr[i].sh_name];
        if (strcmp(name, ".strtab") == 0)
        {
            Elf32_Shdr strtab_hdr = shdr[i];
            strtab = malloc(strtab_hdr.sh_size);
            fseek(elf_fp, strtab_hdr.sh_offset, SEEK_SET);
            Assert(fread(strtab, strtab_hdr.sh_size, 1, elf_fp) == 1, "Cannot read string table");
        }
        if (strcmp(name, ".symtab") == 0)
        {
            Elf32_Shdr symtab_hdr = shdr[i];
            symtab = malloc(symtab_hdr.sh_size);
            fseek(elf_fp, symtab_hdr.sh_offset, SEEK_SET);
            Assert(fread(symtab, symtab_hdr.sh_size, 1, elf_fp) == 1, "Cannot read symbol table");
            gen_fm(symtab_hdr.sh_size / sizeof(Elf32_Sym));
        }
    }
    Assert(symtab, "Failed to find symbol table");
    Assert(strtab, "Failed to find string table");
    free(shdr);
    free(shstrtab);
    fclose(elf_fp);
}

void gen_fm(word_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        Elf32_Sym *sym = &symtab[i];
        if (ELF32_ST_TYPE(sym->st_info) == STT_FUNC)
        {
            FM* fm = malloc(sizeof(FM));
            char *name = &strtab[sym->st_name];
    assert(0);

            strcpy(fm->name, name);
            fm->start = sym->st_value;
            fm->size = sym->st_size;
            fm->next = head;
            head = fm;
        }
    }
}

char *match_func(paddr_t addr)
{
    FM* fm = head;
    while (!fm)
    {
        if(addr >= fm->start && addr < fm->start + fm->size)  return fm->name;
        fm = fm->next;
    }
    return "???";
}

void func_call(paddr_t src, paddr_t dest)
{
    assert(0);
    char *name = match_func(dest);
    printf(FMT_PADDR ": ", src);
    for (int i = 0; i < calls; i++) printf("  ");
    printf("call [%s@" FMT_PADDR "]\n", name, dest);
    calls++;
}

void func_ret(paddr_t src)
{
    assert(0);
    calls--;
    char *name = match_func(src);
    printf(FMT_PADDR ": ", src);
    for (int i = 0; i < calls; i++) printf("  ");
    printf("ret [%s]\n", name);
}