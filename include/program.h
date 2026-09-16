// Jason Brillante "Damdoshi"
// Hanged Bunny Studio 2014-2026
// EFRITS SAS 2022-2026
// Pentacle Technologie 2008-2026
//
// GenDiplome

#ifndef			GENDIPLOME_H
# define		GENDIPLOME_H
# include		<lapin.h>
# include		<stdbool.h>
# include		<stdint.h>

# define		MAX_DIPLOMA_SIGNATORIES 12

typedef struct		s_diploma_signatory
{
  const char		*label;
  const char		*name;
  const char		*title;
  const char		*signature_path;
} 			t_diploma_signatory;

typedef struct		s_program_options
{
  const char		*output_file;
  bool			output_overridden;
  const char		*codename_override;
  const char		*secret;
  bool			secret_provided;
  bool			secret_disabled;
} 			t_program_options;

typedef struct		s_diploma_style
{
  int			width;
  int			height;
  int			super_sampling;

  unsigned int		background_color;
  unsigned int		border_color;

  int			border_margin;
  int			border_min_horizontal_length;
  int			border_max_horizontal_length;
  int			border_min_vertical_length;
  int			border_max_vertical_length;
  int			border_min_thickness;
  int			border_max_thickness;
  int			border_min_lines;
  int			border_max_lines;
  int			border_line_spacing;

  int			border_glow_min_style;
  int			border_glow_max_style;

  bool			botanical_enabled;
  unsigned int		botanical_color;
  int			botanical_margin;
  int			botanical_min_scale;
  int			botanical_max_scale;
  int			botanical_min_depth;
  int			botanical_max_depth;
  int			botanical_min_thickness;
  int			botanical_max_thickness;

  const char		*logo_path;
  const char		*seal_path;

  const char		*font_path;
  const char		*main_text;

  const char		*promotion_font_path;
  int			promotion_year;
  char			promotion_text[64];
  const char		*school_name_font_path;
  const char		*school_name;
  const char		*recommendation_font_path;
  const char		*recommendation_text;
  const char		*recipient_name_font_path;
  const char		*recipient_name;
  const char		*recipient_alias_font_path;
  const char		*recipient_alias;
  const char		*recipient_birth_font_path;
  const char		*recipient_birth_text;
  char			recipient_birth_text_gendered[512];
  const char		*attribution_font_path;
  const char		*attribution_text;
  const char		*certification_font_path;
  const char		*certification_text;

  const char		*diploma_number_font_path;
  const char		*school_phrase_font_path;
  const char		*school_phrase;
  int			recipient_number;
  char			diploma_key_text[9];
  char			diploma_number_text[96];

  const char		*signatory_label_font_path;
  const char		*signatory_name_font_path;
  const char		*signatory_title_font_path;
  t_diploma_signatory	signatories[MAX_DIPLOMA_SIGNATORIES];
  int			signatory_count;

  unsigned int		text_color;
} 			t_diploma_style;

void			rng_init(t_bunny_hash r);
double			rngf(void);
int			rngi(int min, int max);

double			clamp(double v);
unsigned int		blend_color(unsigned int dst,
			    unsigned int src,
			    double alpha_mul);
unsigned int		lerp_color(unsigned int ca,
			   unsigned int cb,
			   double t);
void			set_pixel(t_bunny_pixelarray *pix,
			  t_bunny_position pos,
			  unsigned int color,
			  double alpha);
void			set_line(t_bunny_pixelarray *pix,
			 t_bunny_accurate_position *pos,
			 unsigned int *col,
			 double opacity);

bool			load_diploma_configuration(int argc,
					 char **argv,
					 t_bunny_configuration **config,
					 t_program_options *opt);
void			diploma_style_from_configuration(t_bunny_configuration *config,
						     t_diploma_style *style,
						     const char *secret);
const char		*diploma_output_from_configuration(t_bunny_configuration *config,
						       const t_program_options *opt);
const char		*diploma_codename_from_configuration(t_bunny_configuration *config,
							 const char *fallback);
bool			diploma_secret_is_disabled(const char *secret);
uint64_t		diploma_keyed_hash64(const char *secret,
				    const char *purpose,
				    const char *payload);
void			diploma_keyed_alnum8(char out[9],
				    const char *secret,
				    const char *purpose,
				    const char *payload);
void			render_diploma_background(t_bunny_pixelarray *pix,
						  const t_diploma_style *style,
						  const char *codename,
						  const char *secret);
void			render_diploma_static_assets(t_bunny_pixelarray *pix,
						     const t_diploma_style *style);
t_bunny_pixelarray	*downsample_pixelarray(t_bunny_pixelarray *src,
					   int factor);

#endif	//		GENDIPLOME_H
