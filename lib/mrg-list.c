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
void mrg_list_free (MrgList **list);

void mrg_list_prepend_full (MrgList **list, void *data,
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

int mrg_list_length (MrgList *list)
{
  int length = 0;
  MrgList *l;
  for (l = list; l; l = l->next, length++);
  return length;
}

void mrg_list_prepend (MrgList **list, void *data)
{
  MrgList *new_=calloc (sizeof (MrgList), 1);
  new_->next= *list;
  new_->data=data;
  *list = new_;
}

void
mrg_list_reverse (MrgList **list)
{
  MrgList *new_ = NULL;
  MrgList *l;
  for (l = *list; l; l=l->next)
    mrg_list_prepend (&new_, l->data);
  mrg_list_free (list);
  *list = new_;
}

void mrg_list_append_full (MrgList **list, void *data,
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

void mrg_list_append (MrgList **list, void *data)
{
  mrg_list_append_full (list, data, NULL, NULL);
}

void mrg_list_remove (MrgList **list, void *data)
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

void mrg_list_free (MrgList **list)
{
  while (*list)
    mrg_list_remove (list, (*list)->data);
}

MrgList *mrg_list_nth (MrgList *list, int no)
{
  while(no-- && list)
    list = list->next;
  return list;
}

MrgList *mrg_list_find (MrgList *list, void *data)
{
  for (;list;list=list->next)
    if (list->data == data)
      break;
  return list;
}

static MrgList*
mrg_list_merge_sorted (MrgList* list1,
                       MrgList* list2,
    int(*compare)(const void *a, const void *b, void *userdata), void *userdata
)
{
  if (list1 == NULL)
     return(list2);
  else if (list2==NULL)
     return(list1);

  if (compare (list1->data, list2->data, userdata) >= 0)
  {
    list1->next = mrg_list_merge_sorted (list1->next,list2, compare, userdata);
    /*list1->next->prev = list1;
      list1->prev = NULL;*/
    return list1;
  }
  else
  {
    list2->next = mrg_list_merge_sorted (list1,list2->next, compare, userdata);
    /*list2->next->prev = list2;
      list2->prev = NULL;*/
    return list2;
  }
}

static void
mrg_list_split_half (MrgList*  head,
                     MrgList** list1,
                     MrgList** list2)
{
  MrgList* fast;
  MrgList* slow;
  if (head==NULL || head->next==NULL)
  {
    *list1 = head;
    *list2 = NULL;
  }
  else
  {
    slow = head;
    fast = head->next;

    while (fast != NULL)
    {
      fast = fast->next;
      if (fast != NULL)
      {
        slow = slow->next;
        fast = fast->next;
      }
    }

    *list1 = head;
    *list2 = slow->next;
    slow->next = NULL;
  }
}

void mrg_list_sort (MrgList **head,
    int(*compare)(const void *a, const void *b, void *userdata),
    void *userdata)
{
  MrgList* list1;
  MrgList* list2;

  /* Base case -- length 0 or 1 */
  if ((*head == NULL) || ((*head)->next == NULL))
  {
    return;
  }

  mrg_list_split_half (*head, &list1, &list2);
  mrg_list_sort (&list1, compare, userdata);
  mrg_list_sort (&list2, compare, userdata);
  *head = mrg_list_merge_sorted (list1, list2, compare, userdata);
}

void
mrg_list_insert_sorted (MrgList **list, void *data,
                       int(*compare)(const void *a, const void *b, void *userdata),
                       void *userdata)
{
  mrg_list_prepend (list, data);
  mrg_list_sort (list, compare, userdata);
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

#endif
