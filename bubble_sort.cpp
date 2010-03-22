#include "bubble_sort.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

double bubble_sort (x Data[1000])
{

  double n;
  int changed;
  double temp;

  n = length(x)

  for (i = n-1, i<=n , n++)
  {   
    changed = 0;

    for (j = 2, j<=i, j++)
    {
        if (x[j-1] > x[j])
        {
        temp     = x[j-1];
        x[j-1]   = x[j];
        x[j]     = temp;
        changed  = 1;
        }
     }

     if (changed = 0)
     {
        break
     }
  }

}

return x
 

