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

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mrg.h"

int session_main (int argc, char **argv)
{
  int i;
  if (getenv ("MMM_PATH"))
  {
    printf ("MMM_PATH is set in environment, pass -f to force nesting");
    if (argv[1] == 0 || strcmp(argv[1], "-f"))
      return -1;
  }

  for (i = 0; i < 8; i++)
  {
    system ("mrg-host");
  }

  return 0;
}
