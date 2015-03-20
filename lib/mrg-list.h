/* mrg - MicroRaptor Gui
 * Copyright (c) 2014 Øyvind Kolås <pippin@hodefoting.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __MRG_LIST__
#define  __MRG_LIST__

#include <stdlib.h>

/* The whole mrg_list implementation is in the header and will be inlined
 * wherever it is used.
 */

typedef struct _MrgList MrgList;
  struct _MrgList {void *data;MrgList *next;
  void (*freefunc)(void *data, void *freefunc_data);
  void *freefunc_data;
}
;

static inline void mrg_list_prepend_full (MrgList **list, void *data,
    void (*freefunc)(void *data, void *freefunc_data),
    void *freefunc_data)
{
  MrgList *new_=calloc (sizeof (MrgList), 1);
  new_->next=*list;
  new_->data=data;
  new_->freefunc=freefunc;
  new_->freefunc_data = freefunc_data;
  *list = new_;
}

static inline int mrg_list_length (MrgList *list)
{
  int length = 0;
  MrgList *l;
  for (l = list; l; l = l->next, length++);
  return length;
}

static inline void mrg_list_prepend (MrgList **list, void *data)
{
  MrgList *new_=calloc (sizeof (MrgList), 1);
  new_->next= *list;
  new_->data=data;
  *list = new_;
}

static inline void *mrg_list_last (MrgList *list)
{
  if (list)
    {
      MrgList *last;
      for (last = list; last->next; last=last->next);
      return last->data;
    }
  return NULL;
}

static inline void mrg_list_append_full (MrgList **list, void *data,
    void (*freefunc)(void *data, void *freefunc_data),
    void *freefunc_data)
{
  MrgList *new_= calloc (sizeof (MrgList), 1);
  new_->data=data;
  new_->freefunc = freefunc;
  new_->freefunc_data = freefunc_data;
  if (*list)
    {
      MrgList *last;
      for (last = *list; last->next; last=last->next);
      last->next = new_;
      return;
    }
  *list = new_;
  return;
}

static inline void mrg_list_append (MrgList **list, void *data)
{
  mrg_list_append_full (list, data, NULL, NULL);
}

static inline void mrg_list_remove (MrgList **list, void *data)
{
  MrgList *iter, *prev = NULL;
  if ((*list)->data == data)
    {
      if ((*list)->freefunc)
        (*list)->freefunc ((*list)->data, (*list)->freefunc_data);
      prev = (void*)(*list)->next;
      free (*list);
      *list = prev;
      return;
    }
  for (iter = *list; iter; iter = iter->next)
    if (iter->data == data)
      {
        if (iter->freefunc)
          iter->freefunc (iter->data, iter->freefunc_data);
        prev->next = iter->next;
        free (iter);
        break;
      }
    else
      prev = iter;
}

static inline void mrg_list_free (MrgList **list)
{
  while (*list)
    mrg_list_remove (list, (*list)->data);
}

static inline MrgList *mrg_list_nth (MrgList *list, int no)
{
  while(no-- && list)
    list = list->next;
  return list;
}

static inline MrgList *mrg_list_find (MrgList *list, void *data)
{
  for (;list;list=list->next)
    if (list->data == data)
      break;
  return list;
}

/* a bubble-sort for now, simplest thing that could be coded up
 * right to make the code continue working
 */
static inline void mrg_list_sort (MrgList **list, 
    int(*compare)(const void *a, const void *b, void *userdata),
    void *userdata)
{
  /* replace this with an insertion sort */
  MrgList *temp = *list;
  MrgList *t;
  MrgList *prev;
again:
  prev = NULL;

  for (t = temp; t; t = t->next)
    {
      if (t->next)
        {
          if (compare (t->data, t->next->data, userdata) > 0)
            {
              /* swap */
              if (prev)
                {
                  MrgList *tnn = t->next->next;
                  prev->next = t->next;
                  prev->next->next = t;
                  t->next = tnn;
                }
              else
                {
                  MrgList *tnn = t->next->next;
                  temp = t->next;
                  temp->next = t;
                  t->next = tnn;
                }
              goto again;
            }
        }
      prev = t;
    }
  *list = temp;
}

static inline void
mrg_list_insert_sorted (MrgList **list, void *data,
                       int(*compare)(const void *a, const void *b, void *userdata),
                       void *userdata)
{
  mrg_list_prepend (list, data);
  mrg_list_sort (list, compare, userdata);
}

static inline void
mrg_list_reverse (MrgList **list)
{
  MrgList *new_ = NULL;
  MrgList *l;
  for (l = *list; l; l=l->next)
    mrg_list_prepend (&new_, l->data);
  mrg_list_free (list);
  *list = new_;
}

#endif
