// Jason Brillante "Damdoshi"
// Hanged Bunny Studio 2014-2026
// EFRITS SAS 2022-2026
// Pentacle Technologie 2008-2026
//
// GenDiplome

#include		<math.h>
#include		"program.h"

static double		distance_to_segment(double px,
				    double py,
				    t_bunny_accurate_position a,
				    t_bunny_accurate_position b)
{
  double		vx;
  double		vy;
  double		wx;
  double		wy;
  double		c1;
  double		c2;
  double		t;
  double		dx;
  double		dy;

  vx = b.x - a.x;
  vy = b.y - a.y;
  wx = px - a.x;
  wy = py - a.y;
  c1 = vx * wx + vy * wy;
  if (c1 <= 0.0)
  {
    dx = px - a.x;
    dy = py - a.y;
    return (sqrt(dx * dx + dy * dy));
  }
  c2 = vx * vx + vy * vy;
  if (c2 <= c1)
  {
    dx = px - b.x;
    dy = py - b.y;
    return (sqrt(dx * dx + dy * dy));
  }
  t = c1 / c2;
  dx = px - (a.x + t * vx);
  dy = py - (a.y + t * vy);
  return (sqrt(dx * dx + dy * dy));
}

void			set_line(t_bunny_pixelarray		*pix,
			 t_bunny_accurate_position	*pos,
			 unsigned int			*col,
			 double				opacity)
{
  int			x;
  int			y;
  int			minx;
  int			maxx;
  int			miny;
  int			maxy;
  double		d;
  double		cov;
  t_bunny_position	p;

  minx = (int)floor(fmin(pos[0].x, pos[1].x) - 2.0);
  maxx = (int)ceil(fmax(pos[0].x, pos[1].x) + 2.0);
  miny = (int)floor(fmin(pos[0].y, pos[1].y) - 2.0);
  maxy = (int)ceil(fmax(pos[0].y, pos[1].y) + 2.0);
  for (y = miny; y <= maxy; ++y)
    for (x = minx; x <= maxx; ++x)
    {
      d = distance_to_segment((double)x + 0.5, (double)y + 0.5,
				      pos[0], pos[1]);
      cov = clamp(1.5 - d) * opacity;
      if (cov > 0.0)
      {
	p.x = x;
	p.y = y;
	set_pixel(pix, p, col[0], cov);
      }
    }
}
