/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
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

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    memset(wp_pool[i].EXPR, 0, sizeof(wp_pool[i].EXPR));
    wp_pool[i].initial_ans = 0;
  }
  head = NULL;
  free_ = wp_pool;
}
/*   complete a linked list which starts at "head", ends at "free_"    */

WP *new_wp(char *e, int *success){
  if(free_ == NULL){
    *success = 0;
    printf("watchpoint pool is already full \n");
    return NULL;
  }
  word_t ans = expr(e, success);
  WP* new_one = free_;
  if(*success != 1) return NULL;
  else if(head == NULL){
    head = free_;
    free_ = free_->next;
    head->next = free_;
    strcpy(head->EXPR, e);
    head->initial_ans = ans;
  }
  else{
    strcpy(free_->EXPR, e);
    free_->initial_ans = ans;
    free_ = free_->next;
  }
  return new_one;
}

void clear_wp(WP* now){
  //memset(now->EXPR, 0, sizeof(now->EXPR));
  now->initial_ans = 0;
}

WP* free_wp(int N, int *success){
  WP *now = head, *erase = head;
  if(now == NULL){
    *success = 0;
    printf("No watchpoint in the pool \n");
    return NULL;
  }
  if(now->NO == N){
    erase = now;
    if(head->next == free_){
      head = NULL;
      erase->next = free_->next;
      free_->next = erase;
      return erase;
    }
    head = erase->next;
    erase->next = free_->next;
    free_->next = erase;
    clear_wp(erase);
    return erase;
  }
  while(now->next != free_){
    if(now->next->NO == N){
      WP* erase = now->next;
      now->next = erase->next;
      erase->next = free_->next;
      free_->next = erase;
      clear_wp(erase);
      return erase;
    }
    now = now->next;
  }
  *success = 0;
  printf("No such watchpoint numbered %d\n", N);
  return NULL;
}

void print_wp(){
  if(head == NULL){
    printf("No watchpoint in the pool \n");
    return;
  }
  WP* now = head;
  while(now != free_){
    printf("watchpoint %d:  %s    value = %u\n", now->NO, now->EXPR, now->initial_ans);
    now = now->next;
  }
  return;
}

void check_wp(){
  if(head == NULL) return;
  WP *now = head;
  int success = 1;
  while(now != free_){
    word_t val = expr(now->EXPR, &success);
    if(val != now->initial_ans){
      nemu_state.state = NEMU_STOP;
      printf("watchpoint %d is changed:  %s    value_pre = %u, value_now = %u\n", now->NO, now->EXPR, now->initial_ans, val);
      now->initial_ans = val;    
    }
    now = now->next;
  }
  return;
}
/* TODO: Implement the functionality of watchpoint */
 

