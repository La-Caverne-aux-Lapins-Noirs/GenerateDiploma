// Jason Brillante "Damdoshi"
// Hanged Bunny Studio 2014-2026
// EFRITS SAS 2022-2026
// Pentacle Technologie 2008-2026
//
// GenDiplome

#include		<string.h>
#include		<stddef.h>
#include		<stdio.h>
#include		<math.h>
#include		"program.h"

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

typedef struct		s_line_fx
{
  int			glow_style;
  double		glow_phase;
  double		glow_freq;
  double		glow_amplitude;
  double		laser_t1;
  double		laser_t2;
  double		laser_width1;
  double		laser_width2;
  double		laser_strength1;
  double		laser_strength2;
} 			t_line_fx;

typedef struct		s_botanical_ctx
{
  t_bunny_pixelarray	*pix;
  const t_diploma_style	*style;
  unsigned int		color;
  int			pair_mode;
  int			ss;
  double			extrude_x;
  double			extrude_y;
} 			t_botanical_ctx;

typedef struct		s_vec3
{
  double			x;
  double			y;
  double			z;
} 			t_vec3;

typedef struct		s_edge3
{
  int			a;
  int			b;
} 			t_edge3;

typedef enum		e_corner_object_kind
{
  OBJ_OVOID,
  OBJ_CUBE,
  OBJ_CONE,
  OBJ_HOURGLASS,
  OBJ_CYLINDER,
  OBJ_TORUS
} 			t_corner_object_kind;

static void		draw_corner_identifier_objects(t_bunny_pixelarray *pix,
			     const t_diploma_style *style);
static void		draw_upper_right_waves(t_bunny_pixelarray *pix,
			      const t_diploma_style *style);


static void		fill_pixelarray(t_bunny_pixelarray *pix,
					unsigned int color)
{
  unsigned int		*pixels;
  size_t			nb;
  size_t			i;

  pixels = pix->pixels;
  nb = (size_t)pix->clipable.buffer.width * (size_t)pix->clipable.buffer.height;
  for (i = 0; i < nb; ++i)
    pixels[i] = color;
}

static unsigned int	with_alpha(unsigned int color,
				   double alpha_mul)
{
  t_bunny_color		c;

  c.full = color;
  c.argb[ALPHA_CMP] = (unsigned char)(c.argb[ALPHA_CMP] * clamp(alpha_mul));
  return (c.full);
}

static t_bunny_hash	tagged_seed(const char *codename,
				    const char *tag,
				    const char *secret)
{
  char			buf[1024];

  if (diploma_secret_is_disabled(secret))
    {
      snprintf(buf, sizeof(buf), "%s#%s",
	       codename != NULL ? codename : "", tag != NULL ? tag : "");
      return (bunny_hash(BH_DJB2, buf, strlen(buf)));
    }
  return ((t_bunny_hash)diploma_keyed_hash64(secret, tag, codename));
}

static double		distance_to_segment(double px,
					 double py,
					 double ax,
					 double ay,
					 double bx,
					 double by)
{
  double			vx;
  double			vy;
  double			wx;
  double			wy;
  double			c1;
  double			c2;
  double			t;
  double			dx;
  double			dy;

  vx = bx - ax;
  vy = by - ay;
  wx = px - ax;
  wy = py - ay;
  c1 = vx * wx + vy * wy;
  if (c1 <= 0.0)
  {
    dx = px - ax;
    dy = py - ay;
    return (sqrt(dx * dx + dy * dy));
  }
  c2 = vx * vx + vy * vy;
  if (c2 <= c1)
  {
    dx = px - bx;
    dy = py - by;
    return (sqrt(dx * dx + dy * dy));
  }
  t = c1 / c2;
  dx = px - (ax + t * vx);
  dy = py - (ay + t * vy);
  return (sqrt(dx * dx + dy * dy));
}

static double		segment_parameter(double px,
					  double py,
					  double ax,
					  double ay,
					  double bx,
					  double by)
{
  double			vx;
  double			vy;
  double			len2;
  double			t;

  vx = bx - ax;
  vy = by - ay;
  len2 = vx * vx + vy * vy;
  if (len2 <= 0.0)
    return (0.0);
  t = ((px - ax) * vx + (py - ay) * vy) / len2;
  return (clamp(t));
}

static double		gaussian_peak(double x,
				      double center,
				      double width)
{
  double			d;

  if (width <= 0.0001)
    return (0.0);
  d = (x - center) / width;
  return (exp(-d * d));
}

static double		segment_light_mod(double t,
				      const t_line_fx *fx)
{
  double			mod;
  double			wave;

  mod = 1.0;
  wave = 0.5 + 0.5 * sin(fx->glow_phase + t * fx->glow_freq * 2.0 * M_PI);
  mod *= (1.0 - fx->glow_amplitude) + fx->glow_amplitude * wave;
  if (fx->glow_style >= 6)
  {
    mod += fx->laser_strength1 * gaussian_peak(t, fx->laser_t1, fx->laser_width1);
    mod += fx->laser_strength2 * gaussian_peak(t, fx->laser_t2, fx->laser_width2);
  }
  if (fx->glow_style >= 8)
    mod += 0.18 * gaussian_peak(t, 0.5, 0.18);
  return (mod);
}

static void		draw_segment_stroke_alpha(t_bunny_pixelarray *pix,
					 double ax,
					 double ay,
					 double bx,
					 double by,
					 double thickness,
					 unsigned int color,
					 double opacity)
{
  int			minx;
  int			maxx;
  int			miny;
  int			maxy;
  int			x;
  int			y;
  double		half;
  double		dist;
  double		cov;
  t_bunny_position	pos;

  half = thickness * 0.5;
  minx = (int)floor(fmin(ax, bx) - half - 2.0);
  maxx = (int)ceil(fmax(ax, bx) + half + 2.0);
  miny = (int)floor(fmin(ay, by) - half - 2.0);
  maxy = (int)ceil(fmax(ay, by) + half + 2.0);
  for (y = miny; y <= maxy; ++y)
    for (x = minx; x <= maxx; ++x)
    {
      dist = distance_to_segment((double)x + 0.5, (double)y + 0.5,
				 ax, ay, bx, by);
      cov = clamp(half + 0.5 - dist) * opacity;
      if (cov <= 0.0)
	continue;
      pos.x = x;
      pos.y = y;
      set_pixel(pix, pos, color, cov);
    }
}

static void		draw_segment_glow(t_bunny_pixelarray *pix,
					 double ax,
					 double ay,
					 double bx,
					 double by,
					 double thickness,
					 double radius,
					 unsigned int color,
					 double opacity,
					 double exponent,
					 const t_line_fx *fx)
{
  int			minx;
  int			maxx;
  int			miny;
  int			maxy;
  int			x;
  int			y;
  double		inner;
  double		outer;
  double		dist;
  double		alpha;
  double		t;
  double		st;
  t_bunny_position	pos;

  if (radius <= 0.0 || opacity <= 0.0)
    return;
  inner = thickness * 0.5;
  outer = inner + radius;
  minx = (int)floor(fmin(ax, bx) - outer - 2.0);
  maxx = (int)ceil(fmax(ax, bx) + outer + 2.0);
  miny = (int)floor(fmin(ay, by) - outer - 2.0);
  maxy = (int)ceil(fmax(ay, by) + outer + 2.0);
  for (y = miny; y <= maxy; ++y)
    for (x = minx; x <= maxx; ++x)
    {
      dist = distance_to_segment((double)x + 0.5, (double)y + 0.5,
				 ax, ay, bx, by);
      if (dist > outer)
	continue;
      if (dist <= inner)
	alpha = opacity;
      else
      {
	t = (dist - inner) / (outer - inner);
	alpha = opacity * pow(1.0 - clamp(t), exponent);
      }
      st = segment_parameter((double)x + 0.5, (double)y + 0.5, ax, ay, bx, by);
      alpha *= segment_light_mod(st, fx);
      if (alpha <= 0.0)
	continue;
      pos.x = x;
      pos.y = y;
      set_pixel(pix, pos, color, alpha);
    }
}

static void		draw_segment_stroke(t_bunny_pixelarray *pix,
					   double ax,
					   double ay,
					   double bx,
					   double by,
					   double thickness,
					   unsigned int color,
					   const t_line_fx *fx)
{
  int			minx;
  int			maxx;
  int			miny;
  int			maxy;
  int			x;
  int			y;
  double		half;
  double		dist;
  double		cov;
  double		st;
  t_bunny_position	pos;

  half = thickness * 0.5;
  minx = (int)floor(fmin(ax, bx) - half - 2.0);
  maxx = (int)ceil(fmax(ax, bx) + half + 2.0);
  miny = (int)floor(fmin(ay, by) - half - 2.0);
  maxy = (int)ceil(fmax(ay, by) + half + 2.0);
  for (y = miny; y <= maxy; ++y)
    for (x = minx; x <= maxx; ++x)
    {
      dist = distance_to_segment((double)x + 0.5, (double)y + 0.5,
				 ax, ay, bx, by);
      cov = clamp(half + 0.5 - dist);
      if (cov <= 0.0)
	continue;
      st = segment_parameter((double)x + 0.5, (double)y + 0.5, ax, ay, bx, by);
      cov *= segment_light_mod(st, fx);
      pos.x = x;
      pos.y = y;
      set_pixel(pix, pos, color, cov);
    }
}

static void		draw_segment_with_glow_style(t_bunny_pixelarray *pix,
					     double ax,
					     double ay,
					     double bx,
					     double by,
					     int thickness,
					     unsigned int color,
					     const t_line_fx *fx,
					     int ss)
{
  if (fx->glow_style == 1)
    draw_segment_glow(pix, ax, ay, bx, by, thickness,
		      4.0 * ss, color, 0.16, 2.8, fx);
  else if (fx->glow_style == 2)
    draw_segment_glow(pix, ax, ay, bx, by, thickness,
		      8.0 * ss, color, 0.22, 1.9, fx);
  else if (fx->glow_style == 3)
    draw_segment_glow(pix, ax, ay, bx, by, thickness,
		      14.0 * ss, color, 0.18, 1.35, fx);
  else if (fx->glow_style == 4)
  {
    draw_segment_glow(pix, ax, ay, bx, by, thickness,
		      4.0 * ss, color, 0.24, 3.0, fx);
    draw_segment_glow(pix, ax, ay, bx, by, thickness,
		      12.0 * ss, color, 0.12, 1.45, fx);
  }
  else if (fx->glow_style == 5)
  {
    draw_segment_glow(pix, ax, ay, bx, by, thickness,
		      6.0 * ss, color, 0.26, 3.2, fx);
    draw_segment_glow(pix, ax, ay, bx, by, thickness,
		      18.0 * ss, color, 0.16, 1.20, fx);
  }
  else if (fx->glow_style == 6)
  {
    draw_segment_glow(pix, ax, ay, bx, by, thickness,
		      5.0 * ss, color, 0.34, 3.8, fx);
    draw_segment_glow(pix, ax, ay, bx, by, thickness,
		      10.0 * ss, color, 0.12, 2.0, fx);
  }
  else if (fx->glow_style == 7)
  {
    draw_segment_glow(pix, ax, ay, bx, by, thickness,
		      3.5 * ss, color, 0.40, 4.5, fx);
    draw_segment_glow(pix, ax, ay, bx, by, thickness,
		      16.0 * ss, color, 0.08, 1.3, fx);
  }
  else if (fx->glow_style >= 8)
  {
    draw_segment_glow(pix, ax, ay, bx, by, thickness,
		      4.0 * ss, color, 0.44, 5.0, fx);
    draw_segment_glow(pix, ax, ay, bx, by, thickness,
		      9.0 * ss, color, 0.20, 2.6, fx);
    draw_segment_glow(pix, ax, ay, bx, by, thickness,
		      24.0 * ss, color, 0.07, 1.1, fx);
  }
  draw_segment_stroke(pix, ax, ay, bx, by, thickness, color, fx);
}

static void		draw_corner_frame(t_bunny_pixelarray *pix,
					  int margin,
					  int hlength,
					  int vlength,
					  int thickness,
					  unsigned int color,
					  const t_line_fx *fx,
					  int ss)
{
  int			x0;
  int			y0;
  int			x1;
  int			y1;

  x0 = margin;
  y0 = margin;
  x1 = pix->clipable.buffer.width - margin - thickness;
  y1 = pix->clipable.buffer.height - margin - thickness;
  if (x1 <= x0 || y1 <= y0)
    return;
  if (hlength > (x1 - x0 + thickness) / 2)
    hlength = (x1 - x0 + thickness) / 2;
  if (vlength > (y1 - y0 + thickness) / 2)
    vlength = (y1 - y0 + thickness) / 2;

  draw_segment_with_glow_style(pix, x0, y0 + thickness * 0.5,
			       x0 + hlength, y0 + thickness * 0.5,
			       thickness, color, fx, ss);
  draw_segment_with_glow_style(pix, x0 + thickness * 0.5, y0,
			       x0 + thickness * 0.5, y0 + vlength,
			       thickness, color, fx, ss);

  draw_segment_with_glow_style(pix, x1 - hlength + thickness,
			       y0 + thickness * 0.5,
			       x1 + thickness, y0 + thickness * 0.5,
			       thickness, color, fx, ss);
  draw_segment_with_glow_style(pix, x1 + thickness * 0.5, y0,
			       x1 + thickness * 0.5, y0 + vlength,
			       thickness, color, fx, ss);

  draw_segment_with_glow_style(pix, x0, y1 + thickness * 0.5,
			       x0 + hlength, y1 + thickness * 0.5,
			       thickness, color, fx, ss);
  draw_segment_with_glow_style(pix, x0 + thickness * 0.5,
			       y1 - vlength + thickness,
			       x0 + thickness * 0.5, y1 + thickness,
			       thickness, color, fx, ss);

  draw_segment_with_glow_style(pix, x1 - hlength + thickness,
			       y1 + thickness * 0.5,
			       x1 + thickness, y1 + thickness * 0.5,
			       thickness, color, fx, ss);
  draw_segment_with_glow_style(pix, x1 + thickness * 0.5,
			       y1 - vlength + thickness,
			       x1 + thickness * 0.5, y1 + thickness,
			       thickness, color, fx, ss);
}

static void		generate_line_fx(t_line_fx *fx,
				     const t_diploma_style *style)
{
  fx->glow_style = rngi(style->border_glow_min_style,
			style->border_glow_max_style);
  fx->glow_phase = rngf() * 2.0 * M_PI;
  fx->glow_freq = 1.2 + rngf() * 3.2;
  fx->glow_amplitude = 0.10 + rngf() * 0.22;
  fx->laser_t1 = 0.18 + rngf() * 0.22;
  fx->laser_t2 = 0.62 + rngf() * 0.20;
  fx->laser_width1 = 0.05 + rngf() * 0.06;
  fx->laser_width2 = 0.05 + rngf() * 0.08;
  fx->laser_strength1 = 0.20 + rngf() * 0.28;
  fx->laser_strength2 = 0.12 + rngf() * 0.22;
  if (fx->glow_style == 0)
    fx->glow_amplitude *= 0.45;
  if (fx->glow_style >= 6)
  {
    fx->glow_amplitude += 0.10;
    fx->glow_freq += 1.2;
  }
}

static void		draw_procedural_border(t_bunny_pixelarray *pix,
					       const t_diploma_style *style)
{
  int			ss;
  int			lines;
  int			thickness;
  int			base_hlength;
  int			base_vlength;
  int			max_vlength;
  int			i;
  int			margin;
  t_line_fx		fx;

  ss = style->super_sampling;
  lines = rngi(style->border_min_lines, style->border_max_lines);
  thickness = rngi(style->border_min_thickness, style->border_max_thickness) * ss;
  base_hlength = rngi(style->border_min_horizontal_length,
		      style->border_max_horizontal_length);
  max_vlength = style->border_max_vertical_length;
  if (max_vlength >= base_hlength)
    max_vlength = base_hlength - 1;
  if (max_vlength < style->border_min_vertical_length)
    max_vlength = style->border_min_vertical_length;
  if (max_vlength >= base_hlength)
    max_vlength = base_hlength > 1 ? base_hlength - 1 : 1;
  if (max_vlength < 1)
    max_vlength = 1;
  base_vlength = rngi(style->border_min_vertical_length, max_vlength);
  if (thickness < 1)
    thickness = 1;
  for (i = 0; i < lines; ++i)
  {
    margin = (style->border_margin + i * style->border_line_spacing) * ss;
    generate_line_fx(&fx, style);
    draw_corner_frame(pix, margin, base_hlength * ss, base_vlength * ss,
		      thickness, style->border_color, &fx, ss);
  }
}

static void		transform_botanical_point(const t_botanical_ctx *ctx,
					 double lx,
					 double ly,
					 double *sx,
					 double *sy)
{
  double			w;

  w = (double)ctx->pix->clipable.buffer.width - 1.0;
  if (ctx->pair_mode == 0)
  {
    *sx = lx;
    *sy = ly;
  }
  else
  {
    *sx = w - lx;
    *sy = ly;
  }
}

static void		draw_botanical_line(const t_botanical_ctx *ctx,
				    double ax,
				    double ay,
				    double bx,
				    double by,
				    double thickness,
				    double opacity)
{
  double			x1;
  double			y1;
  double			x2;
  double			y2;
  double			mx1;
  double			my1;
  double			mx2;
  double			my2;
  double			w;
  double			h;
  unsigned int		col;

  w = (double)ctx->pix->clipable.buffer.width - 1.0;
  h = (double)ctx->pix->clipable.buffer.height - 1.0;
  transform_botanical_point(ctx, ax, ay, &x1, &y1);
  transform_botanical_point(ctx, bx, by, &x2, &y2);
  col = with_alpha(ctx->color, opacity);
  draw_segment_stroke_alpha(ctx->pix, x1, y1, x2, y2, thickness, col, 1.0);
  mx1 = w - x1;
  my1 = h - y1;
  mx2 = w - x2;
  my2 = h - y2;
  draw_segment_stroke_alpha(ctx->pix, mx1, my1, mx2, my2, thickness, col, 1.0);
}

static void		draw_wire_leaf(const t_botanical_ctx *ctx,
				   double cx,
				   double cy,
				   double angle,
				   double size,
				   double thickness)
{
  double			dx;
  double			dy;
  double			px;
  double			py;
  double			bx;
  double			by;
  double			tx;
  double			ty;
  double			lx;
  double			ly;
  double			rx;
  double			ry;
  double			ex;
  double			ey;

  dx = cos(angle);
  dy = sin(angle);
  px = -dy;
  py = dx;
  bx = cx - dx * size * 0.50;
  by = cy - dy * size * 0.50;
  tx = cx + dx * size * 0.64;
  ty = cy + dy * size * 0.64;
  lx = cx + px * size * 0.40;
  ly = cy + py * size * 0.40;
  rx = cx - px * size * 0.40;
  ry = cy - py * size * 0.40;
  ex = ctx->extrude_x * (0.55 + size * 0.0012);
  ey = ctx->extrude_y * (0.55 + size * 0.0012);

  draw_botanical_line(ctx, bx, by, lx, ly, thickness, 0.82);
  draw_botanical_line(ctx, lx, ly, tx, ty, thickness, 1.00);
  draw_botanical_line(ctx, tx, ty, rx, ry, thickness, 1.00);
  draw_botanical_line(ctx, rx, ry, bx, by, thickness, 0.82);
  draw_botanical_line(ctx, bx, by, tx, ty, thickness * 0.92, 0.62);

  draw_botanical_line(ctx, bx + ex, by + ey, lx + ex, ly + ey, thickness, 0.34);
  draw_botanical_line(ctx, lx + ex, ly + ey, tx + ex, ty + ey, thickness, 0.46);
  draw_botanical_line(ctx, tx + ex, ty + ey, rx + ex, ry + ey, thickness, 0.46);
  draw_botanical_line(ctx, rx + ex, ry + ey, bx + ex, by + ey, thickness, 0.34);
  draw_botanical_line(ctx, bx, by, bx + ex, by + ey, thickness * 0.85, 0.30);
  draw_botanical_line(ctx, tx, ty, tx + ex, ty + ey, thickness * 0.85, 0.38);
}

static void		bezier_point(double t,
				  double x0,
				  double y0,
				  double x1,
				  double y1,
				  double x2,
				  double y2,
				  double x3,
				  double y3,
				  double *x,
				  double *y)
{
  double			u;
  double			tt;
  double			uu;
  double			uuu;
  double			ttt;

  u = 1.0 - t;
  tt = t * t;
  uu = u * u;
  uuu = uu * u;
  ttt = tt * t;
  *x = uuu * x0 + 3.0 * uu * t * x1 + 3.0 * u * tt * x2 + ttt * x3;
  *y = uuu * y0 + 3.0 * uu * t * y1 + 3.0 * u * tt * y2 + ttt * y3;
}

static void		bezier_tangent(double t,
				    double x0,
				    double y0,
				    double x1,
				    double y1,
				    double x2,
				    double y2,
				    double x3,
				    double y3,
				    double *dx,
				    double *dy)
{
  double			u;

  u = 1.0 - t;
  *dx = 3.0 * u * u * (x1 - x0)
    + 6.0 * u * t * (x2 - x1)
    + 3.0 * t * t * (x3 - x2);
  *dy = 3.0 * u * u * (y1 - y0)
    + 6.0 * u * t * (y2 - y1)
    + 3.0 * t * t * (y3 - y2);
}

static void		draw_wire_arc_segment(const t_botanical_ctx *ctx,
				   double ax,
				   double ay,
				   double bx,
				   double by,
				   double thickness,
				   double depth_scale)
{
  double			ex;
  double			ey;
  double			mx;
  double			my;

  ex = ctx->extrude_x * depth_scale;
  ey = ctx->extrude_y * depth_scale;
  draw_botanical_line(ctx, ax, ay, bx, by, thickness, 1.0);
  draw_botanical_line(ctx, ax + ex, ay + ey, bx + ex, by + ey,
		      thickness * 0.92, 0.36);
  draw_botanical_line(ctx, ax, ay, ax + ex, ay + ey, thickness * 0.85, 0.26);
  draw_botanical_line(ctx, bx, by, bx + ex, by + ey, thickness * 0.85, 0.34);
  mx = (ax + bx) * 0.5;
  my = (ay + by) * 0.5;
  draw_botanical_line(ctx, mx, my, mx + ex, my + ey,
		      thickness * 0.65, 0.18);
}

static void		draw_arc_spine(const t_botanical_ctx *ctx,
				 double x0,
				 double y0,
				 double x1,
				 double y1,
				 double x2,
				 double y2,
				 double x3,
				 double y3,
				 double thickness)
{
  int			i;
  int			steps;
  double			ax;
  double			ay;
  double			bx;
  double			by;
  double			t;

  steps = 30;
  bezier_point(0.0, x0, y0, x1, y1, x2, y2, x3, y3, &ax, &ay);
  for (i = 1; i <= steps; ++i)
  {
    t = (double)i / (double)steps;
    bezier_point(t, x0, y0, x1, y1, x2, y2, x3, y3, &bx, &by);
    draw_wire_arc_segment(ctx, ax, ay, bx, by,
			  thickness, 0.78 + 0.45 * t);
    ax = bx;
    ay = by;
  }
}

static void		draw_arc_leaflet(const t_botanical_ctx *ctx,
				   double px,
				   double py,
				   double tangent_angle,
				   int side,
				   double scale,
				   double thickness)
{
  double			arm_len;
  double			arm_angle;
  double			tipx;
  double			tipy;
  double			leaf_angle;

  arm_len = scale * (0.28 + rngf() * 0.20);
  arm_angle = tangent_angle + side * (1.00 + rngf() * 0.24);
  tipx = px + cos(arm_angle) * arm_len;
  tipy = py + sin(arm_angle) * arm_len;
  draw_wire_arc_segment(ctx, px, py, tipx, tipy,
			thickness * 0.88, 0.46 + arm_len * 0.0010);

  leaf_angle = arm_angle + side * (0.10 + rngf() * 0.26);
  draw_wire_leaf(ctx, tipx, tipy, leaf_angle,
		 scale * (0.22 + rngf() * 0.14), thickness * 0.94);

  if (rngi(0, 99) < 68)
    draw_wire_leaf(ctx,
		   tipx - cos(arm_angle) * arm_len * 0.22,
		   tipy - sin(arm_angle) * arm_len * 0.22,
		   arm_angle - side * (0.14 + rngf() * 0.22),
		   scale * (0.14 + rngf() * 0.10),
		   thickness * 0.74);
}


static void		draw_circle_arc_trunk(const t_botanical_ctx *ctx,
					      double cx,
					      double cy,
					      double radius,
					      double start,
					      double end,
					      double thickness)
{
  int			i;
  int			steps;
  double		a;
  double		ax;
  double		ay;
  double		bx;
  double		by;
  double		arc_length;

  arc_length = fabs(end - start) * radius;
  steps = (int)(arc_length / (14.0 * ctx->ss));
  if (steps < 24)
    steps = 24;
  if (steps > 180)
    steps = 180;
  a = start;
  ax = cx - cos(a) * radius;
  ay = cy + sin(a) * radius;
  for (i = 1; i <= steps; ++i)
  {
    a = start + (end - start) * ((double)i / (double)steps);
    bx = cx - cos(a) * radius;
    by = cy + sin(a) * radius;
    draw_wire_arc_segment(ctx, ax, ay, bx, by,
			  thickness, 0.75 + 0.35 * ((double)i / (double)steps));
    ax = bx;
    ay = by;
  }
}

static void		draw_branch_segment(const t_botanical_ctx *ctx,
				  double ax,
				  double ay,
				  double bx,
				  double by,
				  double thickness)
{
  double			ex;
  double			ey;

  ex = ctx->extrude_x * 0.35;
  ey = ctx->extrude_y * 0.35;
  draw_botanical_line(ctx, ax, ay, bx, by, thickness, 0.92);
  draw_botanical_line(ctx, ax + ex, ay + ey, bx + ex, by + ey,
		      thickness * 0.72, 0.28);
  draw_botanical_line(ctx, ax, ay, ax + ex, ay + ey, thickness * 0.56, 0.22);
  draw_botanical_line(ctx, bx, by, bx + ex, by + ey, thickness * 0.56, 0.30);
}

static void		normalize_vec(double *x,
			       double *y)
{
  double			len;

  len = sqrt((*x) * (*x) + (*y) * (*y));
  if (len == 0.0)
    return;
  *x /= len;
  *y /= len;
}

static void		draw_fern_branch_recursive(const t_botanical_ctx *ctx,
				     double ax,
				     double ay,
				     double dx,
				     double dy,
				     double trunk_dx,
				     double trunk_dy,
				     double len,
				     double thickness,
				     int depth,
				     int handedness)
{
  double			bx;
  double			by;
  double			nx;
  double			ny;
  double			cx;
  double			cy;
  double			cont_dx;
  double			cont_dy;
  double			cont_len;
  int			barb_count;
  int			i;

  normalize_vec(&dx, &dy);
  bx = ax + dx * len;
  by = ay + dy * len;
  draw_branch_segment(ctx, ax, ay, bx, by, thickness);
  if (depth <= 0 || len < 3.0 * ctx->ss || thickness < 0.22)
    return;

  nx = -dy;
  ny = dx;

  /* Continue the leaflet spine a bit further. */
  cx = ax + (bx - ax) * (0.34 + rngf() * 0.14);
  cy = ay + (by - ay) * (0.34 + rngf() * 0.14);
  cont_dx = dx * (0.86 + rngf() * 0.10)
    + trunk_dx * (0.18 + rngf() * 0.12)
    + handedness * nx * ((rngf() - 0.5) * 0.14);
  cont_dy = dy * (0.86 + rngf() * 0.10)
    + trunk_dy * (0.18 + rngf() * 0.12)
    + handedness * ny * ((rngf() - 0.5) * 0.14);
  normalize_vec(&cont_dx, &cont_dy);
  cont_len = len * (0.46 + rngf() * 0.18);
  draw_fern_branch_recursive(ctx, cx, cy, cont_dx, cont_dy,
			     trunk_dx, trunk_dy,
			     cont_len, thickness * 0.74,
			     depth - 1, handedness);

  barb_count = 1 + (depth > 1 ? 1 : 0);
  if (depth > 2 && rngi(0, 99) < 35)
    barb_count += 1;
  for (i = 0; i < barb_count; ++i)
  {
    double			px;
    double			py;
    double			t;
    double			barb_dx;
    double			barb_dy;
    double			barb_len;

    t = 0.18 + ((double)(i + 1) / (double)(barb_count + 1)) * 0.58
      + (rngf() - 0.5) * 0.08;
    px = ax + (bx - ax) * clamp(t);
    py = ay + (by - ay) * clamp(t);
    barb_dx = dx * (0.24 + rngf() * 0.16)
      + handedness * nx * (0.88 + rngf() * 0.26)
      + trunk_dx * (0.12 + rngf() * 0.10);
    barb_dy = dy * (0.24 + rngf() * 0.16)
      + handedness * ny * (0.88 + rngf() * 0.26)
      + trunk_dy * (0.12 + rngf() * 0.10);
    normalize_vec(&barb_dx, &barb_dy);
    barb_len = len * (0.26 + rngf() * 0.20) * (1.0 - 0.12 * i);
    draw_fern_branch_recursive(ctx, px, py, barb_dx, barb_dy,
			       dx, dy,
			       barb_len, thickness * 0.58,
			       depth - 1, handedness);
  }
}

static void		draw_arc_vegetation(const t_botanical_ctx *ctx,
				  double cx,
				  double cy,
				  double radius,
				  double start,
				  double end,
				  double thickness,
				  double density_scale)
{
  int			count;
  int			i;
  int			side;
  double			t;
  double			a;
  double			px;
  double			py;
  double			tx;
  double			ty;
  double			tlen;
  double			nx;
  double			ny;
  double			base_len;
  double			vx;
  double			vy;
  double			vlen;
  double			along;
  double			outward;

  /*
  ** Fern-like vegetation with richer recursive branchlets.
  ** Primary leaflets are distributed with some jitter along the trunk,
  ** then each of them can recursively produce 3 or 4 levels of
  ** continuation and side branchlets.
  */
  count = (int)(24.0 * density_scale) + rngi(4, 10);
  for (i = 0; i < count; ++i)
  {
    int			branch_count;
    int			j;
    int			depth;
    double			spacing;
    double			base_t;
    double			jitter;
    double			anchor_shift;
    double			apx;
    double			apy;

    spacing = 0.88 / (double)(count > 1 ? count - 1 : 1);
    base_t = 0.06 + 0.88 * ((double)i / (double)(count > 1 ? count - 1 : 1));
    jitter = (rngf() - 0.5) * spacing * 1.20;
    t = clamp(base_t + jitter);
    a = start + (end - start) * t;
    px = cx - cos(a) * radius;
    py = cy + sin(a) * radius;

    tx = sin(a);
    ty = cos(a);
    tlen = sqrt(tx * tx + ty * ty);
    if (tlen == 0.0)
      continue;
    tx /= tlen;
    ty /= tlen;
    nx = -ty;
    ny = tx;

    base_len = radius * (0.060 + rngf() * 0.022) * (1.0 - 0.30 * t);
    branch_count = 1 + (rngi(0, 99) < 82 ? 1 : 0) + (rngi(0, 99) < 34 ? 1 : 0);
    for (side = -1; side <= 1; side += 2)
    {
      for (j = 0; j < branch_count; ++j)
      {
        along = 0.38 + rngf() * 0.24;
        outward = 0.82 + rngf() * 0.30;
        anchor_shift = ((rngf() - 0.5) * 0.028
			+ (j - (branch_count - 1) * 0.5) * 0.012) * radius;
        apx = px + tx * anchor_shift;
        apy = py + ty * anchor_shift;

        vx = tx * along + side * nx * outward;
        vy = ty * along + side * ny * outward;
        vlen = sqrt(vx * vx + vy * vy);
        if (vlen == 0.0)
          continue;
        vx /= vlen;
        vy /= vlen;

        depth = 2 + rngi(1, 2);
        draw_fern_branch_recursive(ctx, apx, apy, vx, vy,
			       tx, ty,
			       base_len * (0.80 + rngf() * 0.26) * (1.0 - 0.08 * j),
			       thickness * 0.60,
			       depth, side);
      }
    }
  }
}

static void		draw_fern_arc_from_root(const t_botanical_ctx *ctx,
				     double rootx,
				     double rooty,
				     double start_angle,
				     double end_angle,
				     double radius,
				     double thickness,
				     double density_scale)
{
  double			cx;
  double			cy;

  cx = rootx + cos(start_angle) * radius;
  cy = rooty - sin(start_angle) * radius;
  draw_circle_arc_trunk(ctx, cx, cy, radius, start_angle, end_angle, thickness);
  draw_arc_vegetation(ctx, cx, cy, radius, start_angle, end_angle,
		      thickness, density_scale);
}

static void		draw_laser_surface_segment(const t_botanical_ctx *ctx,
				 double ax,
				 double ay,
				 double bx,
				 double by,
				 double thickness,
				 double brightness)
{
  double				ex;
  double				ey;
  double				gx;
  double				gy;

  ex = ctx->extrude_x * 0.16;
  ey = ctx->extrude_y * 0.16;
  gx = ctx->extrude_x * 0.42;
  gy = ctx->extrude_y * 0.42;
  draw_botanical_line(ctx, ax, ay, bx, by,
			      thickness * 3.10, 0.06 * brightness);
  draw_botanical_line(ctx, ax, ay, bx, by,
			      thickness * 2.10, 0.12 * brightness);
  draw_botanical_line(ctx, ax + gx, ay + gy, bx + gx, by + gy,
			      thickness * 1.80, 0.07 * brightness);
  draw_botanical_line(ctx, ax + ex, ay + ey, bx + ex, by + ey,
			      thickness * 1.20, 0.22 * brightness);
  draw_botanical_line(ctx, ax, ay, bx, by,
			      thickness * 0.98, 0.42 * brightness);
  draw_botanical_line(ctx, ax, ay, bx, by,
			      thickness * 0.58, 0.98 * brightness);
}

static void		rotate_sphere_point(double x,
			      double y,
			      double z,
			      double tilt_x,
			      double tilt_y,
			      double *rx,
			      double *ry,
			      double *rz)
{
  double			y1;
  double			z1;
  double			x2;
  double			z2;

  y1 = y * cos(tilt_x) - z * sin(tilt_x);
  z1 = y * sin(tilt_x) + z * cos(tilt_x);
  x2 = x * cos(tilt_y) + z1 * sin(tilt_y);
  z2 = -x * sin(tilt_y) + z1 * cos(tilt_y);
  *rx = x2;
  *ry = y1;
  *rz = z2;
}

static void		project_wire_sphere_point(double cx,
				   double cy,
				   double radius,
				   double squash_y,
				   double tilt_x,
				   double tilt_y,
				   double lat,
				   double lon,
				   double *x,
				   double *y,
				   double *depth)
{
  double			px;
  double			py;
  double			pz;
  double			rx;
  double			ry;
  double			rz;
  double			perspective;

  px = cos(lat) * cos(lon);
  py = sin(lat);
  pz = cos(lat) * sin(lon);
  rotate_sphere_point(px, py, pz, tilt_x, tilt_y, &rx, &ry, &rz);
  perspective = 1.0 + rz * 0.10;
  *x = cx + rx * radius * perspective;
  *y = cy + ry * radius * squash_y * perspective;
  *depth = rz;
}

static void		draw_wire_sphere_polyline(const t_botanical_ctx *ctx,
				  double cx,
				  double cy,
				  double radius,
				  double squash_y,
				  double tilt_x,
				  double tilt_y,
				  int is_latitude,
				  double fixed_angle,
				  int sample_count,
				  double thickness,
				  double base_brightness)
{
  double			x1;
  double			y1;
  double			x2;
  double			y2;
  double			d1;
  double			d2;
  double			avg_depth;
  double			brightness;
  double			a1;
  int			i;

  if (is_latitude)
    project_wire_sphere_point(cx, cy, radius, squash_y, tilt_x, tilt_y,
			      fixed_angle, 0.0, &x1, &y1, &d1);
  else
    project_wire_sphere_point(cx, cy, radius, squash_y, tilt_x, tilt_y,
			      -M_PI * 0.5, fixed_angle, &x1, &y1, &d1);

  for (i = 1; i <= sample_count; ++i)
  {
    if (is_latitude)
    {
      a1 = ((double)i / (double)sample_count) * 2.0 * M_PI;
      project_wire_sphere_point(cx, cy, radius, squash_y, tilt_x, tilt_y,
				fixed_angle, a1, &x2, &y2, &d2);
    }
    else
    {
      a1 = -M_PI * 0.5 + ((double)i / (double)sample_count) * M_PI;
      project_wire_sphere_point(cx, cy, radius, squash_y, tilt_x, tilt_y,
				a1, fixed_angle, &x2, &y2, &d2);
    }
    avg_depth = (d1 + d2) * 0.5;
    brightness = base_brightness * (0.10 + 0.90 * (avg_depth * 0.5 + 0.5));
    if (brightness < 0.03)
      brightness = 0.03;
    if (brightness > 1.00)
      brightness = 1.00;
    draw_laser_surface_segment(ctx, x1, y1, x2, y2, thickness, brightness);
    x1 = x2;
    y1 = y2;
    d1 = d2;
  }
}

static void		draw_wire_sphere_polyline_range(const t_botanical_ctx *ctx,
				      double cx,
				      double cy,
				      double radius,
				      double squash_y,
				      double tilt_x,
				      double tilt_y,
				      int is_latitude,
				      double fixed_angle,
				      double start_t,
				      double end_t,
				      int sample_count,
				      double thickness,
				      double base_brightness)
{
  double				x1;
  double				y1;
  double				x2;
  double				y2;
  double				d1;
  double				d2;
  double				avg_depth;
  double				brightness;
  double				a0;
  double				a1;
  double				t;
  int				i;

  if (sample_count < 8)
    sample_count = 8;
  if (start_t > end_t)
  {
    t = start_t;
    start_t = end_t;
    end_t = t;
  }
  if (start_t < 0.0)
    start_t = 0.0;
  if (end_t > 1.0)
    end_t = 1.0;

  if (is_latitude)
  {
    a0 = start_t * 2.0 * M_PI;
    project_wire_sphere_point(cx, cy, radius, squash_y, tilt_x, tilt_y,
			      fixed_angle, a0, &x1, &y1, &d1);
  }
  else
  {
    a0 = -M_PI * 0.5 + start_t * M_PI;
    project_wire_sphere_point(cx, cy, radius, squash_y, tilt_x, tilt_y,
			      a0, fixed_angle, &x1, &y1, &d1);
  }

  for (i = 1; i <= sample_count; ++i)
  {
    t = start_t + (end_t - start_t) * ((double)i / (double)sample_count);
    if (is_latitude)
    {
      a1 = t * 2.0 * M_PI;
      project_wire_sphere_point(cx, cy, radius, squash_y, tilt_x, tilt_y,
				fixed_angle, a1, &x2, &y2, &d2);
    }
    else
    {
      a1 = -M_PI * 0.5 + t * M_PI;
      project_wire_sphere_point(cx, cy, radius, squash_y, tilt_x, tilt_y,
				a1, fixed_angle, &x2, &y2, &d2);
    }
    avg_depth = (d1 + d2) * 0.5;
    brightness = base_brightness * (0.10 + 0.90 * (avg_depth * 0.5 + 0.5));
    if (brightness < 0.02)
      brightness = 0.02;
    if (brightness > 1.00)
      brightness = 1.00;
    draw_laser_surface_segment(ctx, x1, y1, x2, y2, thickness, brightness);
    x1 = x2;
    y1 = y2;
    d1 = d2;
  }
}

static void		draw_sphere_spin_trails(const t_botanical_ctx *ctx,
				   double cx,
				   double cy,
				   double radius,
				   double squash_y,
				   double tilt_x,
				   double tilt_y,
				   double highlight_lon,
				   double highlight_lat,
				   int sample_count,
				   double thickness)
{
  int				i;
  int				count;
  double				direction;
  double				fade;
  double				lon_step;
  double				lat_step;
  double				lon;
  double				lat;

  direction = (ctx->pair_mode == 0 ? -1.0 : 1.0);
  count = 6 + rngi(0, 2);
  lon_step = 0.070 + rngf() * 0.020;
  lat_step = 0.055 + rngf() * 0.020;
  for (i = 0; i < count; ++i)
  {
    fade = 1.0 - ((double)i / (double)(count + 1));
    lon = highlight_lon + direction * (double)(i + 1) * lon_step;
    draw_wire_sphere_polyline_range(ctx, cx, cy, radius, squash_y,
			      tilt_x, tilt_y, 0, lon,
			      0.08, 0.92,
			      sample_count,
			      thickness * (1.06 - 0.08 * i),
			      0.40 * fade);
    draw_wire_sphere_polyline_range(ctx, cx, cy, radius, squash_y,
			      tilt_x, tilt_y, 0,
			      lon + direction * 0.016,
			      0.18, 0.82,
			      (int)(sample_count * 0.72),
			      thickness * (0.66 - 0.05 * i),
			      0.18 * fade);
  }
  count = 3 + rngi(0, 1);
  for (i = 0; i < count; ++i)
  {
    fade = 1.0 - ((double)i / (double)(count + 1));
    lat = highlight_lat + direction * (double)(i + 1) * lat_step;
    draw_wire_sphere_polyline_range(ctx, cx, cy, radius, squash_y,
			      tilt_x, tilt_y, 1, lat,
			      0.10, 0.86,
			      (int)(sample_count * 0.80),
			      thickness * (0.84 - 0.10 * i),
			      0.22 * fade);
  }
}

static void		draw_wire_surface_pair(const t_botanical_ctx *ctx)
{
  double				w;
  double				h;
  double				radius;
  double				cx;
  double				cy;
  double				tilt_x;
  double				tilt_y;
  double				squash_y;
  double				thickness;
  double				intrusion_x;
  double				intrusion_y;
  int			base_lat;
  int			base_lon;
  int			visible_step_lat;
  int			visible_step_lon;
  int			sample_count;
  int			i;
  double			lat;
  double			lon;
  double			brightness;
  double			accent;
  double			line_thickness;
  double			tech_phase;
  double			highlight_lon;

  w = (double)ctx->pix->clipable.buffer.width - 1.0;
  h = (double)ctx->pix->clipable.buffer.height - 1.0;
  tilt_x = -0.86 + (rngf() - 0.5) * 0.18;
  tilt_y = 0.98 + (rngf() - 0.5) * 0.26;
  squash_y = 0.88 + rngf() * 0.08;
  /* Grosses spheres majoritairement hors-champ:
  ** on ne doit en voir qu'un fragment, mais ce fragment doit
  ** occuper franchement le coin de la feuille. */
  /* On fait entrer davantage la sphere dans la page,
  ** mais avec une amplitude plus faible pour limiter les cas
  ** ou les deux grosses formes paraissent trop se repondre. */
  intrusion_x = w * (0.50 + rngf() * 0.03);
  intrusion_y = h * (0.50 + rngf() * 0.03);
  radius = (h < w ? h : w) * (0.92 + rngf() * 0.05);
  if (radius < 1.0)
    radius = 1.0;
  cx = intrusion_x - radius;
  cy = intrusion_y - radius * squash_y;
  thickness = (double)rngi(ctx->style->botanical_min_thickness,
			   ctx->style->botanical_max_thickness) * ctx->ss * 0.58;
  if (thickness < 1.0)
    thickness = 1.0;

  /* Sphere tres grande: on peut densifier fortement le maillage. */
  base_lat = 168 + rngi(0, 36);
  base_lon = 176 + rngi(0, 36);
  visible_step_lat = 2;
  visible_step_lon = 2;
  sample_count = 256 + rngi(0, 64);
  tech_phase = rngf() * 2.0 * M_PI;
  highlight_lon = tech_phase;

  /* Latitudes: dense underlying sphere, sparse visible arcs with laser accents. */
  for (i = visible_step_lat; i < base_lat; i += visible_step_lat)
  {
    lat = -M_PI * 0.5 + ((double)i / (double)base_lat) * M_PI;
    accent = 0.5 + 0.5 * sin(lat * 6.0 + tech_phase);
    brightness = 0.32 + 0.26 * (1.0 - fabs(lat) / (M_PI * 0.5)) + 0.20 * accent;
    line_thickness = thickness * (0.86 + 0.24 * accent);
    draw_wire_sphere_polyline(ctx, cx, cy, radius, squash_y,
			      tilt_x, tilt_y, 1, lat,
			      sample_count, line_thickness, brightness);
  }
  /* Longitudes: more contrasted and more segmented for a techno-laser feeling. */
  for (i = 0; i < base_lon; i += visible_step_lon)
  {
    lon = ((double)i / (double)base_lon) * 2.0 * M_PI;
    accent = 0.5 + 0.5 * sin(lon * 3.0 + tech_phase * 1.3);
    brightness = 0.22 + 0.12 * sin(lon * 2.0) + 0.24 * accent;
    line_thickness = thickness * (0.78 + 0.22 * accent);
    draw_wire_sphere_polyline(ctx, cx, cy, radius, squash_y,
			      tilt_x, tilt_y, 0, lon,
			      sample_count, line_thickness, brightness);
  }

  /* Strong laser guide lines. */
  draw_wire_sphere_polyline(ctx, cx, cy, radius, squash_y,
			    tilt_x, tilt_y, 1, 0.0,
			    sample_count, thickness * 1.36, 0.98);
  lat = -0.34 + (rngf() - 0.5) * 0.12;
  draw_wire_sphere_polyline(ctx, cx, cy, radius, squash_y,
			    tilt_x, tilt_y, 1, lat,
			    sample_count, thickness * 1.18, 0.82);
  draw_wire_sphere_polyline(ctx, cx, cy, radius, squash_y,
			    tilt_x, tilt_y, 0, highlight_lon,
			    sample_count, thickness * 1.26, 0.92);
  draw_sphere_spin_trails(ctx, cx, cy, radius, squash_y,
			   tilt_x, tilt_y, highlight_lon, lat,
			   sample_count, thickness * 0.98);
}

static void		draw_botanical_motif(t_bunny_pixelarray *pix,
					 const t_diploma_style *style)
{
  t_botanical_ctx	ctx;
  double			w;
  double			h;
  double			target_height;
  double			start_angle;
  double			end_angle;
  double			radius;
  double			reference_x;
  double			start_x;
  double			start_y;
  double			thickness;
  double			sub_radius;
  double			sub_end_angle;
  double			sub_radius2;
  double			sub_end_angle2;
  t_botanical_ctx	surface_ctx;

  if (style->botanical_enabled == false)
    return;
  ctx.pix = pix;
  ctx.style = style;
  ctx.color = style->botanical_color;
  ctx.ss = style->super_sampling;
  ctx.pair_mode = rngi(0, 1);
  ctx.extrude_x = (2.0 + rngf() * 1.6) * ctx.ss;
  ctx.extrude_y = -(1.4 + rngf() * 1.2) * ctx.ss;

  w = (double)pix->clipable.buffer.width;
  h = (double)pix->clipable.buffer.height;
  target_height = h * (0.62 + rngf() * 0.05);

  /*
  ** Les arcs sont dessines localement, puis reproduits par symetrie
  ** centrale via draw_botanical_line. On peut donc superposer plusieurs
  ** frondes partageant la meme racine sans perdre la symetrie globale.
  ** On tasse volontairement un peu l'amplitude pour eviter les cas
  ** ou les frondes debordent lateralement selon le tirage.
  */
  start_angle = -1.16 + (rngf() - 0.5) * 0.10;
  end_angle = 0.70 + (rngf() - 0.5) * 0.12;
  radius = target_height / (sin(end_angle) - sin(start_angle));

  /*
  ** reference_x represente la position cible du milieu visuel de l'arc
  ** dans sa demi-page, plus proche du bord exterieur qu'avant pour
  ** garder la fronde bien contenue dans la composition.
  */
  reference_x = (w * 0.5) * (0.22 + rngf() * 0.05);
  start_y = -h * (0.035 + rngf() * 0.035);
  start_x = reference_x
    + (cos((start_angle + end_angle) * 0.5) - cos(start_angle)) * radius;
  thickness = (double)rngi(style->botanical_min_thickness,
			   style->botanical_max_thickness) * ctx.ss;

  sub_radius2 = radius * (0.50 + rngf() * 0.08);
  sub_end_angle2 = end_angle - (0.42 + rngf() * 0.16);
  if (sub_end_angle2 > start_angle + 0.24)
    draw_fern_arc_from_root(&ctx, start_x, start_y, start_angle, sub_end_angle2,
		       sub_radius2, thickness * 0.64, 0.78);

  sub_radius = radius * (0.66 + rngf() * 0.08);
  sub_end_angle = end_angle - (0.22 + rngf() * 0.12);
  if (sub_end_angle > start_angle + 0.24)
    draw_fern_arc_from_root(&ctx, start_x, start_y, start_angle, sub_end_angle,
		       sub_radius, thickness * 0.80, 0.92);

  draw_fern_arc_from_root(&ctx, start_x, start_y, start_angle, end_angle,
		   radius, thickness, 1.12);

  surface_ctx = ctx;
  surface_ctx.pair_mode = 1 - ctx.pair_mode;
  draw_wire_surface_pair(&surface_ctx);
  draw_corner_identifier_objects(pix, style);
}

static void		draw_local_line(const t_botanical_ctx *ctx,
			 double ax,
			 double ay,
			 double bx,
			 double by,
			 double thickness,
			 double opacity)
{
  unsigned int		col;

  col = with_alpha(ctx->color, opacity);
  draw_segment_stroke_alpha(ctx->pix, ax, ay, bx, by, thickness, col, 1.0);
}

static void		draw_local_laser_line(const t_botanical_ctx *ctx,
			       double ax,
			       double ay,
			       double bx,
			       double by,
			       double thickness,
			       double brightness)
{
  draw_local_line(ctx, ax, ay, bx, by, thickness * 2.6, 0.06 * brightness);
  draw_local_line(ctx, ax, ay, bx, by, thickness * 1.8, 0.12 * brightness);
  draw_local_line(ctx, ax, ay, bx, by, thickness * 1.1, 0.24 * brightness);
  draw_local_line(ctx, ax, ay, bx, by, thickness * 0.62, 0.96 * brightness);
}

static t_vec3		vec3(double x, double y, double z)
{
  t_vec3			v;

  v.x = x;
  v.y = y;
  v.z = z;
  return (v);
}

static t_vec3		rotate_vec3(t_vec3 v, double rx, double ry, double rz)
{
  double			c;
  double			s;
  double			t;

  c = cos(rx);
  s = sin(rx);
  t = v.y * c - v.z * s;
  v.z = v.y * s + v.z * c;
  v.y = t;
  c = cos(ry);
  s = sin(ry);
  t = v.x * c + v.z * s;
  v.z = -v.x * s + v.z * c;
  v.x = t;
  c = cos(rz);
  s = sin(rz);
  t = v.x * c - v.y * s;
  v.y = v.x * s + v.y * c;
  v.x = t;
  return (v);
}

static void		project_object_point(double cx, double cy, double scale,
			      double cam_dist, t_vec3 v,
			      double *sx, double *sy)
{
  double			k;

  k = scale / (cam_dist - v.z);
  *sx = cx + v.x * k;
  *sy = cy - v.y * k;
}

static void		draw_object_segment(const t_botanical_ctx *ctx,
			    double cx, double cy, double scale,
			    double cam_dist, double rx, double ry, double rz,
			    t_vec3 a, t_vec3 b, double thickness, double brightness)
{
  double			x1;
  double			y1;
  double			x2;
  double			y2;

  a = rotate_vec3(a, rx, ry, rz);
  b = rotate_vec3(b, rx, ry, rz);
  project_object_point(cx, cy, scale, cam_dist, a, &x1, &y1);
  project_object_point(cx, cy, scale, cam_dist, b, &x2, &y2);
  draw_local_laser_line(ctx, x1, y1, x2, y2, thickness, brightness);
}

static void		draw_object_edges(const t_botanical_ctx *ctx,
			  double cx, double cy, double scale,
			  double cam_dist, double rx, double ry, double rz,
			  const t_vec3 *verts, int vcount,
			  const t_edge3 *edges, int ecount,
			  double thickness, double brightness)
{
  int			i;

  (void)vcount;
  for (i = 0; i < ecount; ++i)
    draw_object_segment(ctx, cx, cy, scale, cam_dist, rx, ry, rz,
				verts[edges[i].a], verts[edges[i].b], thickness, brightness);
}

static void		draw_corner_cube(const t_botanical_ctx *ctx,
			 double cx, double cy, double scale,
			 double rx, double ry, double rz, double thickness)
{
  static const t_vec3 verts[8] = {
    {-1.0, -1.0, -1.0}, {1.0, -1.0, -1.0},
    {1.0, 1.0, -1.0}, {-1.0, 1.0, -1.0},
    {-1.0, -1.0, 1.0}, {1.0, -1.0, 1.0},
    {1.0, 1.0, 1.0}, {-1.0, 1.0, 1.0}
  };
  static const t_edge3 edges[12] = {
    {0,1},{1,2},{2,3},{3,0},
    {4,5},{5,6},{6,7},{7,4},
    {0,4},{1,5},{2,6},{3,7}
  };

  draw_object_edges(ctx, cx, cy, scale, 5.2, rx, ry, rz,
			verts, 8, edges, 12, thickness, 1.0);
}

static void		draw_corner_cone(const t_botanical_ctx *ctx,
			 double cx, double cy, double scale,
			 double rx, double ry, double rz, double thickness)
{
  int			i;
  int			steps;
  double			a0;
  double			a1;
  t_vec3			apex;
  t_vec3			p0;
  t_vec3			p1;

  steps = 14;
  apex = vec3(0.0, 1.25, 0.0);
  for (i = 0; i < steps; ++i)
  {
    a0 = ((double)i / (double)steps) * 2.0 * M_PI;
    a1 = ((double)(i + 1) / (double)steps) * 2.0 * M_PI;
    p0 = vec3(cos(a0), -1.15, sin(a0));
    p1 = vec3(cos(a1), -1.15, sin(a1));
    draw_object_segment(ctx, cx, cy, scale, 5.0, rx, ry, rz, p0, p1, thickness, 0.90);
    if (i % 2 == 0)
      draw_object_segment(ctx, cx, cy, scale, 5.0, rx, ry, rz, apex, p0, thickness, 1.0);
  }
}

static void		draw_corner_cylinder(const t_botanical_ctx *ctx,
			 double cx, double cy, double scale,
			 double rx, double ry, double rz, double thickness)
{
  int			i;
  int			steps;
  double			a0;
  double			a1;
  t_vec3			t0;
  t_vec3			t1;
  t_vec3			b0;
  t_vec3			b1;

  steps = 14;
  for (i = 0; i < steps; ++i)
  {
    a0 = ((double)i / (double)steps) * 2.0 * M_PI;
    a1 = ((double)(i + 1) / (double)steps) * 2.0 * M_PI;
    t0 = vec3(cos(a0), 1.10, sin(a0));
    t1 = vec3(cos(a1), 1.10, sin(a1));
    b0 = vec3(cos(a0), -1.10, sin(a0));
    b1 = vec3(cos(a1), -1.10, sin(a1));
    draw_object_segment(ctx, cx, cy, scale, 5.2, rx, ry, rz, t0, t1, thickness, 0.92);
    draw_object_segment(ctx, cx, cy, scale, 5.2, rx, ry, rz, b0, b1, thickness, 0.92);
    if (i % 3 == 0)
      draw_object_segment(ctx, cx, cy, scale, 5.2, rx, ry, rz, t0, b0, thickness, 1.0);
  }
}

static void		draw_corner_hourglass(const t_botanical_ctx *ctx,
			  double cx, double cy, double scale,
			  double rx, double ry, double rz, double thickness)
{
  int			i;
  int			steps;
  double			a0;
  double			a1;
  t_vec3			t0;
  t_vec3			t1;
  t_vec3			b0;
  t_vec3			b1;
  t_vec3			mid;

  steps = 14;
  mid = vec3(0.0, 0.0, 0.0);
  for (i = 0; i < steps; ++i)
  {
    a0 = ((double)i / (double)steps) * 2.0 * M_PI;
    a1 = ((double)(i + 1) / (double)steps) * 2.0 * M_PI;
    t0 = vec3(0.92 * cos(a0), 1.25, 0.92 * sin(a0));
    t1 = vec3(0.92 * cos(a1), 1.25, 0.92 * sin(a1));
    b0 = vec3(0.92 * cos(a0), -1.25, 0.92 * sin(a0));
    b1 = vec3(0.92 * cos(a1), -1.25, 0.92 * sin(a1));
    draw_object_segment(ctx, cx, cy, scale, 5.0, rx, ry, rz, t0, t1, thickness, 0.90);
    draw_object_segment(ctx, cx, cy, scale, 5.0, rx, ry, rz, b0, b1, thickness, 0.90);
    if (i % 2 == 0)
    {
      draw_object_segment(ctx, cx, cy, scale, 5.0, rx, ry, rz, t0, mid, thickness, 1.0);
      draw_object_segment(ctx, cx, cy, scale, 5.0, rx, ry, rz, b0, mid, thickness, 1.0);
    }
  }
}

static void		draw_corner_ovoid(const t_botanical_ctx *ctx,
			  double cx, double cy, double scale,
			  double rx, double ry, double rz, double thickness)
{
  int			lat_i;
  int			lon_i;
  int			lat_steps;
  int			lon_steps;
  double			lat0;
  double			lat1;
  double			lon0;
  double			lon1;
  t_vec3			p0;
  t_vec3			p1;

  lat_steps = 5;
  lon_steps = 9;
  for (lat_i = 0; lat_i <= lat_steps; ++lat_i)
  {
    lat0 = -M_PI * 0.5 + M_PI * ((double)lat_i / (double)lat_steps);
    for (lon_i = 0; lon_i < lon_steps; ++lon_i)
    {
      lon0 = 2.0 * M_PI * ((double)lon_i / (double)lon_steps);
      lon1 = 2.0 * M_PI * ((double)(lon_i + 1) / (double)lon_steps);
      p0 = vec3(cos(lat0) * cos(lon0), sin(lat0) * 1.25, cos(lat0) * sin(lon0));
      p1 = vec3(cos(lat0) * cos(lon1), sin(lat0) * 1.25, cos(lat0) * sin(lon1));
      draw_object_segment(ctx, cx, cy, scale, 5.4, rx, ry, rz, p0, p1, thickness, 0.84);
    }
  }
  for (lon_i = 0; lon_i < lon_steps; ++lon_i)
  {
    lon0 = 2.0 * M_PI * ((double)lon_i / (double)lon_steps);
    for (lat_i = 0; lat_i < lat_steps; ++lat_i)
    {
      lat0 = -M_PI * 0.5 + M_PI * ((double)lat_i / (double)lat_steps);
      lat1 = -M_PI * 0.5 + M_PI * ((double)(lat_i + 1) / (double)lat_steps);
      p0 = vec3(cos(lat0) * cos(lon0), sin(lat0) * 1.25, cos(lat0) * sin(lon0));
      p1 = vec3(cos(lat1) * cos(lon0), sin(lat1) * 1.25, cos(lat1) * sin(lon0));
      draw_object_segment(ctx, cx, cy, scale, 5.4, rx, ry, rz, p0, p1, thickness, 0.90);
    }
  }
}

static void		draw_corner_torus(const t_botanical_ctx *ctx,
			  double cx, double cy, double scale,
			  double rx, double ry, double rz, double thickness)
{
  int			i;
  int			j;
  int			u_steps;
  int			v_steps;
  double			u0;
  double			u1;
  double			v0;
  double			v1;
  double			R;
  double			r;
  t_vec3			p0;
  t_vec3			p1;

  u_steps = 12;
  v_steps = 7;
  R = 1.18;
  r = 0.42;
  for (i = 0; i < u_steps; ++i)
    for (j = 0; j < v_steps; ++j)
    {
      u0 = 2.0 * M_PI * ((double)i / (double)u_steps);
      u1 = 2.0 * M_PI * ((double)(i + 1) / (double)u_steps);
      v0 = 2.0 * M_PI * ((double)j / (double)v_steps);
      v1 = 2.0 * M_PI * ((double)(j + 1) / (double)v_steps);
      p0 = vec3((R + r * cos(v0)) * cos(u0), r * sin(v0),
		(R + r * cos(v0)) * sin(u0));
      p1 = vec3((R + r * cos(v0)) * cos(u1), r * sin(v0),
		(R + r * cos(v0)) * sin(u1));
      draw_object_segment(ctx, cx, cy, scale, 5.8, rx, ry, rz, p0, p1, thickness, 0.90);
      if (i % 3 == 0)
      {
	p1 = vec3((R + r * cos(v1)) * cos(u0), r * sin(v1),
		  (R + r * cos(v1)) * sin(u0));
	draw_object_segment(ctx, cx, cy, scale, 5.8, rx, ry, rz, p0, p1, thickness, 0.80);
      }
    }
}

static void		draw_corner_object_kind(const t_botanical_ctx *ctx,
			       t_corner_object_kind kind,
			       double cx, double cy, double scale,
			       double rx, double ry, double rz,
			       double thickness)
{
  if (kind == OBJ_OVOID)
    draw_corner_ovoid(ctx, cx, cy, scale, rx, ry, rz, thickness);
  else if (kind == OBJ_CUBE)
    draw_corner_cube(ctx, cx, cy, scale, rx, ry, rz, thickness);
  else if (kind == OBJ_CONE)
    draw_corner_cone(ctx, cx, cy, scale, rx, ry, rz, thickness);
  else if (kind == OBJ_HOURGLASS)
    draw_corner_hourglass(ctx, cx, cy, scale, rx, ry, rz, thickness);
  else if (kind == OBJ_CYLINDER)
    draw_corner_cylinder(ctx, cx, cy, scale, rx, ry, rz, thickness);
  else
    draw_corner_torus(ctx, cx, cy, scale, rx, ry, rz, thickness);
}

static void		draw_corner_identifier_objects(t_bunny_pixelarray *pix,
			     const t_diploma_style *style)
{
  t_botanical_ctx	ctx;
  t_corner_object_kind	kinds[6];
  int			i;
  int			j;
  int			tmp;
  double			h;
  double			base_x;
  double			base_y;
  double			thickness;
  double			x[5];
  double			y[5];
  double			sz[5];
  double			dx;
  double			dy;
  double			dist2;
  double			slot_x[5];
  double			slot_y[5];
  int			attempt;

  ctx.pix = pix;
  ctx.style = style;
  ctx.color = style->botanical_color;
  ctx.pair_mode = 0;
  ctx.ss = style->super_sampling < 1 ? 1 : style->super_sampling;
  ctx.extrude_x = 0.0;
  ctx.extrude_y = 0.0;
  h = (double)pix->clipable.buffer.height;
  base_x = (style->border_margin + style->border_max_thickness
	    + style->border_line_spacing * style->border_max_lines + 8) * ctx.ss;
  base_y = h - (style->border_margin + style->border_max_thickness
	     + style->border_line_spacing * style->border_max_lines + 8) * ctx.ss;
  thickness = (0.80 + rngf() * 0.40) * ctx.ss;
  /* Composition plus lisible, type constellation d'objets,
  ** tout en restant clairement ancree dans le coin inferieur gauche. */
  slot_x[0] = 28.0; slot_y[0] = 24.0;
  slot_x[1] = 98.0; slot_y[1] = 58.0;
  slot_x[2] = 48.0; slot_y[2] = 126.0;
  slot_x[3] = 148.0; slot_y[3] = 156.0;
  slot_x[4] = 96.0; slot_y[4] = 222.0;
  kinds[0] = OBJ_OVOID;
  kinds[1] = OBJ_CUBE;
  kinds[2] = OBJ_CONE;
  kinds[3] = OBJ_HOURGLASS;
  kinds[4] = OBJ_CYLINDER;
  kinds[5] = OBJ_TORUS;
  for (i = 5; i > 0; --i)
  {
    j = rngi(0, i);
    tmp = kinds[i];
    kinds[i] = kinds[j];
    kinds[j] = tmp;
  }
  for (i = 0; i < 5; ++i)
  {
    /* Gros objets, mais places sur des emplacements guides pour rester
    ** lisibles et faire une sorte de constellation reconnaissable. */
    sz[i] = (108.0 + rngf() * 84.0) * ctx.ss;
    x[i] = base_x + (slot_x[i] + (rngf() - 0.5) * 18.0) * ctx.ss;
    y[i] = base_y - (slot_y[i] + (rngf() - 0.5) * 18.0) * ctx.ss;
    for (attempt = 0; attempt < 120; ++attempt)
    {
      int ok = 1;
      for (j = 0; j < i; ++j)
      {
	dx = x[i] - x[j];
	dy = y[i] - y[j];
	dist2 = dx * dx + dy * dy;
	if (dist2 < (((sz[i] + sz[j]) * 0.72 + 14.0 * ctx.ss)
	    * ((sz[i] + sz[j]) * 0.72 + 14.0 * ctx.ss)))
	{
	  ok = 0;
	  x[i] = base_x + (slot_x[i] + (rngf() - 0.5) * 24.0) * ctx.ss;
	  y[i] = base_y - (slot_y[i] + (rngf() - 0.5) * 24.0) * ctx.ss;
	  break;
	}
      }
      if (ok)
	break;
    }
  }
  for (i = 0; i < 5; ++i)
  {
    draw_corner_object_kind(&ctx, kinds[i], x[i], y[i], sz[i],
			    rngf() * 0.95 - 0.40,
			    rngf() * 1.25 - 0.62,
			    rngf() * 2.0 * M_PI,
			    thickness * (0.92 + rngf() * 0.18));
  }
}

static double	upper_right_wave_sine(double phase)
{
  return (sin(phase));
}

static double	upper_right_wave_saw(double phase)
{
  double	cycles;

  cycles = phase / (2.0 * M_PI);
  cycles -= floor(cycles);
  return (2.0 * cycles - 1.0);
}

static double	upper_right_wave_triangle(double phase)
{
  double	cycles;

  cycles = phase / (2.0 * M_PI);
  cycles -= floor(cycles);
  return (1.0 - 4.0 * fabs(cycles - 0.5));
}

static double	upper_right_wave_square(double phase)
{
  return (sin(phase) >= 0.0 ? 1.0 : -1.0);
}

static void		draw_colored_line(t_bunny_pixelarray *pix,
		      unsigned int color,
		      double ax,
		      double ay,
		      double bx,
		      double by,
		      double thickness,
		      double opacity)
{
  draw_segment_stroke_alpha(pix, ax, ay, bx, by, thickness,
			   with_alpha(color, opacity), 1.0);
}

static void		draw_colored_laser_line(t_bunny_pixelarray *pix,
			    unsigned int color,
			    double ax,
			    double ay,
			    double bx,
			    double by,
			    double thickness,
			    double brightness,
			    double opacity)
{
  draw_colored_line(pix, color, ax, ay, bx, by, thickness * 2.8,
		    opacity * 0.05 * brightness);
  draw_colored_line(pix, color, ax, ay, bx, by, thickness * 1.9,
		    opacity * 0.11 * brightness);
  draw_colored_line(pix, color, ax, ay, bx, by, thickness * 1.1,
		    opacity * 0.22 * brightness);
  draw_colored_line(pix, color, ax, ay, bx, by, thickness * 0.60,
		    opacity * 0.95 * brightness);
}

static void		draw_guilloche_curve_family(t_bunny_pixelarray *pix,
			       double cx,
			       double cy,
			       double rx,
			       double ry,
			       int lines,
			       int turns,
			       int samples_per_turn,
			       double amp_a,
			       double amp_b,
			       double freq_a,
			       double freq_b,
			       double twist_amp,
			       double twist_freq,
			       double line_radius_step,
			       double line_phase_step,
			       double global_phase,
			       unsigned int color_a,
			       unsigned int color_b,
			       double opacity)
{
  int		line;
  int		sample;
  int		total_samples;

  total_samples = turns * samples_per_turn;
  if (total_samples < 2)
    return;
  for (line = 0; line < lines; ++line)
  {
    double	radius_mul;
    double	phase;
    double	ax;
    double	ay;
    int		has_old;
    double	base_thickness;
    double	line_mix;

    radius_mul = 1.0 + ((double)line - (double)(lines - 1) * 0.5)
      * line_radius_step;
    phase = global_phase + (double)line * line_phase_step;
    has_old = 0;
    base_thickness = 0.58 + 0.10 * (double)(line % 3);
    line_mix = (lines <= 1) ? 0.5 : (double)line / (double)(lines - 1);
    for (sample = 0; sample <= total_samples; ++sample)
    {
      double	u;
      double	t;
      double	wobble;
      double	twist;
      double	bx;
      double	by;
      double	color_t;
      unsigned int color;
      double	brightness;

      u = (double)sample / (double)total_samples;
      t = u * 2.0 * M_PI * (double)turns;
      wobble = 1.0
	+ amp_a * sin(freq_a * t + phase)
	+ amp_b * sin(freq_b * t - phase * 0.73);
      twist = twist_amp * sin(twist_freq * t + phase);
      bx = cx + rx * radius_mul * wobble * cos(t + twist);
      by = cy + ry * radius_mul * wobble * sin(t + twist);
      color_t = 0.5 + 0.5 * sin(t * 0.27 + phase * 0.91 + line_mix * M_PI);
      color = lerp_color(color_a, color_b, color_t);
      brightness = 0.78 + 0.34 * (0.5 + 0.5 * sin(t * 0.17 + phase));
      if (has_old)
	draw_colored_laser_line(pix, color, ax, ay, bx, by, base_thickness,
			       brightness, opacity);
      ax = bx;
      ay = by;
      has_old = 1;
    }
  }
}

static void		draw_concentric_guilloche_family(t_bunny_pixelarray *pix,
			      double cx,
			      double cy,
			      int rings,
			      int samples_per_ring,
			      double first_radius,
			      double radius_step,
			      double wave_amp,
			      double wave_freq,
			      double ring_phase_step,
			      double spiral_amp,
			      double spiral_freq,
			      double color_freq,
			      double color_ring_shift,
			      unsigned int color_a,
			      unsigned int color_b,
			      double opacity)
{
  int		ring;

  for (ring = 0; ring < rings; ++ring)
  {
    double	ax;
    double	ay;
    int		has_old;
    double	base_radius;
    double	ring_phase;
    int		sample;

    ax = 0.0;
    ay = 0.0;
    has_old = 0;
    base_radius = first_radius + (double)ring * radius_step;
    ring_phase = (double)ring * ring_phase_step;
    for (sample = 0; sample <= samples_per_ring; ++sample)
    {
      double	u;
      double	t;
      double	spiral;
      double	radius;
      double	bx;
      double	by;
      double	color_t;
      unsigned int color;

      u = (double)sample / (double)samples_per_ring;
      t = u * 2.0 * M_PI;
      spiral = spiral_amp * sin(spiral_freq * t + ring_phase * 0.37) * u;
      radius = base_radius
	+ wave_amp * sin(wave_freq * t + ring_phase)
	+ spiral;
      bx = cx + radius * cos(t);
      by = cy + radius * sin(t);
      color_t = 0.5 + 0.5 * sin(color_freq * t + (double)ring * color_ring_shift);
      color = lerp_color(color_a, color_b, color_t);
      if (has_old)
	draw_colored_laser_line(pix, color, ax, ay, bx, by, 0.45, 0.82, opacity);
      ax = bx;
      ay = by;
      has_old = 1;
    }
  }
}

static void		draw_rosette_guilloche_family(t_bunny_pixelarray *pix,
			   double cx,
			   double cy,
			   double base_radius,
			   int lines,
			   int turns,
			   int samples_per_turn,
			   double petal_amp_a,
			   double petal_amp_b,
			   double petal_freq_a,
			   double petal_freq_b,
			   double twist_amp,
			   double phase_step,
			   double radius_step,
			   double global_phase,
			   unsigned int color_a,
			   unsigned int color_b,
			   double opacity)
{
  int		line;
  int		sample;
  int		total_samples;

  total_samples = turns * samples_per_turn;
  if (total_samples < 2)
    return;
  for (line = 0; line < lines; ++line)
  {
    double	phase;
    double	radius_mul;
    double	ax;
    double	ay;
    int		has_old;
    double	thickness;
    double	line_mix;

    phase = global_phase + (double)line * phase_step;
    radius_mul = 1.0 + ((double)line - (double)(lines - 1) * 0.5)
      * radius_step;
    has_old = 0;
    thickness = 0.64 + 0.12 * (double)(line % 3);
    line_mix = (lines <= 1) ? 0.5 : (double)line / (double)(lines - 1);
    for (sample = 0; sample <= total_samples; ++sample)
    {
      double	u;
      double	t;
      double	radius;
      double	angle;
      double	bx;
      double	by;
      double	color_t;
      unsigned int color;
      double	brightness;

      u = (double)sample / (double)total_samples;
      t = u * 2.0 * M_PI * (double)turns;
      radius = base_radius * radius_mul
	* (1.0
	   + (petal_amp_a * 0.25) * cos(petal_freq_a * t + phase)
	   + (petal_amp_b * 0.25) * cos(petal_freq_b * t - phase * 0.73)
	   + (0.030 * 0.25) * cos((petal_freq_a + petal_freq_b) * t + phase * 0.37));
      angle = t + twist_amp * sin(2.0 * t + phase * 0.6)
	+ 0.018 * sin(4.0 * t - phase * 0.45);
      bx = cx + radius * cos(angle);
      by = cy + radius * sin(angle);
      color_t = 0.5 + 0.5 * sin(t * 0.12 + phase + line_mix * M_PI * 0.6);
      color = lerp_color(color_a, color_b, color_t);
      brightness = 0.98 + 0.38 * (0.5 + 0.5 * sin(t * 0.10 + phase * 1.2));
      if (has_old)
	draw_colored_laser_line(pix, color, ax, ay, bx, by, thickness,
			       brightness, opacity);
      ax = bx;
      ay = by;
      has_old = 1;
    }
  }
}

static void		draw_guilloche_background(t_bunny_pixelarray *pix,
			      const t_diploma_style *style)
{
  double	w;
  double	h;
  double	cx;
  double	cy;
  double	base_radius;
  unsigned int	base_green;
  unsigned int	soft_green;
  unsigned int	bright_green;
  unsigned int	deep_green;
  double	phase;
  int		freq_a;
  int		freq_b;
  int		freq_c;
  int		freq_d;

  w = (double)pix->clipable.buffer.width;
  h = (double)pix->clipable.buffer.height;
  cx = w * 0.5;
  cy = h * 0.53;
  base_radius = (w < h ? w : h) * (0.225 + rngf() * 0.020);
  phase = rngf() * 2.0 * M_PI;
  freq_a = 6 + 2 * rngi(0, 2);
  freq_b = 10 + 2 * rngi(0, 2);
  freq_c = 4 + 2 * rngi(0, 2);
  freq_d = 8 + 2 * rngi(0, 2);
  base_green = lerp_color(style->background_color, style->border_color, 0.56);
  soft_green = lerp_color(style->background_color, style->botanical_color, 0.68);
  bright_green = lerp_color(style->border_color, style->botanical_color, 0.84);
  deep_green = lerp_color(base_green, style->background_color, 0.16);

  draw_rosette_guilloche_family(pix, cx, cy,
			base_radius,
			16 + rngi(0, 6), 8 + rngi(0, 2), 280 + rngi(0, 60),
			0.20 + rngf() * 0.04,
			0.10 + rngf() * 0.03,
			(double)freq_a,
			(double)freq_b,
			0.085 + rngf() * 0.025,
			0.14 + rngf() * 0.08,
			0.012 + rngf() * 0.004,
			phase,
			soft_green, bright_green,
			0.28 + rngf() * 0.05);

  draw_rosette_guilloche_family(pix, cx, cy,
			base_radius * (0.76 + rngf() * 0.06),
			10 + rngi(0, 4), 6 + rngi(0, 2), 250 + rngi(0, 50),
			0.14 + rngf() * 0.04,
			0.07 + rngf() * 0.02,
			(double)freq_c,
			(double)freq_d,
			0.070 + rngf() * 0.020,
			0.22 + rngf() * 0.10,
			0.018 + rngf() * 0.005,
			phase * 1.17 + 0.6,
			base_green, bright_green,
			0.22 + rngf() * 0.04);

  draw_concentric_guilloche_family(pix, cx, cy,
			22 + rngi(0, 6), 360 + rngi(0, 70),
			base_radius * (0.095 + rngf() * 0.030),
			base_radius * (0.030 + rngf() * 0.008),
			base_radius * (0.015 + rngf() * 0.005),
			6.5 + rngf() * 3.5,
			0.24 + rngf() * 0.12,
			base_radius * (0.009 + rngf() * 0.003),
			2.4 + rngf() * 1.0,
			2.2 + rngf() * 0.8,
			0.16 + rngf() * 0.08,
			deep_green, bright_green,
			0.16 + rngf() * 0.04);

  draw_guilloche_curve_family(pix, cx, cy,
			base_radius * (1.00 + rngf() * 0.03),
			base_radius * (0.98 + rngf() * 0.03),
			5 + rngi(0, 2), 3 + rngi(0, 1), 220 + rngi(0, 40),
			0.014 + rngf() * 0.006, 0.010 + rngf() * 0.005,
			10.0 + rngf() * 3.0, 16.0 + rngf() * 3.0,
			0.035 + rngf() * 0.012, 0.9 + rngf() * 0.5,
			0.024 + rngf() * 0.007, 0.70 + rngf() * 0.14,
			phase * 0.85 + 1.0,
			deep_green, soft_green,
			0.10 + rngf() * 0.03);
}



static double	upper_right_wave_sample(int type, double phase)
{
  if (type == 0)
    return (upper_right_wave_sine(phase));
  if (type == 1)
    return (upper_right_wave_saw(phase));
  if (type == 2)
    return (upper_right_wave_triangle(phase));
  return (upper_right_wave_square(phase));
}

static void		draw_upper_right_waves(t_bunny_pixelarray *pix,
			      const t_diploma_style *style)
{
  t_botanical_ctx	ctx;
  double			w;
  double			h;
  double			intrusion;
  double			x_start;
  double			x_end;
  double			base_y;
  double			max_amp;
  double			top_limit;
  int			wave_count;
  int			i;
  int			step;
  int			steps;
  double			ax;
  double			ay;
  double			bx;
  double			by;
  double			t;
  double			local_t;
  double			envelope;
  double			amp;
  double			freq;
  double			phase;
  double			thickness;
  double			brightness;
  double			shape;
  double			v1;
  double			v2;
  double			v3;
  int			wave_type;
  int			type_cycle[3];
  int			swap;
  int			tmp;

  ctx.pix = pix;
  ctx.style = style;
  ctx.color = style->botanical_color;
  ctx.pair_mode = 0;
  ctx.ss = style->super_sampling < 1 ? 1 : style->super_sampling;
  ctx.extrude_x = 0.0;
  ctx.extrude_y = 0.0;
  w = (double)pix->clipable.buffer.width;
  h = (double)pix->clipable.buffer.height;

  /* Vue oscilloscope : les ondes partagent exactement la meme ligne de
  ** base, au meme endroit. Elles se superposent donc sur un meme axe
  ** horizontal, et seule leur amplitude occupe l'espace vertical.
  ** On s'assure aussi que le sommet des oscillations reste bien sous la
  ** partie horizontale du coin superieur droit du cadre. */
  intrusion = w * (0.20 + rngf() * 0.13);
  x_end = w - intrusion;
  x_start = w + (26.0 + rngf() * 58.0) * ctx.ss;
  wave_count = 2 + rngi(0, 2);
  top_limit = (style->border_margin
	       + style->border_max_thickness
	       + style->border_line_spacing * style->border_max_lines
	       + 18) * ctx.ss;
  base_y = top_limit + h * (0.095 + rngf() * 0.040);
  max_amp = h * (0.040 + rngf() * 0.025);
  if (max_amp > base_y - top_limit - 8.0 * ctx.ss)
    max_amp = base_y - top_limit - 8.0 * ctx.ss;
  if (max_amp < 10.0 * ctx.ss)
    max_amp = 10.0 * ctx.ss;
  steps = 160;

  type_cycle[0] = 0;
  type_cycle[1] = 1;
  type_cycle[2] = 2;
  for (i = 2; i > 0; --i)
  {
    swap = rngi(0, i);
    tmp = type_cycle[i];
    type_cycle[i] = type_cycle[swap];
    type_cycle[swap] = tmp;
  }

  for (i = 0; i < wave_count; ++i)
  {
    freq = 1.0 + rngf() * 2.8;
    phase = rngf() * 2.0 * M_PI;
    wave_type = type_cycle[i % 3];
    if (wave_type == 0)
    {
      /* Sinusoide: plus fine, amplitude plutot moderee. */
      amp = max_amp * (0.32 + rngf() * 0.24);
      thickness = (0.65 + rngf() * 0.45) * ctx.ss;
    }
    else if (wave_type == 1)
    {
      /* Dent de scie: plus agressive, amplitude plus marquee. */
      amp = max_amp * (0.52 + rngf() * 0.24);
      thickness = (0.95 + rngf() * 0.60) * ctx.ss;
    }
    else
    {
      /* Triangle: grande amplitude et trait plus epais. */
      amp = max_amp * (0.72 + rngf() * 0.24);
      thickness = (1.20 + rngf() * 0.70) * ctx.ss;
    }
    brightness = 0.80 + rngf() * 0.45;

    t = 0.0;
    local_t = 1.0 - t;
    envelope = 0.97 - 0.12 * t + 0.06 * sin(t * M_PI);
    v1 = upper_right_wave_sample(wave_type, local_t * freq * 2.0 * M_PI + phase);
    v2 = upper_right_wave_sample(wave_type, local_t * freq * 2.0 * M_PI * 0.5 + phase * 0.41);
    v3 = sin(local_t * freq * 2.0 * M_PI * 2.1 + phase * 1.17);
    shape = amp * (v1 + 0.12 * v2 + 0.04 * v3) * envelope;
    ax = x_start + (x_end - x_start) * t;
    ay = base_y + shape;

    for (step = 1; step <= steps; ++step)
    {
      t = (double)step / (double)steps;
      local_t = 1.0 - t;
      envelope = 0.97 - 0.12 * t + 0.06 * sin(t * M_PI);
      v1 = upper_right_wave_sample(wave_type, local_t * freq * 2.0 * M_PI + phase);
      v2 = upper_right_wave_sample(wave_type, local_t * freq * 2.0 * M_PI * 0.5 + phase * 0.41);
      v3 = sin(local_t * freq * 2.0 * M_PI * 2.1 + phase * 1.17);
      shape = amp * (v1 + 0.12 * v2 + 0.04 * v3) * envelope;
      bx = x_start + (x_end - x_start) * t;
      by = base_y + shape;
      draw_local_laser_line(&ctx, ax, ay, bx, by, thickness, brightness);
      ax = bx;
      ay = by;
    }
  }
}

void			render_diploma_background(t_bunny_pixelarray *pix,
				  const t_diploma_style *style,
				  const char *codename,
				  const char *secret)
{
  fill_pixelarray(pix, style->background_color);
  rng_init(tagged_seed(codename, "guilloche", secret));
  draw_guilloche_background(pix, style);
  rng_init(tagged_seed(codename, "waves", secret));
  draw_upper_right_waves(pix, style);
  rng_init(tagged_seed(codename, "botanical", secret));
  draw_botanical_motif(pix, style);
  rng_init(tagged_seed(codename, "border", secret));
  draw_procedural_border(pix, style);
}

