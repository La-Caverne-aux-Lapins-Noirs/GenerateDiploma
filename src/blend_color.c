// Jason Brillante "Damdoshi"
// Hanged Bunny Studio 2014-2026
// EFRITS SAS 2022-2026
// Pentacle Technologie 2008-2026
//
// GenDiplome

#include		"program.h"

unsigned int		blend_color(unsigned int		dst,
			    unsigned int		src,
			    double			alpha_mul)
{
  t_bunny_color		d;
  t_bunny_color		s;
  t_bunny_color		out;
  double		a;

  d.full = dst;
  s.full = src;
  a = ((double)s.argb[ALPHA_CMP] / 255.0) * clamp(alpha_mul);
  out.argb[RED_CMP] =
    (unsigned char)((double)d.argb[RED_CMP] * (1.0 - a) +
		    (double)s.argb[RED_CMP] * a + 0.5);
  out.argb[GREEN_CMP] =
    (unsigned char)((double)d.argb[GREEN_CMP] * (1.0 - a) +
		    (double)s.argb[GREEN_CMP] * a + 0.5);
  out.argb[BLUE_CMP] =
    (unsigned char)((double)d.argb[BLUE_CMP] * (1.0 - a) +
		    (double)s.argb[BLUE_CMP] * a + 0.5);
  out.argb[ALPHA_CMP] = 255;
  return (out.full);
}
