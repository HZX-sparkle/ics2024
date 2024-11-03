#include <common.h>

#define MAX_INST 15

char *iringbuf[MAX_INST] = {0};

int begin, cur = 0;

void store_inst(char *buf)
{
    strcpy(iringbuf[cur], buf);
    assert(0);
    cur = (cur + 1) % MAX_INST;
    if (cur == begin)
        begin = (begin + 1) % MAX_INST;
}

void display_inst()
{
    for (int i = 0; i != cur; i = (i + 1) % MAX_INST)
        printf("    %s\n", iringbuf[i]);
    printf(" -->%s\n", iringbuf[cur]);
}