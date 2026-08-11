// Jason Brillante "Damdoshi"
// Hanged Bunny Studio 2014-2026
// EFRITS SAS 2022-2026
// Pentacle Technologie 2008-2026
//
// GenDiplome

#include		<stdlib.h>
#include		"program.h"

t_bunny_pixelarray	*downsample_pixelarray(t_bunny_pixelarray *src,
					   int factor)
{
  t_bunny_pixelarray	*out;
  unsigned int		*spx;
  unsigned int		*opx;
  int			ow;
  int			oh;
  int			sw;
  int			x;
  int			y;
  int			i;
  int			j;

  if (factor <= 1)
    return (src);
  ow = src->clipable.buffer.width / factor;
  oh = src->clipable.buffer.height / factor;
  out = bunny_new_pixelarray(ow, oh);
  if (out == NULL)
    return (NULL);
  spx = src->pixels;
  opx = out->pixels;
  sw = src->clipable.buffer.width;
  for (y = 0; y < oh; ++y)
    for (x = 0; x < ow; ++x)
    {
      unsigned int ar;
      unsigned int ag;
      unsigned int ab;
      unsigned int aa;
      t_bunny_color c;

      ar = 0;
      ag = 0;
      ab = 0;
      aa = 0;
      for (j = 0; j < factor; ++j)
	for (i = 0; i < factor; ++i)
	{
	  c.full = spx[(y * factor + j) * sw + (x * factor + i)];
	  ar += c.argb[RED_CMP];
	  ag += c.argb[GREEN_CMP];
	  ab += c.argb[BLUE_CMP];
	  aa += c.argb[ALPHA_CMP];
	}
      c.argb[RED_CMP] = ar / (factor * factor);
      c.argb[GREEN_CMP] = ag / (factor * factor);
      c.argb[BLUE_CMP] = ab / (factor * factor);
      c.argb[ALPHA_CMP] = aa / (factor * factor);
      opx[y * ow + x] = c.full;
    }
  return (out);
}
