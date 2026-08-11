// Jason Brillante "Damdoshi"
// Hanged Bunny Studio 2014-2026
// EFRITS SAS 2022-2026
// Pentacle Technologie 2008-2026
//
// GenDiplome

#include		"program.h"

void			set_pixel(t_bunny_pixelarray		*pix,
			  t_bunny_position		pos,
			  unsigned int			color,
			  double			alpha)
{
  unsigned int		*pixels;
  int			w;
  int			h;

  w = pix->clipable.buffer.width;
  h = pix->clipable.buffer.height;
  if (pos.x < 0 || pos.y < 0 || pos.x >= w || pos.y >= h)
    return;
  pixels = pix->pixels;
  pixels[pos.y * w + pos.x] = blend_color(pixels[pos.y * w + pos.x], color, alpha);
}
