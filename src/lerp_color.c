// Jason Brillante "Damdoshi"
// Hanged Bunny Studio 2014-2026
// EFRITS SAS 2022-2026
// Pentacle Technologie 2008-2026
//
// GenDiplome

#include		"program.h"

static unsigned char	lerp_u8(unsigned char			a,
				unsigned char			b,
				double				t)
{
  return (unsigned char)((double)a + ((double)b - (double)a) * t + 0.5);
}

unsigned int		lerp_color(unsigned int			ca,
				   unsigned int			cb,
				   double			t)
{
  t_bunny_color		a;
  t_bunny_color		b;
  t_bunny_color		out;

  t = clamp(t);
  a.full = ca;
  b.full = cb;

  out.argb[RED_CMP] = lerp_u8(a.argb[RED_CMP], b.argb[RED_CMP], t);
  out.argb[GREEN_CMP] = lerp_u8(a.argb[GREEN_CMP], b.argb[GREEN_CMP], t);
  out.argb[BLUE_CMP] = lerp_u8(a.argb[BLUE_CMP], b.argb[BLUE_CMP], t);
  out.argb[ALPHA_CMP] = lerp_u8(a.argb[ALPHA_CMP], b.argb[ALPHA_CMP], t);

  return out.full;
}
