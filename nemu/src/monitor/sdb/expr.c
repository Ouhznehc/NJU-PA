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

#include <isa.h>
#include "memory/vaddr.h"
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_PLUS, TK_MINUS, TK_TIMES, TK_DIVIDE, TK_NUM_10,
  TK_LBRACKET, TK_RBRACKET, TK_NUM_16, TK_NEQ, TK_REG, TK_AND, TK_POINTER, TK_NEG,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},                // spaces
  {"\\+", TK_PLUS},                 // plus
  {"==", TK_EQ},                    // equal
  {"-", TK_MINUS},                  // minus
  {"\\*", TK_TIMES},                // times
  {"/", TK_DIVIDE},                 // divide
  {"\\b[0-9]+\\b", TK_NUM_10},      // numbers_10
  {"\\(", TK_LBRACKET},             // left bracket
  {"\\)", TK_RBRACKET},             // right bracket
  {"0x[0-9a-f]+", TK_NUM_16},       // numbers_16
  {"&&", TK_AND},                   // and
  {"!=", TK_NEQ},                   // not_equal
  {"\\$[$0-9a-z]{1,3}", TK_REG},              // register
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[1000] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;
  memset(tokens, 0, sizeof(tokens));
  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        //  Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //      i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;
        
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          case TK_NUM_10:
          case TK_NUM_16:
          case TK_REG:
            tokens[nr_token].type = rules[i].token_type;
            if(substr_len < 32) strncpy(tokens[nr_token].str, substr_start, substr_len);
            else panic("buffer overflow!");
            nr_token++;
            break;
          default: 
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


int check_parentheses(int p, int q) {
  int is_BNF_valid = 1;
  if(tokens[p].type != TK_LBRACKET || tokens[q].type != TK_RBRACKET) is_BNF_valid = 0;
  int counter = 0;
  for(int i = p; i <= q; i++) {
    if(tokens[i].type == TK_LBRACKET) counter++;
    if(tokens[i].type == TK_RBRACKET) counter--;
    if(counter == 0 && i < q) is_BNF_valid = 0;
    if(counter < 0) return -1;
  }
  if(counter != 0) return -1;
  return is_BNF_valid;
}

int priority(int type){
  if(type == TK_TIMES || type == TK_DIVIDE) return 2;
  else if(type == TK_PLUS || type == TK_MINUS) return 3;
  else if(type == TK_POINTER || type == TK_NEG) return 1;
  else if(type == TK_EQ || type == TK_NEQ) return 4;
  else if(type == TK_AND) return 5;
  return 0;
}

int main_opt_pos(int p, int q) {
  int pos = p, Priority = -1, counter = 0, single = -1, flag = 1;
  for(int i = p; i <= q; i++){
    if(tokens[i].type == TK_LBRACKET){
      counter++;
      while(counter){
        i++;
        if(tokens[i].type == TK_LBRACKET) counter++;
        if(tokens[i].type == TK_RBRACKET) counter--;
      }
    }
    else if(tokens[i].type == TK_NUM_10 || tokens[i].type == TK_NUM_16 || tokens[i].type == TK_REG) continue;
    else{
      if(priority(tokens[i].type) == 1 && flag) single = i, flag = 0;
      if(priority(tokens[i].type) >= Priority){
        Priority = priority(tokens[i].type);
        pos = i;
      }
    }
  }
  if(Priority == -1) return -1;
  if(Priority == 1) return single;
  return pos;
}

word_t eval(int p, int q, int *success) {
  if(p > q){*success = 0; return 0;}
  if(p == q) {
    word_t ans;
  //  Log("%s", tokens[p].str);
    if(tokens[p].type == TK_NUM_10) ans = atoi(tokens[p].str);
    else if(tokens[p].type == TK_NUM_16) sscanf(tokens[p].str, "%x", &ans);
    else if(tokens[p].type == TK_REG) ans = isa_reg_str2val(tokens[p].str + 1, success);
    else {*success = 0; return 0;}
    if(*success == 1) return ans;
    else return 0;
  }
  if(check_parentheses(p, q)) return eval(p + 1, q - 1, success);
  else{
    int main_pos = main_opt_pos(p, q);
    if(main_pos == -1){*success = 0; return 0;}
    word_t val1, val2;
    if(priority(tokens[main_pos].type) != 1){
      val1 = eval(p, main_pos - 1, success);
      val2 = eval(main_pos + 1, q, success);
    }
    else{
      val1 = eval(main_pos + 1, q, success);
      val2 = 0;
    }
    switch (tokens[main_pos].type) {
      case TK_AND:     return val1 && val2;
      case TK_EQ:      return val1 == val2;
      case TK_NEQ:     return val1 != val2;
      case TK_PLUS:    return val1 + val2;
      case TK_MINUS:   return val1 - val2;
      case TK_TIMES:   return val1 * val2;
      case TK_NEG:     return -val1;
      case TK_POINTER: return vaddr_read(val1, 4);
      case TK_DIVIDE: 
        if(val2 == 0){*success = -1; return 0;}
        return val1 / val2;
      default: assert(0);
    }
  }
}

word_t expr(char *e, int *success) {
  if (!make_token(e)){
    *success = 0;
    printf("Invalid expression \n");
    return 0;
  }
  if(check_parentheses(0, nr_token - 1) == -1){
    *success = 0;
    printf("Invalid expression \n");
    return 0;
  }
  for (int i = 0; i < nr_token; i ++)
    if (i == 0 || (tokens[i - 1].type != TK_RBRACKET && tokens[i - 1].type != TK_NUM_10 && tokens[i - 1].type != TK_NUM_16 && tokens[i - 1].type != TK_REG)){
      if(tokens[i].type == TK_MINUS) tokens[i].type = TK_NEG;
      if(tokens[i].type == TK_TIMES) tokens[i].type = TK_POINTER;
    }
  /* TODO: Insert codes to evaluate the expression. */
  word_t ans = eval(0, nr_token - 1, success);
  switch(*success){
    case 0 :    printf("Invalid expression \n"); break;
    case -1:    printf("Runtime error \n"); break;
    case -2:    printf("Invalid register name \n"); break;
    default: break;
  }
  return ans;
}
