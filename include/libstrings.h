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
 *  @file strings.h
 *
 *  @brief Header file for a library to track de-duplicated strings
 */

#ifndef STRINGS_H
#define STRINGS_H

#include <avl.h>

  /**
   *  @typedef enum string_result
   *
   *  @brief Results from various libstrings functions
   */

typedef enum
{
  string_found,       /**<  string was found in lookup operation      */
  string_not_found,   /**<  string was NOT found in lookup operation  */
  string_failed       /**<  lookup operation failed for other reason  */
} string_result;

  /**
   *  @typedef enum string_key
   *
   *  @brief libstrings key type, either ID or value
   */

typedef enum
{
  string_id,     /**<  specific to avl index of string id    */
  string_text  /**<  specific to avl index of string text  */
} string_key;

  /**
   *  @typedef struct string string
   *
   *  @brief create a type for @a string struct
   */

typedef struct string string;

  /**
   *  @struct string
   *
   *  @brief struct to hold contents of string entry
   */

struct string
{
  unsigned int ref_cnt;  /**<  number of times string has been referenced in application  */
  unsigned int id;       /**<  unique id of string entry in AVL tree                      */
  char *text;            /**<  string                              e                      */
};

  /**
   *  @typedef struct string_node string_node
   *
   *  @brief create a type for @a string_node struct
   */

typedef struct string_node string_node;

  /**
   *  @struct string_node
   *
   *  @brief an avl compatible struct to hold contents of string_node entry
   */

struct string_node
{
  avl_node *left;   /**<  points to left (lesser) node    */
  avl_node *right;  /**<  points to right (greater) node  */
  int height;       /**<  current height of node          */
  string value;
};

  /**
   *  @typedef struct strings strings
   *
   *  @brief create a type for @a strings struct
   */

typedef struct strings strings;

  /**
   *  @struct strings
   *
   *  @brief struct to hold all @a string struct entries in AVL tree
   */

struct strings
{
  unsigned int last_id;  /**<   last string id used so far  */
  avl *id_root;          /**<   root of ID AVL tree         */
  avl *text_root;        /**<   root of text AVL tree       */
};

string *string_new(void);
string *string_new_with_values(char *text, unsigned int id);
string *string_dup(string *str);
void string_copy(string *dst, string *src);
void string_free(string *str);

avl_node *string_node_new(void);
avl_node *string_node_new_with_values(char *text, unsigned int id);
avl_node *string_node_dup(avl_node *sn);
void string_node_free(avl_node *sn);
int string_node_compare_text(avl_node *a, avl_node *b);
int string_node_compare_id(avl_node *a, avl_node *b);
void string_node_copy_data(avl_node *dst, avl_node *src);

strings *strings_new(void);
strings *strings_dup(strings *strs);
void strings_free(strings *strs);

string_result strings_add(strings *strs, string *str);
string_result strings_remove(strings *strs, char *text);
string *strings_find_by_text(strings *strs, char *text);
string *strings_find_by_id(strings *strs, unsigned int id);
void strings_walk(strings *strs, string_key key, avl_action action);
void strings_renumber(strings *strs);

char *strings_result_to_str(string_result sr);
string_result strings_str_to_result(char *s);

#endif //STRINGS_H
