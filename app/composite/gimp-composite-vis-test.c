#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>

#include <glib-object.h>

#include "base/base-types.h"

#include "gimp-composite.h"
#include "gimp-composite-regression.h"
#include "gimp-composite-util.h"
#include "gimp-composite-generic.h"
#include "gimp-composite-vis.h"

int
gimp_composite_vis_test (int iterations, int n_pixels)
{
#if defined(COMPILE_VIS_IS_OKAY)
  GimpCompositeContext generic_ctx;
  GimpCompositeContext special_ctx;
  double ft0;
  double ft1;
  gimp_rgba8_t *rgba8D1;
  gimp_rgba8_t *rgba8D2;
  gimp_rgba8_t *rgba8A;
  gimp_rgba8_t *rgba8B;
  gimp_rgba8_t *rgba8M;
  gimp_va8_t *va8A;
  gimp_va8_t *va8B;
  gimp_va8_t *va8M;
  gimp_va8_t *va8D1;
  gimp_va8_t *va8D2;
  int i;

  printf("\nRunning gimp_composite_vis tests...\n");
  if (gimp_composite_vis_init () == 0)
    {
      printf("gimp_composite_vis: Instruction set is not available.\n");
      return (0);
    }

  rgba8A =  gimp_composite_regression_random_rgba8(n_pixels+1);
  rgba8B =  gimp_composite_regression_random_rgba8(n_pixels+1);
  rgba8M =  gimp_composite_regression_random_rgba8(n_pixels+1);
  rgba8D1 = (gimp_rgba8_t *) calloc(sizeof(gimp_rgba8_t), n_pixels+1);
  rgba8D2 = (gimp_rgba8_t *) calloc(sizeof(gimp_rgba8_t), n_pixels+1);
  va8A =    (gimp_va8_t *)   calloc(sizeof(gimp_va8_t), n_pixels+1);
  va8B =    (gimp_va8_t *)   calloc(sizeof(gimp_va8_t), n_pixels+1);
  va8M =    (gimp_va8_t *)   calloc(sizeof(gimp_va8_t), n_pixels+1);
  va8D1 =   (gimp_va8_t *)   calloc(sizeof(gimp_va8_t), n_pixels+1);
  va8D2 =   (gimp_va8_t *)   calloc(sizeof(gimp_va8_t), n_pixels+1);

  for (i = 0; i < n_pixels; i++)
    {
      va8A[i].v = i;
      va8A[i].a = 255-i;
      va8B[i].v = i;
      va8B[i].a = i;
      va8M[i].v = i;
      va8M[i].a = i;
    }

#endif
  return (0);
}

int
main (int argc, char *argv[])
{
  int iterations;
  int n_pixels;

  srand (314159);

  putenv ("GIMP_COMPOSITE=0x1");

  iterations = 10;
  n_pixels = 1048593;

  argv++, argc--;
  while (argc >= 2)
    {
      if (argc > 1 && (strcmp (argv[0], "--iterations") == 0 || strcmp (argv[0], "-i") == 0))
        {
          iterations = atoi(argv[1]);
          argc -= 2, argv++; argv++;
        }
      else if (argc > 1 && (strcmp (argv[0], "--n-pixels") == 0 || strcmp (argv[0], "-n") == 0))
        {
          n_pixels = atoi (argv[1]);
          argc -= 2, argv++; argv++;
        }
      else
        {
          printf("Usage: gimp-composites-*-test [-i|--iterations n] [-n|--n-pixels n]");
          exit(1);
        }
    }

  gimp_composite_generic_install ();

  return (gimp_composite_vis_test (iterations, n_pixels));
}
