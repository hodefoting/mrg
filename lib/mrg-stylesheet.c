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

#include "mrg-internal.h"

MrgStyle *mrg_style (Mrg *mrg)
{
  return &mrg->state->style;
}

const char * html_css =
"html, address,\n"
"blockquote,\n"
"body, dd, div,\n"
"dl, dt, fieldset, form,\n"
"frame, frameset,\n"
"h1, h2, h3, h4,\n"
"h5, h6, noframes,\n"
"ol, p, ul, center,\n"
"dir, hr, menu, pre   { display: block; unicode-bidi: embed }\n"
"li              { display: list-item }\n"
"head            { display: none }\n"
"table           { display: table }\n"
"tr              { display: table-row }\n"
"thead           { display: table-header-group }\n"
"tbody           { display: table-row-group }\n"
"tfoot           { display: table-footer-group }\n"
"col             { display: table-column }\n"
"img             { display: inline-block }\n"
"colgroup        { display: table-column-group }\n"
"td, th          { display: table-cell }\n"
"caption         { display: table-caption }\n"
"th              { font-weight: bolder; text-align: center }\n"
"caption         { text-align: center }\n"
"body            { margin: 0.5em }\n"
"h1              { font-size: 2em; margin: .67em 0 }\n"
"h2              { font-size: 1.5em; margin: .75em 0 }\n"
"h3              { font-size: 1.17em; margin: .83em 0 }\n"
"h4, p,\n"
"blockquote, ul,\n"
"fieldset, form,\n"
"ol, dl, dir,\n"
"menu            { margin: 1.12em 0 }\n"
"h5              { font-size: .83em; margin: 1.5em 0 }\n"
"h6              { font-size: .75em; margin: 1.67em 0 }\n"
"h1, h2, h3, h4,\n"
"h5, h6, b,\n"
"strong          { font-weight: bolder }\n"
"blockquote      { margin-left: 4em; margin-right: 4em }\n"
"i, cite, em,\n"
"var, address    { font-style: italic }\n"
"pre, tt, code,\n"
"kbd, samp       { font-family: monospace }\n"
"pre             { white-space: pre }\n"
"button, textarea,\n"
"input, select   { display: inline-block }\n"
"big             { font-size: 1.17em }\n"
"small, sub, sup { font-size: .83em }\n"
"sub             { vertical-align: sub }\n"
"sup             { vertical-align: super }\n"
"table           { border-spacing: 2px; }\n"
"thead, tbody,\n"
"tfoot           { vertical-align: middle }\n"
"td, th, tr      { vertical-align: inherit }\n"
"s, strike, del  { text-decoration: line-through }\n"
"hr              { border: 1px inset black }\n"
"ol, ul, dir,\n"
"menu, dd        { padding-left: 2.5em }\n"
"ol              { list-style-type: decimal }\n"
"ol ul, ul ol,\n"
"ul ul, ol ol    { margin-top: 0; margin-bottom: 0 }\n"
"u, ins          { text-decoration: underline }\n"
//"br:before     { content: \"\\A\"; white-space: pre-line }\n"
"center          { text-align: center }\n"
":link, :visited { text-decoration: underline }\n"
":focus          { outline: thin dotted invert }\n"

".cursor { color: white; background: black; } \n"

"br       { display: block; }\n"
"document { color : black; font-weight: normal; background-color: white; }\n"
"body     { background-color: transparent; }\n"
"a        { color: blue; text-decoration: underline; }\n"
"a:hover  { background: black; color: white; text-decoration: underline; }\n"
"style, script { display: hidden; }\n"

"hr { margin-top:16px;font-size: 1px; }\n"  /* hack that works in one way, but shrinks top margin too much */
;

void mrg_stylesheet_clear (Mrg *mrg)
{
  if (mrg->stylesheet)
    mrg_list_free (&mrg->stylesheet);
  mrg_css_default (mrg);
}

void mrg_css_default (Mrg *mrg)
{
  char *error = NULL;

  mrg_stylesheet_add (mrg, html_css, NULL, MRG_STYLE_INTERNAL, &error);
  if (error)
  {
    fprintf (stderr, "Mrg css parsing error: %s\n", error);
  }

  mrg_stylesheet_add (mrg,

" bold { font-weight:bold; } \n"
" dim *, dim { opacity:0.5; } \n"
" underline *, underline{ text-decoration:underline; }\n"
" reverse *,selected *, reverse,selected { text-decoration:reverse;}\n"
" unhandled       { color:cyan; }\n"
" binding:key     { background-color:white; color:black; }\n"
" binding:label   { color:cyan; }\n"
      
      ,NULL, MRG_STYLE_INTERNAL, &error);

  if (error)
  {
    fprintf (stderr, "mrg css parsing error: %s\n", error);
  }
}

#define MRG_MAX_SELECTOR_LENGTH 16

typedef struct StyleEntry {
  char        *selector;
  MrgStyleNode parsed[MRG_MAX_SELECTOR_LENGTH];
  int          sel_len;
  char        *css;
  int          specificity;
} StyleEntry;

static void free_entry (StyleEntry *entry)
{
  free (entry->selector);
  free (entry->css);
  free (entry);
}

static int compute_specificity (const char *selector, int priority)
{
  const char *p;
  int n_id = 0;
  int n_class = 0;
  int n_tag = 0;
  int n_pseudo = 0;
  int in_word = 0;

  for (p = selector; *p; p++)
  {
    switch (*p)
    {
      case ' ':
        in_word = 0;
        break;
      case ':':
        in_word = 1;
        n_pseudo++;
        break;
      case '.':
        in_word = 1;
        n_class++;
        break;
      case '#':
        in_word = 1;
        n_id++;
        break;
      default:
        if (!in_word)
        {
          in_word = 1;
          n_tag++;
        }
    }
  }
  return priority * 100000 + n_pseudo * 10000 + n_id * 1000 + n_class * 100 + n_tag * 10;
}

static void mrg_parse_selector (Mrg *mrg, const char *selector, StyleEntry *entry)
{
  const char *p = selector;
  char section[256];
  int sec_l = 0;

  char type = ' ';

  for (p = selector; ; p++)
  {
    switch (*p)
    {
      case '.': case ':': case '#': case ' ': case 0:
        if (sec_l)
        {
          switch (type)
          {
            case ' ':
              entry->parsed[entry->sel_len].element = mrg_intern_string (section);
              break;
            case '#':
              entry->parsed[entry->sel_len].id = mrg_intern_string (section);
              break;
            case '.':
              {
                int i = 0;
                for (i = 0; entry->parsed[entry->sel_len].classes[i]; i++);
                entry->parsed[entry->sel_len].classes[i] = mrg_intern_string (section);
              }
              break;
            case ':':
              {
                int i = 0;
                for (i = 0; entry->parsed[entry->sel_len].pseudo[i]; i++);
                entry->parsed[entry->sel_len].pseudo[i] = mrg_intern_string (section);
              }
              break;
          }
        if (*p == ' ' || *p == 0)
        entry->sel_len ++;
        }
        if (*p == 0)
        {
          return;
        }
        section[(sec_l=0)] = 0;
        type = *p;
        break;
      default:
        section[sec_l++] = *p;
        section[sec_l] = 0;
        break;
    }
  }
}

static void mrg_stylesheet_add_selector (Mrg *mrg, const char *selector, const char *css, int priority)
{
  StyleEntry *entry = calloc (sizeof (StyleEntry), 1);
  entry->selector = strdup (selector);
  entry->css = strdup (css);
  entry->specificity = compute_specificity (selector, priority);
  mrg_parse_selector (mrg, selector, entry);
  mrg_list_prepend_full (&mrg->stylesheet, entry, (void*)free_entry, NULL);
}

#define MAXLEN 4096

#define MAKE_ERROR \
 if (error)\
 {\
   char errbuf[128];\
   sprintf (errbuf, "%i unexpected %c at %i'  %c%c%c", __LINE__, *p, (int)(p-css),\
     p[0], p[1], p[2]);\
   *error = strdup (errbuf);\
 }

#define MAX_RULES 64

typedef struct _MrgCssParseState MrgCssParseState;

struct _MrgCssParseState {
  int   state;
  char  rule[MAX_RULES][MAXLEN];
  int   rule_no ;
  int   rule_l[MAX_RULES];
  char  val[MAXLEN];
  int   val_l;
};

/* XXX: this funciton has no buffer bounds checking */
/* doesnt balance {} () [] and quotes */

enum
{
  NEUTRAL = 0,
  NEUTRAL_COMMENT,
  IN_RULE,
  RULE_COMMENT,
  IN_ARULE,
  ARULE_COMMENT,
  IN_IMPORT,
  IMPORT_COMMENT,
  EXPECT_COLON,
  COLON_COMMENT,
  EXPECT_VAL,
  EXPECT_VAL_COMMENT,
  IN_VAL,
  VAL_COMMENT,
};

static void _mrg_stylesheet_add (MrgCssParseState *ps, Mrg *mrg, const char *css, const char *uri_base,
                         int priority, char **error)
{
  const char *p;
  if (!ps)
    ps = mrg->css_parse_state = calloc (sizeof (MrgCssParseState), 1);

  if (!css)
    return;

  for (p = css; *p; p++)
  {
    switch (ps->state)
    {
      case NEUTRAL_COMMENT:
      case RULE_COMMENT:
      case ARULE_COMMENT:
      case IMPORT_COMMENT:
      case VAL_COMMENT:
      case EXPECT_VAL_COMMENT:
      case COLON_COMMENT:      if (p[0] == '*' && p[1] == '/') { p++; ps->state--; } break;
      case NEUTRAL:
        switch (*p)
        {
          case '/': if (p[1] == '*') { p++; ps->state = NEUTRAL_COMMENT; } break;
          case ' ':
          case '\n':
          case '\t':
          case ';':
            break;
          case '{':
          case '}':
            MAKE_ERROR;
            return;
          case '@':
            ps->state = IN_ARULE;
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]++] = *p;
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]] = 0;
            break;
          default:
            ps->state = IN_RULE;
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]++] = *p;
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]] = 0;
            break;
        }
        break;
      case IN_ARULE:
        switch (*p)
        {
          case '/': if (p[1] == '*') { p++; ps->state = ARULE_COMMENT; } break;
          case '{':
            ps->state = IN_VAL; // should be AVAL for media sections...
            break;
          case '\n':
          case '\t':
          case ' ':
            if (!strcmp (ps->rule[0], "import"))
            {
              MAKE_ERROR;
            }
            else
            {
              ps->state = IN_IMPORT;
            }
            break;
          case ';':
          case '}':
            MAKE_ERROR;
            return;
          default:
            ps->state = IN_ARULE;
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]++] = *p;
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]] = 0;
            break;
        }
        break;
      case IN_IMPORT:
        switch (*p)
        {
          int no;
          case '/': if (p[1] == '*') { p++; ps->state = IMPORT_COMMENT; } break;
          case ';':
            while (ps->val_l && (
                ps->val[ps->val_l-1] == ' ' ||
                ps->val[ps->val_l-1] == '\n' ||
                ps->val[ps->val_l-1] == '\t'))
              ps->val_l--;
            while (ps->val_l && ps->val[ps->val_l-1] != ')')
              ps->val_l--;
            if (ps->val[ps->val_l-1] == ')')
              ps->val_l--;
            if (ps->val[ps->val_l-1] == '"')
              ps->val_l--;
            ps->val[ps->val_l]=0;

            if(mrg->mrg_get_contents){
              char *contents;
              long length;
              char *uri = ps->val;

              /* really ugly string trimming to get to import uri.. */
              while (uri[0]==' ') uri++;
              if (!memcmp (uri, "url", 3)) uri+=3;
              if (uri[0]=='(') uri++;
              if (uri[0]=='"') uri++;
      
              /* XXX: should parse out the media part, and check if we should
               * really include this file
               */

              mrg_get_contents (mrg, uri_base, uri, &contents, &length);
              if (contents)
              {
                MrgCssParseState child_parser = {0,};
                _mrg_stylesheet_add (&child_parser, mrg, contents, uri, priority, error);
                free (contents);
              }
            }

            for (no = 0; no < ps->rule_no+1; no ++)
              ps->rule[no][ps->rule_l[no]=0] = 0;
            ps->val_l = 0;
            ps->val[0] = 0;
            ps->rule_no = 0;
            ps->state = NEUTRAL;
            break;
          default:
            ps->state = IN_IMPORT;
            ps->val[ps->val_l++] = *p;
            ps->val[ps->val_l] = 0;
            break;

        }
        break;
      case IN_RULE:
        switch (*p)
        {
          case '/': if (p[1] == '*') { p++; ps->state = RULE_COMMENT; } break;
          case '{':
            ps->state = IN_VAL;
            break;
          case '\n':
          case '\t':
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]++] = ' ';
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]] = 0;
            break;
          case ',':
            ps->state = NEUTRAL;
            ps->rule_no++;
            break;
          case ';':
          case '}':
            MAKE_ERROR;
            return;
          default:
            ps->state = IN_RULE;
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]++] = *p;
            ps->rule[ps->rule_no][ps->rule_l[ps->rule_no]] = 0;
            break;
        }
        break;
      case EXPECT_COLON:
        switch (*p)
        {
          case '/': if (p[1] == '*') { p++; ps->state = COLON_COMMENT; } break;
          case ' ':
          case '\n':
          case '\t':
            break;
          case ':':
            ps->state = EXPECT_VAL;
            break;
          default:
            MAKE_ERROR;
            return;
        }
        break;
      case EXPECT_VAL:
        switch (*p)
        {
          case '/': if (p[1] == '*') { p++; ps->state = EXPECT_VAL_COMMENT; } break;
          case ' ':
          case '\n':
          case '\t':
          case ';':
            break;
          case '{':
            ps->state = IN_VAL;
            break;
          default:
            MAKE_ERROR;
            return;
        }
        break;
      case IN_VAL:
        switch (*p)
        {
          int no;

          /* XXX: parsing grammar is a bit more complicated, wrt quotes and
           * brackets..
           */

          case '/': if (p[1] == '*') { p++; ps->state = VAL_COMMENT; } break;
          case '}':
            while (ps->val_l && (
                ps->val[ps->val_l-1] == ' ' ||
                ps->val[ps->val_l-1] == '\n' ||
                ps->val[ps->val_l-1] == '\t'))
              ps->val_l--;
            ps->val[ps->val_l]=0;
            for (no = 0; no < ps->rule_no+1; no ++)
            {
              while (ps->rule_l[no] && (
                  ps->rule[no][ps->rule_l[no]-1] == ' ' ||
                  ps->rule[no][ps->rule_l[no]-1] == '\n' ||
                  ps->rule[no][ps->rule_l[no]-1] == '\t'))
                ps->rule_l[no]--;
              ps->rule[no][ps->rule_l[no]]=0;

              mrg_stylesheet_add_selector (mrg, ps->rule[no], ps->val, priority);
              ps->rule[no][ps->rule_l[no]=0] = 0;
            }

            ps->val_l = 0;
            ps->val[0] = 0;
            ps->rule_no = 0;
            ps->state = NEUTRAL;
            break;
          default:
            ps->state = IN_VAL;
            ps->val[ps->val_l++] = *p;
            ps->val[ps->val_l] = 0;
            break;

        }
        break;
    }
  }
}

void mrg_stylesheet_add (Mrg *mrg, const char *css, const char *uri_base,
                         int priority, char **error)
{
  MrgCssParseState *ps = mrg->css_parse_state;
  _mrg_stylesheet_add (ps, mrg, css, uri_base, priority, error);
}

typedef struct StyleMatch
{
  StyleEntry *entry;
  int score;
} StyleMatch;

static int compare_matches (const void *a, const void *b, void *d)
{
  const StyleMatch *ma = a;
  const StyleMatch *mb = b;
  return ma->score - mb->score;
}

static inline int _mrg_nth_match (const char *selector, int child_no)
{
  const char *tmp = selector + 10;
  int a = 0;
  int b = 0;

  if (!strcmp (tmp, "odd)"))
  {
    a = 2; b = 1;
  }
  else if (!strcmp (tmp, "even)"))
  {
    a = 2; b = 0;
  }
  else
  {
    if (strchr (tmp, 'n'))
    {
      a = atoi (tmp);
      b = atoi (strchr (tmp, 'n')+1);
    }
    else
    {
      b = atoi (tmp);
    }
  }

  if (!a)
    return child_no == b;
  else
    if (child_no == b ||
       ((child_no - b > 0) == (a>0)))
      return !((child_no - b) % a);

  return 0;
}

static inline int match_nodes (Mrg *mrg, MrgStyleNode *sel_node, MrgStyleNode *subject)
{
  int j, k;

  if (sel_node->element &&
      sel_node->element != subject->element)
    return 0;

  if (sel_node->id &&
      sel_node->id != subject->id)
    return 0;

  for (j = 0; sel_node->classes[j]; j++)
  {
    int found = 0;
    for (k = 0; subject->classes[k] && !found; k++)
    {
      if (sel_node->classes[j] == subject->classes[k])
        found = 1;
    }
    if (!found)
      return 0;
  }
  for (j = 0; sel_node->pseudo[j]; j++)
  {
    if (!strcmp (sel_node->pseudo[j], "first-child"))
    {
      if (!(_mrg_child_no (mrg) == 1))
        return 0;
    }
    else if (!strncmp (sel_node->pseudo[j], "nth-child(", 10))
    {
      if (!_mrg_nth_match (sel_node->pseudo[j], _mrg_child_no (mrg)))
        return 0;
    }
    else
    {
      int found = 0;

      for (k = 0; subject->pseudo[k] && !found; k++)
      {
        if (sel_node->pseudo[j] == subject->pseudo[k])
          found = 1;
      }
      if (!found)
        return 0;
    }
  }

  return 1;
}

static int mrg_selector_vs_ancestry (Mrg *mrg, StyleEntry *entry, MrgStyleNode **ancestry, int a_depth)
{
  int s = entry->sel_len - 1;

  /* right most part of selector must match */
  if (!match_nodes (mrg, &entry->parsed[s], ancestry[a_depth-1]))
    return 0;

  s--;
  a_depth--;

  if (s < 0)
    return 1;

  while (s >= 0)
  {
    int ai;
    int found_node = 0;

  /* XXX: deal with '>' */
    // if (entry->parsed[s].direct_ancestor) //
    for (ai = a_depth-1; ai >= 0 && !found_node; ai--)
    {
      if (match_nodes (mrg, &entry->parsed[s], ancestry[ai]))
        found_node = 1;
    }
    if (found_node)
    {
      a_depth = ai;
    }
    else
    {
      return 0;
    }
    s--;
  }

  return 1;
}

static int mrg_css_selector_match (Mrg *mrg, StyleEntry *entry, MrgStyleNode **ancestry, int a_depth)
{
  if (entry->selector[0] == '*' &&
      entry->selector[1] == 0)
    return entry->specificity;

  if (a_depth == 0)
    return 0;

  if (mrg_selector_vs_ancestry (mrg, entry, ancestry, a_depth))
    return entry->specificity;

  return 0;
}

static char *_mrg_css_compute_style (Mrg *mrg, MrgStyleNode **ancestry, int a_depth)
{
  MrgList *l;
  MrgList *matches = NULL;
  int totlen = 2;
  char *ret = NULL;

  for (l = mrg->stylesheet; l; l = l->next)
  {
    StyleEntry *entry = l->data;
    int score = 0;
    score = mrg_css_selector_match (mrg, entry, ancestry, a_depth);

    if (score)
    {
      StyleMatch *match = malloc (sizeof (StyleMatch));
      match->score = score;
      match->entry = entry;
      mrg_list_prepend_full (&matches, match, (void*)free, NULL);
      totlen += strlen (entry->css) + 1;
    }
  }

  if (matches)
  {
    MrgList *l;
    char *p;

    p = ret = malloc (totlen);

    mrg_list_sort (&matches, compare_matches, NULL);
    for (l = matches; l; l = l->next)
    {
      StyleMatch *match = l->data;
      StyleEntry *entry = match->entry;
      strcpy (p, entry->css);
      p += strlen (entry->css);
      strcpy (p, ";");
      p ++;
    }
    mrg_list_free (&matches);
  }
  return ret;
}

static int _mrg_get_ancestry (Mrg *mrg, MrgStyleNode **ancestry)
{
  int i, j;
  for (i = 0, j = 0; i <= mrg->state_no; i++)
    if (mrg->states[i].style_id)
    {
      ancestry[j++] = &(mrg->states[i].style_node);
    }
  ancestry[j] = NULL;
  return j;
}

char *_mrg_stylesheet_collate_style (Mrg *mrg)
{
  MrgStyleNode *ancestry[MRG_MAX_STYLE_DEPTH];
  int ancestors = _mrg_get_ancestry (mrg, ancestry);
  char *ret = _mrg_css_compute_style (mrg, ancestry, ancestors);
  return ret;
}

void  mrg_set_line_height (Mrg *mrg, float line_height)
{
  if (mrg_is_terminal (mrg))
    line_height = 1.0;
  mrg_style (mrg)->line_height = line_height;
}

float mrg_line_height (Mrg *mrg)
{
  return mrg_style (mrg)->line_height;
}

float mrg_rem             (Mrg *mrg)
{
  return mrg->rem;
}

void  mrg_set_rem         (Mrg *mrg, float em)
{
  mrg->rem = em;
}

float mrg_em (Mrg *mrg)
{
  return mrg->state->style.font_size;
}

void  mrg_set_em (Mrg *mrg, float em)
{
  if (mrg_is_terminal (mrg)) /* XXX: not _really_ neccesary, and disables huge-title fonts,.."and (small-font vnc)" */
    em = CPX / mrg->ddpx;
  mrg->state->style.font_size = em;
}

void mrg_css_set (Mrg *mrg, const char *css)
{
  mrg_string_set (mrg->style, css);
}

void mrg_css_add (Mrg *mrg, const char *css)
{
  mrg_string_append_str (mrg->style, css);
}

