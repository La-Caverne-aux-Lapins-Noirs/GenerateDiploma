// Jason Brillante "Damdoshi"
// Hanged Bunny Studio 2014-2026
// EFRITS SAS 2022-2026
// Pentacle Technologie 2008-2026
//
// GenDiplome

#include		<stdio.h>
#include		<stdlib.h>
#include		<string.h>
#include		<math.h>
#include		"program.h"

static unsigned int	pixel_at(const t_bunny_pixelarray *pix,
				 int x,
				 int y)
{
  const unsigned int	*pixels;

  if (x < 0 || y < 0 || x >= pix->clipable.buffer.width
      || y >= pix->clipable.buffer.height)
    return (0);
  pixels = pix->pixels;
  return (pixels[y * pix->clipable.buffer.width + x]);
}

static void		put_pixel_alpha(t_bunny_pixelarray *pix,
				int x,
				int y,
				unsigned int color,
				double alpha)
{
  t_bunny_position	pos;

  pos.x = x;
  pos.y = y;
  set_pixel(pix, pos, color, alpha);
}

static unsigned int	make_color(unsigned char red,
			       unsigned char green,
			       unsigned char blue,
			       unsigned char alpha)
{
  t_bunny_color		c;

  c.full = 0;
  c.argb[RED_CMP] = red;
  c.argb[GREEN_CMP] = green;
  c.argb[BLUE_CMP] = blue;
  c.argb[ALPHA_CMP] = alpha;
  return (c.full);
}

static void		blit_scaled_pixelarray(t_bunny_pixelarray *dst,
				       t_bunny_pixelarray *src,
				       double dx,
				       double dy,
				       double dw,
				       double dh,
				       double opacity)
{
  int			x;
  int			y;
  int			sx;
  int			sy;
  int			iw;
  int			ih;

  if (dst == NULL || src == NULL || dw <= 0.0 || dh <= 0.0)
    return;
  iw = (int)ceil(dw);
  ih = (int)ceil(dh);
  for (y = 0; y < ih; ++y)
    for (x = 0; x < iw; ++x)
    {
      sx = (int)(((double)x + 0.5) * src->clipable.buffer.width / dw);
      sy = (int)(((double)y + 0.5) * src->clipable.buffer.height / dh);
      put_pixel_alpha(dst, (int)dx + x, (int)dy + y,
		      pixel_at(src, sx, sy), opacity);
    }
}

static void		render_logo(t_bunny_pixelarray *pix,
			    const t_diploma_style *style)
{
  t_bunny_pixelarray	*logo;
  double		max_width;
  double		scale;
  double		dw;
  double		dh;
  double		dx;
  double		dy;

  if (style->logo_path == NULL || style->logo_path[0] == '\0')
    return;
  logo = bunny_load_pixelarray(style->logo_path);
  if (logo == NULL)
  {
    fprintf(stderr, "Cannot load diploma logo: %s\n", style->logo_path);
    return;
  }
  max_width = pix->clipable.buffer.width * 0.30;
  scale = max_width / (double)logo->clipable.buffer.width;
  dw = logo->clipable.buffer.width * scale;
  dh = logo->clipable.buffer.height * scale;
  dx = ((double)pix->clipable.buffer.width - dw) * 0.5;
  dy = (double)style->border_margin * (double)style->super_sampling * 0.28;
  blit_scaled_pixelarray(pix, logo, dx, dy, dw, dh, 1.0);
  bunny_delete_clipable(&logo->clipable);
}

static void		render_image_fit(t_bunny_pixelarray *pix,
			 const char *path,
			 double box_x,
			 double box_y,
			 double box_w,
			 double box_h)
{
  t_bunny_pixelarray	*img;
  double		scale_w;
  double		scale_h;
  double		scale;
  double		dw;
  double		dh;

  if (path == NULL || path[0] == '\0' || box_w <= 0.0 || box_h <= 0.0)
    return;
  img = bunny_load_pixelarray(path);
  if (img == NULL)
  {
    fprintf(stderr, "Cannot load diploma image: %s\n", path);
    return;
  }
  scale_w = box_w / (double)img->clipable.buffer.width;
  scale_h = box_h / (double)img->clipable.buffer.height;
  scale = scale_w < scale_h ? scale_w : scale_h;
  dw = img->clipable.buffer.width * scale;
  dh = img->clipable.buffer.height * scale;
  blit_scaled_pixelarray(pix, img, box_x + (box_w - dw) * 0.5,
			 box_y + (box_h - dh) * 0.5, dw, dh, 1.0);
  bunny_delete_clipable(&img->clipable);
}

static int		font_pixel_is_background(unsigned int color)
{
  t_bunny_color	c;

  c.full = color;
  return (c.argb[ALPHA_CMP] == 0);
}

static void		recolor_font_foreground(t_bunny_pixelarray *pix,
			      unsigned int color)
{
  unsigned int		*px;
  int			count;
  int			i;
  t_bunny_color		src;
  t_bunny_color		dst;

  if (pix == NULL)
    return;
  px = pix->pixels;
  count = pix->clipable.buffer.width * pix->clipable.buffer.height;
  dst.full = color;
  for (i = 0; i < count; ++i)
    {
      if (font_pixel_is_background(px[i]))
	continue;
      src.full = px[i];
      dst.argb[ALPHA_CMP] = src.argb[ALPHA_CMP];
      px[i] = dst.full;
    }
}

static void		blit_font_clipable_preserve_background(t_bunny_pixelarray *dst,
				       const t_bunny_clipable *src,
				       const t_bunny_position *pos)
{
  t_bunny_pixelarray	*tmp;
  t_bunny_position	origin;
  unsigned int		*spx;
  unsigned int		*dpx;
  int			x;
  int			y;
  int			dx;
  int			dy;
  int			w;
  int			h;
  int			dst_w;
  int			dst_h;

  if (dst == NULL || src == NULL || pos == NULL
      || src->buffer.width <= 0 || src->buffer.height <= 0)
    return;
  w = src->buffer.width;
  h = src->buffer.height;
  tmp = bunny_new_pixelarray(w, h);
  if (tmp == NULL)
  {
    bunny_blit(&dst->clipable.buffer, src, pos);
    return;
  }
  memset(tmp->pixels, 0, sizeof(unsigned int) * (size_t)w * (size_t)h);
  origin.x = 0;
  origin.y = 0;
  bunny_blit(&tmp->clipable.buffer, src, &origin);
  spx = tmp->pixels;
  dpx = dst->pixels;
  dst_w = dst->clipable.buffer.width;
  dst_h = dst->clipable.buffer.height;
  for (y = 0; y < h; ++y)
    for (x = 0; x < w; ++x)
    {
      if (font_pixel_is_background(spx[y * w + x]))
	continue;
      dx = pos->x + x;
      dy = pos->y + y;
      if (dx < 0 || dy < 0 || dx >= dst_w || dy >= dst_h)
	continue;
      dpx[dy * dst_w + dx] = blend_color(dpx[dy * dst_w + dx],
					 spx[y * w + x], 1.0);
    }
  bunny_delete_clipable(&tmp->clipable);
}

static void		blit_pixelarray_preserve_background(t_bunny_pixelarray *dst,
			      const t_bunny_pixelarray *src,
			      const t_bunny_position *pos)
{
  const unsigned int	*spx;
  unsigned int		*dpx;
  int			x;
  int			y;
  int			dx;
  int			dy;
  int			w;
  int			h;
  int			dst_w;
  int			dst_h;

  if (dst == NULL || src == NULL || pos == NULL)
    return;
  w = src->clipable.buffer.width;
  h = src->clipable.buffer.height;
  spx = src->pixels;
  dpx = dst->pixels;
  dst_w = dst->clipable.buffer.width;
  dst_h = dst->clipable.buffer.height;
  for (y = 0; y < h; ++y)
    for (x = 0; x < w; ++x)
      {
	if (font_pixel_is_background(spx[y * w + x]))
	  continue;
	dx = pos->x + x;
	dy = pos->y + y;
	if (dx < 0 || dy < 0 || dx >= dst_w || dy >= dst_h)
	  continue;
	dpx[dy * dst_w + dx] = blend_color(dpx[dy * dst_w + dx],
				       spx[y * w + x], 1.0);
      }
}

static void		blit_pixelarray_preserve_background_shear(t_bunny_pixelarray *dst,
			      const t_bunny_pixelarray *src,
			      const t_bunny_position *pos,
			      double shear)
{
  const unsigned int	*spx;
  unsigned int		*dpx;
  int			x;
  int			y;
  int			dx;
  int			dy;
  int			w;
  int			h;
  int			dst_w;
  int			dst_h;

  if (dst == NULL || src == NULL || pos == NULL)
    return;
  w = src->clipable.buffer.width;
  h = src->clipable.buffer.height;
  spx = src->pixels;
  dpx = dst->pixels;
  dst_w = dst->clipable.buffer.width;
  dst_h = dst->clipable.buffer.height;
  for (y = 0; y < h; ++y)
    for (x = 0; x < w; ++x)
      {
	if (font_pixel_is_background(spx[y * w + x]))
	  continue;
	dx = pos->x + x + (int)((double)(h - 1 - y) * shear);
	dy = pos->y + y;
	if (dx < 0 || dy < 0 || dx >= dst_w || dy >= dst_h)
	  continue;
	dpx[dy * dst_w + dx] = blend_color(dpx[dy * dst_w + dx],
				       spx[y * w + x], 1.0);
      }
}

static int		inside_rounded_shape(double px,
				     double py,
				     double w,
				     double h,
				     double radius)
{
  double			cx;
  double			cy;
  double			dx;
  double			dy;

  if (w <= 0.0 || h <= 0.0)
    return (0);
  if (px < 0.0 || py < 0.0 || px >= w || py >= h)
    return (0);
  if (radius <= 0.0)
    return (1);
  if (radius > w * 0.5)
    radius = w * 0.5;
  if (radius > h * 0.5)
    radius = h * 0.5;
  cx = px;
  cy = py;
  if (px < radius)
    cx = radius;
  else if (px > w - radius)
    cx = w - radius;
  if (py < radius)
    cy = radius;
  else if (py > h - radius)
    cy = h - radius;
  dx = px - cx;
  dy = py - cy;
  return (dx * dx + dy * dy <= radius * radius);
}

static void		render_rounded_box(t_bunny_pixelarray *pix,
		   double box_x,
		   double box_y,
		   double box_w,
		   double box_h,
		   double radius,
		   double border,
		   unsigned int border_color,
		   unsigned int background_color)
{
  int			x;
  int			y;
  int			iw;
  int			ih;
  double		px;
  double		py;
  int			outer;
  int			inner;
  double		t;
  double		fade;
  t_bunny_color		fill_color;
  t_bunny_color		line_color;

  if (pix == NULL || box_w <= 0.0 || box_h <= 0.0)
    return;
  (void)background_color;
  iw = (int)ceil(box_w);
  ih = (int)ceil(box_h);
  line_color.full = border_color;
  for (y = 0; y < ih; ++y)
    for (x = 0; x < iw; ++x)
    {
      px = (double)x + 0.5;
      py = (double)y + 0.5;
      outer = inside_rounded_shape(px, py, box_w, box_h, radius);
      if (!outer)
	continue;
      inner = 0;
      if (box_w - border * 2.0 > 0.0 && box_h - border * 2.0 > 0.0)
	inner = inside_rounded_shape(px - border, py - border,
			       box_w - border * 2.0,
			       box_h - border * 2.0,
			       radius - border);
      t = (ih <= 1 ? 0.0 : (double)y / (double)(ih - 1));
      if (inner)
	{
	  fill_color.full = 0;
	  fill_color.argb[RED_CMP] = 255;
	  fill_color.argb[GREEN_CMP] = 255;
	  fill_color.argb[BLUE_CMP] = 255;
	  fill_color.argb[ALPHA_CMP] = 255;
	  put_pixel_alpha(pix, (int)box_x + x, (int)box_y + y,
			  fill_color.full, 1.0);
	}
      else
	{
	  if (t <= 0.40)
	    fade = 1.0;
	  else if (t >= 0.60)
	    fade = 0.0;
	  else
	    fade = 1.0 - (t - 0.40) / 0.20;
	  line_color.full = border_color;
	  line_color.argb[ALPHA_CMP] = (unsigned char)((double)line_color.argb[ALPHA_CMP] * fade + 0.5);
	  if (line_color.argb[ALPHA_CMP] > 0)
	    put_pixel_alpha(pix, (int)box_x + x, (int)box_y + y,
			    line_color.full, 1.0);
	}
    }
}

static char		*expand_text_escapes(const char *text)
{
  size_t		i;
  size_t		j;
  size_t		len;
  char		*out;

  if (text == NULL)
    return (NULL);
  len = strlen(text);
  out = malloc(len + 1);
  if (out == NULL)
    return (NULL);
  for (i = 0, j = 0; i < len; ++i)
    {
      if (text[i] == '\\' && text[i + 1] == 'n')
	{
	  out[j++] = '\n';
	  ++i;
	}
      else
	out[j++] = text[i];
    }
  out[j] = '\0';
  return (out);
}

static void		render_textbox_colored(t_bunny_pixelarray *pix,
			       const char *font_path,
			       const char *text,
			       double box_x,
			       double box_y,
			       double box_w,
			       double box_h,
			       int place_halign,
			       int place_valign,
			       unsigned int text_color)
{
  t_bunny_font		*font;
  t_bunny_pixelarray	*tmp;
  t_bunny_position	pos;
  t_bunny_position	origin;
  char			*expanded;
  double		dw;
  double		dh;

  if (font_path == NULL || font_path[0] == '\0'
      || text == NULL || text[0] == '\0')
    return;
  font = bunny_load_text(font_path);
  if (font == NULL)
  {
    fprintf(stderr, "Cannot load diploma font configuration: %s\n", font_path);
    return;
  }
  expanded = expand_text_escapes(text);
  font->string = expanded != NULL ? expanded : text;
  font->string_offset = 0;
  font->string_len = strlen(font->string);
  bunny_draw(&font->clipable);
  tmp = bunny_new_pixelarray(font->clipable.buffer.width, font->clipable.buffer.height);
  if (tmp == NULL)
    {
      free(expanded);
      bunny_delete_clipable(&font->clipable);
      return;
    }
  memset(tmp->pixels, 0, sizeof(unsigned int)
	 * (size_t)tmp->clipable.buffer.width * (size_t)tmp->clipable.buffer.height);
  origin.x = 0;
  origin.y = 0;
  bunny_blit(&tmp->clipable.buffer, &font->clipable, &origin);
  recolor_font_foreground(tmp, text_color);
  dw = (double)font->clipable.buffer.width;
  dh = (double)font->clipable.buffer.height;
  if (place_halign == BAL_RIGHT)
    pos.x = (int)(box_x + box_w - dw);
  else if (place_halign == BAL_MIDDLE)
    pos.x = (int)(box_x + (box_w - dw) * 0.5);
  else
    pos.x = (int)box_x;
  if (place_valign == BAL_BOTTOM)
    pos.y = (int)(box_y + box_h - dh);
  else if (place_valign == BAL_MIDDLE)
    pos.y = (int)(box_y + (box_h - dh) * 0.5);
  else
    pos.y = (int)box_y;
  blit_pixelarray_preserve_background(pix, tmp, &pos);
  bunny_delete_clipable(&tmp->clipable);
  free(expanded);
  bunny_delete_clipable(&font->clipable);
}

static void		render_textbox(t_bunny_pixelarray *pix,
			       const char *font_path,
			       const char *text,
			       double box_x,
			       double box_y,
			       double box_w,
			       double box_h,
			       int place_halign,
			       int place_valign)
{
  t_bunny_font		*font;
  t_bunny_position	pos;
  char			*expanded;
  double		dw;
  double		dh;

  if (font_path == NULL || font_path[0] == '\0'
      || text == NULL || text[0] == '\0')
    return;
  font = bunny_load_text(font_path);
  if (font == NULL)
  {
    fprintf(stderr, "Cannot load diploma font configuration: %s\n", font_path);
    return;
  }
  expanded = expand_text_escapes(text);
  font->string = expanded != NULL ? expanded : text;
  font->string_offset = 0;
  font->string_len = strlen(font->string);
  bunny_draw(&font->clipable);
  dw = (double)font->clipable.buffer.width;
  dh = (double)font->clipable.buffer.height;
  if (place_halign == BAL_RIGHT)
    pos.x = (int)(box_x + box_w - dw);
  else if (place_halign == BAL_MIDDLE)
    pos.x = (int)(box_x + (box_w - dw) * 0.5);
  else
    pos.x = (int)box_x;
  if (place_valign == BAL_BOTTOM)
    pos.y = (int)(box_y + box_h - dh);
  else if (place_valign == BAL_MIDDLE)
    pos.y = (int)(box_y + (box_h - dh) * 0.5);
  else
    pos.y = (int)box_y;
  blit_font_clipable_preserve_background(pix, &font->clipable, &pos);
  free(expanded);
  bunny_delete_clipable(&font->clipable);
}

static void		render_textbox_italic(t_bunny_pixelarray *pix,
			      const char *font_path,
			      const char *text,
			      double box_x,
			      double box_y,
			      double box_w,
			      double box_h,
			      int place_halign,
			      int place_valign)
{
  t_bunny_font		*font;
  t_bunny_pixelarray	*tmp;
  t_bunny_position	pos;
  t_bunny_position	origin;
  char			*expanded;
  double		shear;
  double		dw;
  double		dh;

  if (font_path == NULL || font_path[0] == '\0'
      || text == NULL || text[0] == '\0')
    return;
  font = bunny_load_text(font_path);
  if (font == NULL)
  {
    fprintf(stderr, "Cannot load diploma font configuration: %s\n", font_path);
    return;
  }
  expanded = expand_text_escapes(text);
  font->string = expanded != NULL ? expanded : text;
  font->string_offset = 0;
  font->string_len = strlen(font->string);
  bunny_draw(&font->clipable);
  tmp = bunny_new_pixelarray(font->clipable.buffer.width,
			     font->clipable.buffer.height);
  if (tmp == NULL)
    {
      free(expanded);
      bunny_delete_clipable(&font->clipable);
      return;
    }
  memset(tmp->pixels, 0, sizeof(unsigned int)
	 * (size_t)tmp->clipable.buffer.width * (size_t)tmp->clipable.buffer.height);
  origin.x = 0;
  origin.y = 0;
  bunny_blit(&tmp->clipable.buffer, &font->clipable, &origin);
  shear = 0.18;
  dw = (double)font->clipable.buffer.width
    + (double)font->clipable.buffer.height * shear;
  dh = (double)font->clipable.buffer.height;
  if (place_halign == BAL_RIGHT)
    pos.x = (int)(box_x + box_w - dw);
  else if (place_halign == BAL_MIDDLE)
    pos.x = (int)(box_x + (box_w - dw) * 0.5);
  else
    pos.x = (int)box_x;
  if (place_valign == BAL_BOTTOM)
    pos.y = (int)(box_y + box_h - dh);
  else if (place_valign == BAL_MIDDLE)
    pos.y = (int)(box_y + (box_h - dh) * 0.5);
  else
    pos.y = (int)box_y;
  blit_pixelarray_preserve_background_shear(pix, tmp, &pos, shear);
  bunny_delete_clipable(&tmp->clipable);
  free(expanded);
  bunny_delete_clipable(&font->clipable);
}

static void		render_recipient_alias(t_bunny_pixelarray *pix,
			       const t_diploma_style *style,
			       double w,
			       double h)
{
  char			buffer[512];

  if (style->recipient_alias == NULL || style->recipient_alias[0] == '\0')
    return;
  snprintf(buffer, sizeof(buffer), "« %s »", style->recipient_alias);
  render_textbox_italic(pix, style->recipient_alias_font_path, buffer,
			0.250 * w, 0.323 * h, 0.500 * w, 0.040 * h,
			BAL_MIDDLE, BAL_MIDDLE);
}

static void		render_diploma_information(t_bunny_pixelarray *pix,
			      const t_diploma_style *style)
{
  double		w;
  double		h;
  double		margin;

  w = pix->clipable.buffer.width;
  h = pix->clipable.buffer.height;
  margin = (double)style->border_margin * (double)style->super_sampling;
  render_textbox(pix, style->promotion_font_path, style->promotion_text,
		 margin, 0.021 * h, 0.230 * w, 0.050 * h,
		 BAL_LEFT, BAL_TOP);
  render_textbox(pix, style->school_name_font_path, style->school_name,
		 0.180 * w, 0.180 * h, 0.640 * w, 0.055 * h,
		 BAL_MIDDLE, BAL_MIDDLE);
  render_textbox(pix, style->recommendation_font_path,
		 style->recommendation_text,
		 0.170 * w, 0.230 * h, 0.660 * w, 0.045 * h,
		 BAL_MIDDLE, BAL_MIDDLE);
  render_textbox(pix, style->recipient_name_font_path, style->recipient_name,
		 0.250 * w, 0.270 * h, 0.500 * w, 0.060 * h,
		 BAL_MIDDLE, BAL_MIDDLE);
  render_recipient_alias(pix, style, w, h);
  render_textbox(pix, style->recipient_birth_font_path,
		 style->recipient_birth_text,
		 0.250 * w, 0.358 * h, 0.500 * w, 0.040 * h,
		 BAL_MIDDLE, BAL_MIDDLE);
  render_textbox(pix, style->attribution_font_path, style->attribution_text,
		 0.250 * w, 0.393 * h, 0.500 * w, 0.040 * h,
		 BAL_MIDDLE, BAL_MIDDLE);
  render_textbox(pix, style->font_path, style->main_text,
		 0.100 * w, 0.410 * h, 0.800 * w, 0.170 * h,
		 BAL_MIDDLE, BAL_MIDDLE);
  render_textbox(pix, style->certification_font_path,
		 style->certification_text,
		 0.110 * w, 0.570 * h, 0.780 * w, 0.090 * h,
		 BAL_MIDDLE, BAL_MIDDLE);
  render_image_fit(pix, style->seal_path,
		   0.455 * w, 0.650 * h, 0.090 * w, 0.080 * h);
}

static int		signatory_is_titulaire(const char *label)
{
  if (label == NULL)
    return (0);
  return (strstr(label, "Titulaire") != NULL
	  || strstr(label, "titulaire") != NULL
	  || strstr(label, "TITULAIRE") != NULL);
}

static void		render_signatories(t_bunny_pixelarray *pix,
			   const t_diploma_style *style)
{
  double		w;
  double		h;
  double		x;
  double		y;
  double		width;
  double		height;
  double		cell_w;
  double		cell_x;
  double		block_x;
  double		block_y;
  double		block_w;
  double		block_h;
  double		pad_x;
  double		radius;
  double		border;
  const char		*name;
  const char		*label;
  unsigned int	black;
  int		i;

  if (style->signatory_count <= 0)
    return;
  w = pix->clipable.buffer.width;
  h = pix->clipable.buffer.height;
  x = 0.120 * w;
  y = 0.735 * h;
  width = 0.760 * w;
  height = 0.145 * h;
  cell_w = width / (double)style->signatory_count;
  black = make_color(0, 0, 0, 255);
  for (i = 0; i < style->signatory_count; ++i)
  {
    cell_x = x + cell_w * (double)i;
    block_x = cell_x + cell_w * 0.045;
    block_y = y + height * 0.03;
    block_w = cell_w * 0.91;
    block_h = height * 0.88;
    radius = block_h * 0.10;
    border = block_h * 0.016;
    if (border < 2.0)
      border = 2.0;
    render_rounded_box(pix, block_x, block_y, block_w, block_h,
		       radius, border, style->border_color,
		       style->background_color);
    pad_x = block_w * 0.06;
    label = style->signatories[i].label;
    if (signatory_is_titulaire(label))
      label = "Titulaire";
    name = style->signatories[i].name;
    if ((name == NULL || name[0] == '\0') && style->recipient_name != NULL
	&& style->recipient_name[0] != '\0'
	&& signatory_is_titulaire(style->signatories[i].label))
      name = style->recipient_name;
    if (label != NULL && label[0] != '\0')
      {
	render_textbox_colored(pix, style->signatory_label_font_path,
			       label,
			       block_x + pad_x, block_y + block_h * 0.06,
			       block_w - pad_x * 2.0, block_h * 0.12,
			       BAL_MIDDLE, BAL_TOP, black);
	render_textbox_colored(pix, style->signatory_name_font_path,
			       name,
			       block_x + pad_x, block_y + block_h * 0.18,
			       block_w - pad_x * 2.0, block_h * 0.15,
			       BAL_MIDDLE, BAL_TOP, black);
	render_textbox_colored(pix, style->signatory_title_font_path,
			       style->signatories[i].title,
			       block_x + pad_x, block_y + block_h * 0.31,
			       block_w - pad_x * 2.0, block_h * 0.14,
			       BAL_MIDDLE, BAL_TOP, black);
      }
    else
      {
	render_textbox_colored(pix, style->signatory_title_font_path,
			       style->signatories[i].title,
			       block_x + pad_x, block_y + block_h * 0.06,
			       block_w - pad_x * 2.0, block_h * 0.14,
			       BAL_MIDDLE, BAL_TOP, black);
	render_textbox_colored(pix, style->signatory_name_font_path,
			       name,
			       block_x + pad_x, block_y + block_h * 0.18,
			       block_w - pad_x * 2.0, block_h * 0.15,
			       BAL_MIDDLE, BAL_TOP, black);
      }
    render_image_fit(pix, style->signatories[i].signature_path,
		     block_x + block_w * 0.18, block_y + block_h * 0.56,
		     block_w * 0.64, block_h * 0.20);
  }
}


static void		render_footer_texts(t_bunny_pixelarray *pix,
			    const t_diploma_style *style)
{
  double		margin;
  double		box_x;
  double		box_y;
  double		box_w;
  double		box_h;

  margin = (double)style->border_margin * (double)style->super_sampling;
  box_x = margin;
  box_y = (double)pix->clipable.buffer.height - margin + margin * 0.06;
  box_w = (double)pix->clipable.buffer.width - margin * 2.0;
  box_h = margin * 0.90;
  render_textbox(pix, style->diploma_number_font_path,
		     style->diploma_number_text, box_x, box_y, box_w, box_h,
		     BAL_LEFT, BAL_TOP);
  render_textbox(pix, style->school_phrase_font_path,
		     style->school_phrase, box_x, box_y, box_w, box_h,
		     BAL_LEFT, BAL_TOP);
}

void			render_diploma_static_assets(t_bunny_pixelarray *pix,
			     const t_diploma_style *style)
{
  render_logo(pix, style);
  render_diploma_information(pix, style);
  render_signatories(pix, style);
  render_footer_texts(pix, style);
}
