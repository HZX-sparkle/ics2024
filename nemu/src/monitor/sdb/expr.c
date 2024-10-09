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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_REG, TK_HEX, TK_DEC

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},          // spaces
  {"\\*", '*'},               // multiply or pointer
  {"\\/", '/'},               // divide
  {"\\+", '+'},               // plus
  {"\\-", '-'},               // minus or negetive
  {"\\(", '('},               // parentheses
  {"\\)", ')'},               // back parentheses
  {"==", TK_EQ},              // equal
  {"\\$[0-9a-z]+", TK_REG},   // register
  {"0x[0-9a-fA-F]+", TK_HEX}, // heximal
  {"[0-9]+", TK_DEC},         // decimal

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

static Token tokens[320] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case '*': tokens[nr_token++].type = '*'; break;
          case '/': tokens[nr_token++].type = '/'; break;
          case '+': tokens[nr_token++].type = '+'; break;
          case '-': tokens[nr_token++].type = '-'; break;
          case '(': 
            strcpy(tokens[nr_token].str, "(");
            tokens[nr_token++].type = '('; 
            break;
          case ')': 
            strcpy(tokens[nr_token].str, ")");
            tokens[nr_token++].type = ')'; 
            break;
          case TK_DEC: {
            if(substr_len > 32) {
              printf("substr too long at position %d\n", position - substr_len);
              assert(0);
              return false;
            }
            tokens[nr_token].type = TK_DEC;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = 0;

            nr_token++;
            break;
          }
          default: break;
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

static bool check_parentheses(int p, int q) {
  // ()() , (()), ((()), ( ))(( ), () **, ((
  int cnt = 0;
  bool flag = false;
  for (int i = p; i <= q; i++) {
    if (cnt == 0 && i != p) flag = true;
    if ( tokens[i].type == '(') cnt++;
    if ( tokens[i].type == ')') cnt--;
    if ( cnt < 0 ) {
      printf("Bad expression: parentheses invalid\n");
      assert(0);
    }
  }
  if ( cnt != 0 ) {
    printf("Bad expression: parentheses invalid\n");
    // for (int i = p; i <= q; i++) {
    //   if( tokens[i].type == '(' || tokens[i].type == ')') printf("%s", tokens[i].str);
    // }
    // printf("\n");
    assert(0);
  }
  if ( flag ) return false;
  if ( tokens[q].type != ')') return false;
  return true;
}

static int eval(int p, int q) {
  // printf("%d %d\n", p, q);
  if (p > q) {
    /* Bad expression or --2 situation */
    if( q < 0 ) return 0;
    printf("Bad expression: p > q\n");
    assert(0);
  }
  else if (p == q) {
    /* Single token.
    * For now this token should be a number.
    * Return the value of the number.
    */
    // printf("%s\n", tokens[p].str);
    int val = atoi(tokens[p].str);
    // printf("%d\n", val);
    return val;
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
    * If that is the case, just throw away the parentheses.
    */
    // if (strcmp(tokens[p].str, "-(") == 0 ) return -eval(p + 1, q - 1);
    return eval(p + 1, q - 1);
  }
  else {
    /* We should do more things here. */
    // 1. Find the main operator.
    int i;
    int op = -1;
    int op_type = -1;
    // int op_prec;
    for ( i = q; i >= p; i--)
    {
      switch (tokens[i].type)
      {
        case ')':
          // skip to the next matching '('.
          int cnt = 1;
          while (cnt != 0)
          {
            i--;
            if (tokens[i].type == ')') cnt++;
            else if (tokens[i].type == '(') cnt--;
          }
          continue;
        
        case '+': case '-':
          op = i;
          op_type = tokens[i].type;
          break;
        
        case '*': case '/':
          if( op == -1 ){
            op = i;
            op_type = tokens[i].type;
          }
          continue;

        case TK_DEC:
          continue;

        default:
          printf("Bad expression: cannot match the token: %s\n", tokens[i].str);
          assert(0);
      }
      
      break;
    }

    printf("main op: %c, position: %d\n", tokens[op].type, op);
    
    // 2. Get the value of two parts splited by op.
    int val1 = eval(p, op - 1 );
    int val2 = eval(op + 1, q);
    switch (op_type)
    {
      case '+':
        return val1+val2;
        break;

      case '-':
        return val1-val2;

      case '*':
        return val1*val2;
      
      case '/':
        if (val2 == 0) {
          printf("division by 0\n");
          assert(0);
        }
        return val1/val2;

      default:
        break;
    }
  }
  assert(0);
  return 0;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // int cnt=1;
  // for (int i = 1; i < nr_token; i++)
  // {
  //   if( tokens[i].type == '-' && tokens[i-1].type != TK_DEC && tokens[i-1].type != ')' ) {
  //     tokens[i].type = TK_NOTYPE;
  //     cnt*=-1;
  //   }
  //   if(cnt==-1&& (tokens[i].type=='(' || tokens[i].type==TK_DEC)) {
  //     char str[32]="-";
  //     strcat(str, tokens[i].str);
  //     strcpy(tokens[i].str, str);
  //     cnt=1;  
  //   }
  // }
  
  // int j = 0;
  // for (int i = 0; i < nr_token; i++)
  // {
  //   if(tokens[i].type != TK_NOTYPE) {
  //     tokens[j] = tokens[i];
  //     j++;
  //   }
  // }
  // nr_token = j;

  // for(int i=0;i<nr_token;i++){
  //   if(tokens[i].type == TK_DEC) printf("%s ", tokens[i].str);
  // }
  // printf("\n");
  
  // printf("e: %s\n", e);
  return eval(0, nr_token-1);

}

