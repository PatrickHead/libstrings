/*
 *  Copyright 2023 Patrick Head
 */

/*
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

#include "libstrings.h"

void print_node(avl_node *n);

char *keys[] = {
  "hello",
  "world",
  "this is fun",
  "another string",
  "123456",
  "my",
  "name",
  "is",
  "Kid",
  "Rock",
  NULL
};

int main(int argc, char **argv)
{
  string *str = NULL;
  strings *strs = NULL;
  char **s;
  string_result sr;
  unsigned int id = 0;

  printf("Test:  strings\n");

  str = string_new_with_values("hello, world", 1);
  if (!str)
  {
    fprintf(stderr, "string_new_with_value() failed\n");
    return 1;
  }

  printf("str->ref_cnt=%d, str->text='%s'\n", str->ref_cnt, str->text);

  string_free(str);

  printf("string_free(): completed\n");

  strs = strings_new();

  printf("strs=%p\n", strs);

  if (strs)
  {
    s = keys;
    while (*s)
    {
      str = string_new_with_values(*s, id);
      sr = strings_add(strs, str);
      printf("strings_add(strs, \"%s\", %d)=%s\n", *s, id, strings_result_to_str(sr));
      ++id;
      ++s;
    }

    printf("number of nodes in text tree = %d\n", strs->text_root->n_nodes);fflush(stdout);
    printf("number of nodes in id tree = %d\n", strs->id_root->n_nodes);fflush(stdout);

    if (argc > 1)
    {
      str = strings_find_by_text(strs, argv[1]);
      if (str)
      {
        printf("strings_find_by_text('%s') returned str=%p, str->id=%d, str->text=%s\n",
               argv[1],
               str,
               str->id,
               str->text);
        str = strings_find_by_id(strs, str->id);
        if (str) printf("strings_find_by_id(%u) returned str=%p, str->id=%d, str->text=%s\n",
                        str->id,
                        str,
                        str->id,
                        str->text);
        else printf("strings_find_by_id():  FAILED\n");
      }
      else printf("strings_find_by_text('%s'):  FAILED\n", argv[1]);
    }

    printf("strings (by string order):\n");
    strings_walk(strs, string_text, print_node);

    printf("strings (by id order):\n");
    strings_walk(strs, string_id, print_node);

    sr = strings_remove(strs, "my");
    printf("strings_remove(strs, \"%s\")=%s\n", "my", strings_result_to_str(sr));
    printf("strings (by string order):\n");
    strings_walk(strs, string_text, print_node);
    printf("strings (by id order):\n");
    strings_walk(strs, string_id, print_node);

    sr = strings_remove(strs, "Kid");
    printf("strings_remove(strs, \"%s\")=%s\n", "Kid", strings_result_to_str(sr));
    printf("strings (by string order):\n");
    strings_walk(strs, string_text, print_node);
    printf("strings (by id order):\n");
    strings_walk(strs, string_id, print_node);

    strings_renumber(strs);
    printf("after strings_renumber()\n");
    printf("strings (by string order):\n");
    strings_walk(strs, string_text, print_node);
    printf("strings (by id order):\n");
    strings_walk(strs, string_id, print_node);

    strings_free(strs);
    printf("strings_free(): completed\n");
  }
  else
  {
    printf("strings_new() failed\n");
    return 1;
  }

  return 0;
}

void print_node(avl_node *n)
{
  string_node *s = (string_node *)n;

  if (!s) goto exit;

  printf("id=%d,ref_cnt=%d,text='%s'\n",
         s->value.id,
         s->value.ref_cnt,
         s->value.text);
 
exit:
  return;
}

