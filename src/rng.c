// Jason Brillante "Damdoshi"
// Hanged Bunny Studio 2014-2026
// EFRITS SAS 2022-2026
// Pentacle Technologie 2008-2026
//
// GenDiplome

#include	"program.h"

typedef struct	s_rng
{
  uint64_t	state;
}		t_rng;

static uint64_t	rng_next(t_rng		*xrng)
{
  uint64_t	x = xrng->state;

  x ^= x >> 12;
  x ^= x << 25;
  x ^= x >> 27;
  xrng->state = x;
  return x * 0x2545F4914F6CDD1DULL;
}

static double	rng_double(t_rng	*xrng)
{
  return (rng_next(xrng) >> 11) * (1.0 / 9007199254740992.0);
}

static int	rng_int(t_rng		*xrng,
			int		min,
			int		max)
{
  return min + (int)(rng_double(xrng) * (double)(max - min + 1));
}

static t_rng	rng;

void		rng_init(t_bunny_hash	r)
{
  rng.state = r;
  if (rng.state == 0)
    rng.state = 0x123456789ABCDEFULL;
}

int		rngi(int		min,
		     int		max)
{
  return (rng_int(&rng, min, max));
}

double		rngf(void)
{
  return (rng_double(&rng));
}

