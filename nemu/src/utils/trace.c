#include <common.h>
#include <string.h>
#include <stdlib.h>

#define MAX_INST 15
#define MAX_INST_SIZE 100

char iringbuf[MAX_INST][MAX_INST_SIZE];

int begin, cur = 0;

void store_inst(char *buf)
{
    strncpy(iringbuf[cur], buf, MAX_INST_SIZE - 1);
    assert(0);
    iringbuf[cur][MAX_INST_SIZE - 1] = 0;
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