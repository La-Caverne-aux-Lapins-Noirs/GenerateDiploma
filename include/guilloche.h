#ifndef DIPLOMA_GUILLOCHE_H
# define DIPLOMA_GUILLOCHE_H

# include <lapin.h>

typedef struct s_guilloche_curve
{
  double cx;
  double cy;

  double rx;
  double ry;

  int    lines;
  int    turns;
  int    samples_per_turn;

  double amp_a;
  double amp_b;
  double freq_a;
  double freq_b;

  double twist_amp;
  double twist_freq;

  double line_radius_step;
  double line_phase_step;

  double global_phase;

  unsigned int color_a;
  unsigned int color_b;

  double opacity;
} t_guilloche_curve;

typedef struct s_concentric_guilloche
{
  double cx;
  double cy;

  int    rings;
  int    samples_per_ring;

  double first_radius;
  double radius_step;

  double wave_amp;
  double wave_freq;
  double ring_phase_step;

  double spiral_amp;
  double spiral_freq;

  double color_freq;
  double color_ring_shift;

  unsigned int color_a;
  unsigned int color_b;

  double opacity;
} t_concentric_guilloche;

void draw_guilloche_curve(
  t_bunny_pixelarray *pix,
  const t_guilloche_curve *g
);

void draw_concentric_guilloche(
  t_bunny_pixelarray *pix,
  const t_concentric_guilloche *g
);

#endif

