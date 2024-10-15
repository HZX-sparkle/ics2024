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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expr[256];
  word_t val;

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp() {
  Assert(free_ != NULL, "No free watchpoints available");

  WP* wp = free_;
  free_ = free_->next;

  wp->next = head;
  head = wp;

  return wp;
}

void free_wp(WP *wp) {
  if (head == NULL) return;
  
  WP **p = &head;
  while (*p != wp && *p != NULL) p = &(*p)->next;
  if(*p != NULL) {
    *p = wp->next;
    wp->next = free_;
    free_ = wp;
  }
}

void set_wp(char* e, word_t val) {
  WP* wp = new_wp();
  strcpy(wp->expr, e);
  wp->val = val;

  printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
}

void delete_wp(int n) {
  if( n < 0 || n > NR_WP-1 ) {
    printf("N out of bound.\n");
    return;
  }
  WP *wp = &wp_pool[n];
  free_wp(wp);
}

void check_wp() {
  if( head == NULL ) return;
  WP *wp = head;
  while (wp != NULL)
  {
    bool success = true;
    word_t old = wp->val;
    word_t new = expr(wp->expr, &success);
    if( old != new ) {
      printf(
        "\n"
        "Watchpoint %d: %s\n"
        "\n"
        "Old value = 0x%08x\n"
        "New value = 0x%08x\n",
        wp->NO, wp->expr, old, new
      );
      wp->val = new;
      nemu_state.state = NEMU_STOP;
    }
    wp = wp->next;
  }
}

void display_wp() {
  if( head == NULL ) {
    printf("No watchpoints.\n");
    return;
  }
  WP *wp = head;
  printf("%-8s%-15s%s\n", "Num", "Type", "What");
  while (wp != NULL) {
    printf("%-8d%-15s%s\n", wp->NO, "watchpoint", wp->expr);
    wp = wp->next;
  }
}
