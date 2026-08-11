// Jason Brillante "Damdoshi"
// Hanged Bunny Studio 2014-2026
// EFRITS SAS 2022-2026
// Pentacle Technologie 2008-2026
//
// GenDiplome

#include		<assert.h>
#include		<stdio.h>
#include		"program.h"

int			main(int argc,
			     char **argv)
{
  t_bunny_configuration *config;
  t_program_options	opt;
  t_diploma_style	style;
  const char		*codename;
  const char		*output;
  t_bunny_pixelarray	*render;
  t_bunny_pixelarray	*final;

  bunny_enable_full_blit(true);

  if (!load_diploma_configuration(argc, argv, &config, &opt))
    return (1);
  diploma_style_from_configuration(config, &style, opt.secret);
  codename = diploma_codename_from_configuration(config, opt.codename_override);
  output = diploma_output_from_configuration(config, &opt);

  assert((render = bunny_new_pixelarray(style.width * style.super_sampling,
						style.height * style.super_sampling)));
  render_diploma_background(render, &style, codename, opt.secret);
  render_diploma_static_assets(render, &style);
  final = downsample_pixelarray(render, style.super_sampling);
  if (final == NULL)
  {
    fprintf(stderr, "Cannot downsample diploma.\n");
    return (1);
  }
  bunny_save_pixelarray(final, output);
  if (final != render)
    bunny_delete_clipable(&final->clipable);
  bunny_delete_clipable(&render->clipable);
  bunny_delete_configuration(config);
  return (0);
}
