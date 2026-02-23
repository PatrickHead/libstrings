/*
 *  Copyright 2021,2022,2024,2025 Patrick T. Head
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/**
 *  @file strings.c
 *
 *  @brief Source code file for a library to track de-duplicated strings
 */

#include <stdlib.h>
#include <string.h>
#include <search.h>

#include "libstrings.h"

static void renumber_action(avl_node *n);
static void duper_action(avl_node *n);

  /**
   *  @fn string *string_new(void)
   *
   *  @brief create a new @a string struct
   *
   *  @par Parameters
   *  None.
   *
   *  @return pointer to new @a string struct
   */

string *string_new(void)
{
  string *str = NULL;

  str = malloc(sizeof(string));
  if (str) memset(str, 0, sizeof(string));

  return str;
}

  /**
   *  @fn string *string_new_with_values(char *text, unsigned int id)
   *
   *  @brief create a new @a string struct populated with values
   *
   *  @param text - string for text value
   *  @param id - id value
   *
   *  @return pointer to new @a string struct
   */

string *string_new_with_values(char *text, unsigned int id)
{
  string *str = NULL;

  if (!text) goto exit;

  if (!(str = string_new())) goto exit;

  if (text) str->text = strdup(text);
  str->id = id;
  str->ref_cnt = 0;

exit:
  return str;
}

  /**
   *  @fn string *string_dup(string *str)
   *
   *  @brief creates a deep copy of @p str
   *
   *  @param str - pointer to existing @a string struct
   *
   *  @return pointer to new @a string struct
   */

string *string_dup(string *str)
{
  string *nstr = NULL;

  if (!str || !str->text) goto exit;

  nstr = string_new();
  if (!nstr) goto exit;

  memcpy(nstr, str, sizeof(string));

  if (str->text) nstr->text = strdup(str->text);

exit:
  return nstr;
}

  /**
   *  @fn void string_copy(string *dst, string *src)
   *
   *  @brief copies data from @p src to @p dst
   *
   *  @param dst - pointer to existing @a string struct
   *  @param src - pointer to existing @a string struct
   *
   *  @par Returns
   *  Nothing.
   */

void string_copy(string *dst, string *src)
{
  if (!dst || !src) return;

  memcpy(dst, src, sizeof(string));

  if (src->text) dst->text = strdup(src->text);
}

  /**
   *  @fn void string_free(string *str)
   *
   *  @brief frees all memory associated with @p str
   *
   *  @param str - pointer to existing @a string struct
   *
   *  @par Returns
   *  Nothing.
   */

void string_free(string *str)
{
  if (!str) return;

  if (str->text) free(str->text);

  free(str);
}

  /**
   *  @fn strings *strings_new(void)
   *
   *  @brief create a new @a strings struct
   *
   *  @par Parameters
   *  None.
   *
   *  @return pointer to new @a strings struct
   */

strings *strings_new(void)
{
  strings *strs = NULL;

  if (!(strs = malloc(sizeof(strings)))) goto exit;

  memset(strs, 0, sizeof(strings));

  strs->id_root = avl_new();
  if (!strs->id_root) goto exit;

  avl_set_free(strs->id_root, string_node_free);
  avl_set_cmp(strs->id_root, string_node_compare_id);
  avl_set_new(strs->id_root, string_node_new);
  avl_set_dup(strs->id_root, string_node_dup);
  avl_set_copy_data(strs->id_root, string_node_copy_data);

  strs->text_root = avl_new();
  if (!strs->text_root) goto exit;

  avl_set_free(strs->text_root, string_node_free);
  avl_set_cmp(strs->text_root, string_node_compare_text);
  avl_set_new(strs->text_root, string_node_new);
  avl_set_dup(strs->text_root, string_node_dup);
  avl_set_copy_data(strs->text_root, string_node_copy_data);

exit:
  return strs;
}

static strings *nstrs = NULL;  /**<  used by duper_action()  */

  /**
   *  @fn strings *strings_dup(strings *strs)
   *
   *  @brief create a deep copy of @p strs
   *
   *  @param strs - pointer to existing @a strings struct
   *
   *  @return pointer to new @a strings struct
   */

strings *strings_dup(strings *strs)
{
  if (!strs) goto exit;

  nstrs = strings_new();
  if (!nstrs) goto exit;

  strings_walk(strs, string_id, duper_action);

exit:
  return nstrs;
}

  /**
   *  @fn void strings_free(strings *strs)
   *
   *  @brief frees all memory allocated to @p strs
   *
   *  @param strs - pointer to existing @a strings struct
   *
   *  @par Returns
   *  Nothing.
   */

void strings_free(strings *strs)
{
  if (!strs) return;

  if (strs->text_root) avl_free(strs->text_root);
  if (strs->id_root) avl_free(strs->id_root);

  free(strs);
}

  /**
   *  @fn string_result strings_add(strings *strs, string *str)
   *
   *  @brief adds @p str to @p strs
   *
   *  @param strs - pointer to existing @a strings struct
   *  @param str  - pointer to existing @a string struct
   *
   *  @return @a string_result indicating success or failure
   */

string_result strings_add(strings *strs, string *str)
{
  string *s = NULL;
  string_node *n = NULL;
  avl_node *found = NULL;
  string_result r = string_failed;

  if (!strs || !str) goto bail;

  n = (string_node *)string_node_new_with_values(str->text, str->id);
  if (!n) goto bail;

  n->value.ref_cnt = n->value.id = 0;

    /*
     * Does string already exist?
     */

  found = avl_find(strs->text_root, (avl_node *)n);
  if (found)
  {
    avl_node_free(strs->text_root, (avl_node *)n);
    s = (string *)&found->value;
    ++s->ref_cnt;
    return string_found;
  }

  n->value.ref_cnt = 1;
  n->value.id = strs->last_id;

  ++strs->last_id;

  if (avl_insert(strs->text_root, (avl_node *)n))
  {
    avl_node_free(strs->text_root, (avl_node *)n);
    goto bail;
  }

  n = (string_node *)string_node_dup((avl_node *)n);

  n->left = n->right = NULL;

  if (avl_insert(strs->id_root, (avl_node *)n))
  {
    avl_node_free(strs->text_root, (avl_node *)n);
    goto bail;
  }

  r = string_found;

bail:
  switch (r)
  {
    case string_found: break;

    case string_not_found:
    case string_failed:
      break;
  }

  return r;
}

  /**
   *  @fn string_result strings_remove(strings *strs, char *text)
   *
   *  @brief removes entry with text key of @p text from @p strs
   *
   *  @param strs - pointer to existing @a strings struct
   *  @param text - text value of @a string to remove
   *
   *  @return @a string_result indicating success or failure
   */

string_result strings_remove(strings *strs, char *text)
{
  string_node sn;
  string *fs;
  avl_node *found = NULL;

  if (!strs || !text) return string_failed;

  memset(&sn, 0, sizeof(string_node));
  sn.value.text = text;

  found = avl_find(strs->text_root, (avl_node *)&sn);
  if (!found) return string_failed;

  fs = (string *)&found->value;
  sn.value.id = fs->id;

  avl_delete(strs->text_root, (avl_node *)&sn);

  avl_delete(strs->id_root, (avl_node *)&sn);

  return string_found;
}

  /**
   *  @fn string *strings_find_by_text(strings *strs, char *text)
   *
   *  @brief searches @p strs for entry with text value of @p text
   *
   *  @param strs - pointer to existing @a strings struct
   *  @param text - text value of @a string to remove
   *
   *  @return pointer to @a string struct if found, NULL if not
   */

string *strings_find_by_text(strings *strs, char *text)
{
  string_node n;
  avl_node *found;

  if (!strs || !text) return NULL;
  if (!strs->text_root) return NULL;

  memset(&n, 0, sizeof(string_node));
  n.value.text = text;

  found = avl_find(strs->text_root, (avl_node *)&n);

  return found ? &((string_node *)found)->value : NULL;
}

  /**
   *  @fn string *strings_find_by_id(strings *strs, unsigned int id)
   *
   *  @brief searches @p strs for entry with id value of @p id
   *
   *  @param strs - pointer to existing @a strings struct
   *  @param id - id value of @a string to remove
   *
   *  @return pointer to @a string struct if found, NULL if not
   */

string *strings_find_by_id(strings *strs, unsigned int id)
{
  string_node n;
  avl_node *found;

  if (!strs) return NULL;
  if (!strs->id_root) return NULL;

  memset(&n, 0, sizeof(string_node));
  n.value.id = id;

  found = avl_find(strs->id_root, (avl_node *)&n);

  return found ? &((string_node *)found)->value : NULL;
}

  /**
   *  @fn void strings_walk(strings *strs, string_key key, avl_action action)
   *
   *  @brief walks through all entries in @p strs AVL tree
   *
   *  Walks through each entry in @p strs, in either text or id order based on value of @p key.
   *  @p action will be called at each entry.
   *
   *  @param strs - pointer to existing @a strings struct
   *  @param key - @a string_key (enum value of id search order)
   *  @param action - pointer to function to call at each entry found
   *
   *  @par Returns
   *  Nothing.
   */

void strings_walk(strings *strs, string_key key, avl_action action)
{
  avl *tree = NULL;

  if (!strs || !action) return;

  switch (key)
  {
    case string_id: tree = strs->id_root; break;
    case string_text: tree = strs->text_root; break;
  }

  avl_walk(tree, avl_forward_order, action);
}

  /**
   *  @fn char *strings_result_to_str(string_result sr)
   *
   *  @brief returns string representation of @p sr
   *
   *  @param sr - @a string_result to convert to string
   *
   *  @return string representation of @p sr
   */

char *strings_result_to_str(string_result sr)
{
  switch (sr)
  {
    case string_found: return "FOUND";
    case string_not_found: return "NOT FOUND";
    case string_failed:
    default:
      return "FAILED";
  }

  return "FAILED";
}

  /**
   *  @fn string_result strings_str_to_result(char *s)
   *
   *  @brief returns @a string_result from string representation
   *
   *  @param s - string representation of @a string_result
   *
   *  @return @a string_result
   */

string_result strings_str_to_result(char *s)
{
  if (!s) return string_failed;

  if (!strcmp(s, "FOUND")) return string_found;
  if (!strcmp(s, "NOT FOUND")) return string_not_found;

  return string_failed;
}

static unsigned int _new_id = 0;  /**<  used by strings_renumber()  */
static avl *_id_root = NULL;     /**<  used by strings_renumber()  */

  /**
   *  @fn void strings_renumber(strings *strs)
   *
   *  @brief renumbers all entries in @p strs
   *
   *  Walks entire AVL tree of entires in @p str, changing the id of each entry.
   *  Rebuilds id_root (AVL index of ids)
   *
   *  @param strs - pointer to existing @a strings struct
   *
   *  @par Returns
   *  Nothing.
   */

void strings_renumber(strings *strs)
{
  if (!strs) return;

  avl_free(strs->id_root);
  strs->id_root = NULL;

  _new_id = 0;
  _id_root = avl_new();
  avl_set_free(_id_root, string_node_free);
  avl_set_cmp(_id_root, string_node_compare_id);
  avl_set_new(_id_root, string_node_new);
  avl_set_dup(_id_root, string_node_dup);
  avl_set_copy_data(_id_root, string_node_copy_data);

  avl_free(strs->id_root);
  strs->id_root = NULL;

  avl_walk(strs->text_root, avl_forward_order, renumber_action);

  strs->id_root = _id_root;
  strs->last_id = _new_id;
}

  /**
   *  @fn avl_node *string_node_new()
   *
   *  @brief create a new @a string_node struct
   *
   *  @par Parameters
   *  None.
   *
   *  @return pointer to new @a avl_node struct
   */

avl_node *string_node_new(void)
{
  string_node *sn;

  sn = malloc(sizeof(string_node));
  if (sn) memset(sn, 0, sizeof(string_node));

  return (avl_node *)sn;
}

  /**
   *  @fn avl_node *string_node_new_with_values(char *text, unsigned int id)
   *
   *  @brief create a new @a string_node struct
   *
   *  @param text - string containing text value of node
   *  @param id   - unique indentifier of node
   *
   *  @return pointer to new @a avl_node struct
   */

avl_node *string_node_new_with_values(char *text, unsigned int id)
{
  string_node *sn;

  sn = (string_node *)string_node_new();
  if (!sn) return NULL;

  if (text) sn->value.text = strdup(text);
  sn->value.id = id;

  return (avl_node *)sn;
}

  /**
   *  @fn void string_node_dup(avl_node *n)
   *
   *  @brief creates a deep copy of @p n
   *
   *  @param n - pointer to existing @a avl_node that is a @a string_node
   *
   *  @return pointer to new @a avl_node struct
   */

avl_node *string_node_dup(avl_node *n)
{
  avl_node *nsn;
  string_node *sn;

  if (!n) return NULL;

  sn = (string_node *)n;

  nsn = string_node_new_with_values(sn->value.text, sn->value.id);

  return nsn;
}

  /**
   *  @fn void string_node_copy_data(avl_node *dst, avl_node *src)
   *
   *  @brief copies payload data contained in @p src to @p dst
   *
   *  @param dst - pointer to existing @a avl_node that is a @a string_node
   *  @param src - pointer to existing @a avl_node that is a @a string_node
   *
   *  @par Returns
   *       Nothing.
   */

void string_node_copy_data(avl_node *dst, avl_node *src)
{
  string *sd, *ss;

  if (!dst || !src) return;

  sd = &((string_node *)dst)->value;
  ss = &((string_node *)src)->value;

  string_copy(sd, ss);
}

  /**
   *  @fn void string_node_free(avl_node *n)
   *
   *  @brief frees all memory allocated to @p n that is a @a string_node
   *
   *  @param n - pointer to existing @a avl_node that is a @a string_node
   *
   *  @par Returns
   *  Nothing.
   */

void string_node_free(avl_node *n)
{
  string_node *sn;

  if (!n) return;

  sn = (string_node *)n;

  --sn->value.ref_cnt;
  if (!sn->value.ref_cnt)
  {
    if (sn->value.text) free(sn->value.text);
    free(n);
  }
}

  /**
   *  @fn int string_node_compare_text(avl_node *a, avl_node *b)
   *
   *  @brief compares text of @a string payload of @p a with
   *  text of @a string payload of @p b
   *
   *  @param a - pointer to existing @a avl_node struct
   *  @param b - pointer to existing @a avl_node struct
   *
   *  @return <0 if @p a is lexically less than @p b
   *  @return >0 if @p a is lexically greater than @p b
   *  @return 0 if @p a is lexically equal to @p b
   */

int string_node_compare_text(avl_node *a, avl_node *b)
{
  string_node *sna, *snb;
  char *ta, *tb;
  int cmp = 0;

  if (!a || !b) return 0;

  sna = (string_node *)a;
  snb = (string_node *)b;

  ta = sna->value.text;
  tb = snb->value.text;

  if (!ta || !tb) return 0;

  cmp = strcmp(ta, tb);

  if (cmp < 0) cmp = -1;
  else if (cmp > 0) cmp = 1;
  else cmp = 0;

  return cmp;
}

  /**
   *  @fn int string_node_compare_id(avl_node *a, avl_node *b)
   *
   *  @brief compares id of @a string payload of @p a with
   *  id of @a string payload of @p b
   *
   *  @param a - pointer to existing @a avl_node struct
   *  @param b - pointer to existing @a avl_node struct
   *
   *  @return <0 if id of @p a is less than id of @p b
   *  @return >0 if id of @p a is greater than id of @p b
   *  @return 0 if id of @p a is equal to id of @p b
   */

int string_node_compare_id(avl_node *a, avl_node *b)
{
  string_node *sna, *snb;

  if (!a || !b) return 0;

  sna = (string_node *)a;
  snb = (string_node *)b;

  if (sna->value.id < snb->value.id) return -1;
  if (sna->value.id > snb->value.id) return 1;
  return 0;
}

  /**
   *  @fn void renumber_action(avl_node *n)
   *
   *  @brief callback function for avl_walk(), used by strings_renumber()
   *
   *  @param n - pointer to existing @a avl_node struct
   *
   *  @return 0
   */

static void renumber_action(avl_node *n)
{
  string_node *sn;
  string *str;

  if (!n) goto exit;

  sn = (string_node *)n;
  str = &sn->value;

  str->id = _new_id;

  sn = (string_node *)string_node_dup((avl_node *)sn);
  if (!sn) goto exit;

  sn->left = sn->right = NULL;
  sn->height = 0;

  if (avl_insert(_id_root, (avl_node *)sn)) goto exit;

  ++_new_id;

exit:
}

  /**
   *  @fn void duper_action(avl_node *n)
   *
   *  @brief callback function for avl_walk(), used by strings_dup()
   *
   *  @param n - pointer to existing @a avl_node struct
   *
   *  @return 0
   */

static void duper_action(avl_node *n)
{
  string *str;
  string *nstr;

  if (!n) goto exit;

  str = (string *)n->value;
  if (!str) goto exit;

  nstr = string_dup(str);
  if (!nstr) goto exit;

  strings_add(nstrs, nstr);

exit:
}

