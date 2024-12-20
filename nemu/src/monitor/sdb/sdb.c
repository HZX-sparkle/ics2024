/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_test(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);


static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Step one instruction exactly.", cmd_si },
  { "info", "Display program status", cmd_info },
  { "x", "Examine memory", cmd_x },
  { "test", "Test your functions", cmd_test},
  { "p", "Expression evalution", cmd_p},
  { "w", "Set watchpoint", cmd_w },
  { "d", "Delete watchpoint", cmd_d },
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  /* n defaults to 1 */
  int n = 1;

  if (arg != NULL) n = atoi(arg);
  cpu_exec(n);
  return 0;
}

static int cmd_info(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");

  if ( arg == NULL ) {
    /* no argument given */
    printf("List of info subcommands:\n\n");
    printf("info r -- List of registers and their content.\n" );
    printf("info w -- List of watchpoints.\n" );
  } else {
    switch ( *arg ) {
      case 'r' : isa_reg_display(); break;
      case 'w' : display_wp(); break;
      default : printf("Unknown command '%s'\n", arg);
    }
  }
  return 0;
}

static int cmd_x(char *args) {
  /* extract the first argument */
  char *arg1 = strtok(NULL, " ");
  /* extract the second argument */
  char *arg2 = strtok(NULL, " ");

  if ( arg1 == NULL || arg2 == NULL )
  {
    printf("Arguments required (x N EXPR).\n");
  } else {
    int n = atoi(arg1);
    bool success = true;
    // vaddr_t addr = expr(arg2, &success);
    vaddr_t addr = strtoul(arg2, NULL, 16);
    if ( success ) 
    {
      printf("%-20s %-20s\n", "Address", "Value");
      for (size_t i = 0; i < n; i++)
      {
        word_t ret = vaddr_read(addr , 4);
        printf("%s%-18x %s%-18x\n", "0x", addr, "0x", ret);
        addr += 4;
      }
    }
  }
  return 0;
}

static int cmd_test(char *args) {
  bool success;
  FILE *fp = fopen("/home/kali/ics2024/nemu/tools/gen-expr/input", "r");
  char e[65536] = {};
  word_t result;
  while(fscanf(fp, "%u %s",&result, e) != EOF) {
    word_t my_result = expr(e, &success);
    Assert(my_result==result, "For the expression %s\nThe answer is %u, while my result is %u", e, result, my_result);
  }
  return 0;
}

static int cmd_p(char *args) {
  if (args == NULL) {
    printf("Usage: p EXPR\n");
    return 0;
  }
  bool success = true;
  word_t ret = expr(args, &success);
  if (!success) {
    printf("Invalid expression: %s\n", args);
    return 0;
  }
  printf("%u\n", ret);
  return 0;
}

static int cmd_w(char *args) {
#ifdef CONFIG_WATCHPOINT
  if( args == NULL ) {
    printf("Usage: w EXPR\n");
    return 0;
  }
  bool success = true;
  word_t val = expr(args, &success);
  if(!success) {
    printf("Invalid expression: %s", args);
    return 0;
  }
  set_wp(args, val);
#else
  printf("Watchpoints not enabled\n");
#endif
  return 0;
}

static int cmd_d(char *args) {
#ifdef CONFIG_WATCHPOINT
  char *arg = strtok(NULL, " ");
  if( arg == NULL) {
    printf("Usage: d N\n");
    return 0;
  }
  int n = strtol(arg, NULL, 10);
  delete_wp(n);
#else
  printf("Watchpoints not enabled\n");
#endif
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
