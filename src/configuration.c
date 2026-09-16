// Jason Brillante "Damdoshi"
// Hanged Bunny Studio 2014-2026
// EFRITS SAS 2022-2026
// Pentacle Technologie 2008-2026
//
// GenDiplome

#include		<stdio.h>
#include		<stdlib.h>
#include		<string.h>
#include		"program.h"

#ifndef GENDIPLOMA_DATADIR
# define GENDIPLOMA_DATADIR "res"
#endif

#define GENDIPLOMA_RESOURCE(file) GENDIPLOMA_DATADIR "/" file

static unsigned char	hex_value(char c)
{
  if (c >= '0' && c <= '9')
    return ((unsigned char)(c - '0'));
  if (c >= 'a' && c <= 'f')
    return ((unsigned char)(10 + c - 'a'));
  if (c >= 'A' && c <= 'F')
    return ((unsigned char)(10 + c - 'A'));
  return (0);
}

static bool		parse_hex_color(const char *str,
					unsigned int *out)
{
  t_bunny_color		c;
  int			len;

  if (str == NULL || str[0] != '#')
    return (false);
  len = (int)strlen(str + 1);
  if (len != 6 && len != 8)
    return (false);
  c.argb[RED_CMP] = (hex_value(str[1]) << 4) | hex_value(str[2]);
  c.argb[GREEN_CMP] = (hex_value(str[3]) << 4) | hex_value(str[4]);
  c.argb[BLUE_CMP] = (hex_value(str[5]) << 4) | hex_value(str[6]);
  c.argb[ALPHA_CMP] = 255;
  if (len == 8)
    c.argb[ALPHA_CMP] = (hex_value(str[7]) << 4) | hex_value(str[8]);
  *out = c.full;
  return (true);
}

static void		set_argb(unsigned int *out,
				 unsigned char r,
				 unsigned char g,
				 unsigned char b,
				 unsigned char a)
{
  t_bunny_color		c;

  c.argb[RED_CMP] = r;
  c.argb[GREEN_CMP] = g;
  c.argb[BLUE_CMP] = b;
  c.argb[ALPHA_CMP] = a;
  *out = c.full;
}

static bool		conf_get_string(t_bunny_configuration *config,
					const char **out,
					const char *path)
{
  if (config == NULL)
    return (false);
  return (bunny_configuration_getf(config, out, path));
}

static bool		conf_get_int(t_bunny_configuration *config,
				     int *out,
				     const char *path)
{
  if (config == NULL)
    return (false);
  return (bunny_configuration_getf(config, out, path));
}

static bool		conf_get_color(t_bunny_configuration *config,
				       unsigned int *out,
				       const char *path)
{
  const char		*str;
  int			integer;

  if (conf_get_string(config, &str, path) && parse_hex_color(str, out))
    return (true);
  if (conf_get_int(config, &integer, path))
  {
    *out = (unsigned int)integer;
    return (true);
  }
  return (false);
}

static bool		conf_get_string_copy(t_bunny_configuration *config,
			  char *out,
			  size_t size,
			  const char *path)
{
  const char		*str;

  if (size == 0)
    return (false);
  if (!conf_get_string(config, &str, path))
    return (false);
  snprintf(out, size, "%s", str);
  return (true);
}

static int		extract_first_int(const char *str)
{
  int			value;

  if (str == NULL)
    return (-1);
  while (*str != '\0')
    {
      if (*str >= '0' && *str <= '9')
	{
	  value = 0;
	  while (*str >= '0' && *str <= '9')
	    {
	      value = value * 10 + (*str - '0');
	      ++str;
	    }
	  return (value);
	}
      ++str;
    }
  return (-1);
}

static int		load_promotion_year(t_bunny_configuration *config,
				    const char *promotion_text)
{
  int			promo;

  promo = -1;
  conf_get_int(config, &promo, "Diploma.Promotion.Year");
  if (promo < 0)
    promo = extract_first_int(promotion_text);
  if (promo < 0)
    conf_get_int(config, &promo, "Diploma.Recipient.Promo");
  if (promo < 0)
    conf_get_int(config, &promo, "Diploma.Recipient.Promotion");
  if (promo < 0)
    conf_get_int(config, &promo, "Student.Promo");
  if (promo < 0)
    conf_get_int(config, &promo, "Student.Promotion");
  return (promo);
}

static int		load_recipient_number(t_bunny_configuration *config)
{
  int			number;

  number = -1;
  conf_get_int(config, &number, "Diploma.Recipient.Number");
  if (number < 0)
    conf_get_int(config, &number, "Diploma.Recipient.Id");
  if (number < 0)
    conf_get_int(config, &number, "Diploma.Recipient.ID");
  if (number < 0)
    conf_get_int(config, &number, "Student.Number");
  if (number < 0)
    conf_get_int(config, &number, "Student.Id");
  if (number < 0)
    conf_get_int(config, &number, "Student.ID");
  return (number);
}

static const char	*load_recipient_gender(t_bunny_configuration *config)
{
  const char		*gender;

  gender = "";
  if (conf_get_string(config, &gender, "Diploma.Recipient.Gender"))
    return (gender);
  if (conf_get_string(config, &gender, "Recipient.Gender"))
    return (gender);
  if (conf_get_string(config, &gender, "Student.Gender"))
    return (gender);
  return ("");
}

static void		gender_recipient_birth_text(t_bunny_configuration *config,
				    t_diploma_style *style)
{
  const char		*gender;
  const char		*marker;
  const char		*replacement;
  size_t		prefix_len;

  if (style->recipient_birth_text == NULL)
    return ;
  gender = load_recipient_gender(config);
  replacement = NULL;
  if (strcmp(gender, "female") == 0)
    replacement = "Née";
  else if (strcmp(gender, "male") == 0)
    replacement = "Né";
  if (replacement == NULL)
    return ;
  marker = strstr(style->recipient_birth_text, "Né(e)");
  if (marker == NULL)
    return ;
  prefix_len = (size_t)(marker - style->recipient_birth_text);
  snprintf(style->recipient_birth_text_gendered,
           sizeof(style->recipient_birth_text_gendered),
           "%.*s%s%s", (int)prefix_len, style->recipient_birth_text,
           replacement, marker + strlen("Né(e)"));
  style->recipient_birth_text = style->recipient_birth_text_gendered;
}

static void		make_diploma_key(t_bunny_configuration *config,
			 t_diploma_style *style,
			 const char *secret)
{
  const char		*codename;
  char			buf[2048];

  codename = "";
  conf_get_string(config, &codename, "Diploma.Recipient.Codename");
  if (codename == NULL || codename[0] == '\0')
    conf_get_string(config, &codename, "Recipient.Codename");
  if (codename == NULL || codename[0] == '\0')
    conf_get_string(config, &codename, "Student.Codename");
  snprintf(buf, sizeof(buf), "%s#%s#%s#%s#%d#%05d",
	   codename != NULL ? codename : "",
	   style->recipient_name != NULL ? style->recipient_name : "",
	   style->recipient_alias != NULL ? style->recipient_alias : "",
	   style->recipient_birth_text != NULL ? style->recipient_birth_text : "",
	   style->promotion_year,
	   style->recipient_number);
  diploma_keyed_alnum8(style->diploma_key_text, secret,
		       "diploma-key", buf);
}

static void		load_promotion_text(t_bunny_configuration *config,
			    t_diploma_style *style)
{
  style->promotion_year = load_promotion_year(config, style->promotion_text);
  if (style->promotion_year >= 0)
    snprintf(style->promotion_text, sizeof(style->promotion_text),
	     "Promotion %d", style->promotion_year);
}

static void		load_signatories(t_bunny_configuration *config,
			 t_diploma_style *style)
{
  int		i;
  bool		has_one;

  style->signatory_count = 0;
  for (i = 0; i < MAX_DIPLOMA_SIGNATORIES; ++i)
  {
    style->signatories[i].label = NULL;
    style->signatories[i].name = NULL;
    style->signatories[i].title = NULL;
    style->signatories[i].signature_path = NULL;
    has_one = false;
    has_one |= bunny_configuration_getf(config, &style->signatories[i].label,
				       "Diploma.Signatories[%d].Label", i);
    has_one |= bunny_configuration_getf(config, &style->signatories[i].label,
				       "Diploma.Signatories[%d].Role", i);
    has_one |= bunny_configuration_getf(config, &style->signatories[i].name,
				       "Diploma.Signatories[%d].Name", i);
    has_one |= bunny_configuration_getf(config, &style->signatories[i].title,
				       "Diploma.Signatories[%d].Title", i);
    has_one |= bunny_configuration_getf(config, &style->signatories[i].signature_path,
				       "Diploma.Signatories[%d].Signature", i);
    has_one |= bunny_configuration_getf(config, &style->signatories[i].signature_path,
				       "Diploma.Signatories[%d].SignaturePath", i);
    if (!has_one)
      break;
    ++style->signatory_count;
  }
}

bool			load_diploma_configuration(int argc,
					 char **argv,
					 t_bunny_configuration **config,
					 t_program_options *opt)
{
  int			i;

  *config = NULL;
  opt->output_file = "out.png";
  opt->output_overridden = false;
  opt->codename_override = NULL;
  opt->secret = NULL;
  opt->secret_provided = false;
  opt->secret_disabled = false;
  for (i = 1; i < argc; ++i)
  {
    if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output"))
    {
      if (i + 1 >= argc)
	{
	  fprintf(stderr, "%s expects an output path.\n", argv[i]);
	  return (false);
	}
      opt->output_file = argv[++i];
      opt->output_overridden = true;
    }
    else if (!strcmp(argv[i], "-n") || !strcmp(argv[i], "--codename"))
      {
	if (i + 1 >= argc)
	  {
	    fprintf(stderr, "%s expects a codename.\n", argv[i]);
	    return (false);
	  }
	opt->codename_override = argv[++i];
      }
    else if (!strcmp(argv[i], "--secret"))
      {
	if (i + 1 >= argc)
	  {
	    fprintf(stderr, "--secret expects a value. Use --secret none only for intentionally unsecured generation.\n");
	    return (false);
	  }
	opt->secret = argv[++i];
	opt->secret_provided = true;
      }
    else if (!strncmp(argv[i], "--secret=", 9))
      {
	opt->secret = argv[i] + 9;
	opt->secret_provided = true;
      }
    else if (argv[i][0] == '-')
      {
	fprintf(stderr, "Unknown option: %s\n", argv[i]);
	return (false);
      }
    else
    {
      *config = bunny_open_configuration(argv[i], *config);
      if (*config == NULL)
      {
	fprintf(stderr, "Cannot load configuration: %s\n", argv[i]);
	return (false);
      }
    }
  }
  if (*config == NULL)
    *config = bunny_new_configuration();
  if (!opt->secret_provided)
    {
      fprintf(stderr, "A diploma secret is required. Use --secret <secret> for authenticated generation, or --secret none for intentionally unsecured generation.\n");
      return (false);
    }
  if (opt->secret == NULL || opt->secret[0] == '\0')
    {
      fprintf(stderr, "The diploma secret cannot be empty. Use --secret none only for intentionally unsecured generation.\n");
      return (false);
    }
  opt->secret_disabled = diploma_secret_is_disabled(opt->secret);
  if (opt->secret_disabled)
    fprintf(stderr, "Warning: generating without a diploma secret because --secret none was specified.\n");
  return (*config != NULL);
}

void			diploma_style_from_configuration(t_bunny_configuration *config,
					     t_diploma_style *style,
					     const char *secret)
{
  style->width = 3508;
  style->height = 2480;
  style->super_sampling = 2;
  set_argb(&style->background_color, 0, 0, 0, 255);
  set_argb(&style->border_color, 0, 255, 96, 255);
  style->border_margin = 150;
  style->border_min_horizontal_length = 420;
  style->border_max_horizontal_length = 760;
  style->border_min_vertical_length = 220;
  style->border_max_vertical_length = 420;
  style->border_min_thickness = 2;
  style->border_max_thickness = 5;
  style->border_min_lines = 1;
  style->border_max_lines = 4;
  style->border_line_spacing = 16;
  style->border_glow_min_style = 0;
  style->border_glow_max_style = 8;

  style->botanical_enabled = true;
  set_argb(&style->botanical_color, 0, 255, 120, 220);
  style->botanical_margin = 170;
  style->botanical_min_scale = 900;
  style->botanical_max_scale = 1350;
  style->botanical_min_depth = 3;
  style->botanical_max_depth = 5;
  style->botanical_min_thickness = 1;
  style->botanical_max_thickness = 3;

  style->logo_path = NULL;
  style->seal_path = NULL;
  style->font_path = NULL;
  style->main_text = "DIPLOME";
  style->promotion_font_path = GENDIPLOMA_RESOURCE("diploma_promotion_text.dab");
  style->promotion_year = -1;
  style->promotion_text[0] = '\0';
  style->school_name_font_path = GENDIPLOMA_RESOURCE("diploma_school_name_text.dab");
  style->school_name = "";
  style->recommendation_font_path = GENDIPLOMA_RESOURCE("diploma_recommendation_text.dab");
  style->recommendation_text = "";
  style->recipient_name_font_path = GENDIPLOMA_RESOURCE("diploma_recipient_name_text.dab");
  style->recipient_name = "";
  style->recipient_alias_font_path = GENDIPLOMA_RESOURCE("diploma_recipient_alias_text.dab");
  style->recipient_alias = "";
  style->recipient_birth_font_path = GENDIPLOMA_RESOURCE("diploma_recipient_birth_text.dab");
  style->recipient_birth_text = "";
  style->recipient_birth_text_gendered[0] = '\0';
  style->attribution_font_path = GENDIPLOMA_RESOURCE("diploma_attribution_text.dab");
  style->attribution_text = "";
  style->certification_font_path = GENDIPLOMA_RESOURCE("diploma_certification_text.dab");
  style->certification_text = "";
  style->diploma_number_font_path = GENDIPLOMA_RESOURCE("diploma_number_text.dab");
  style->school_phrase_font_path = GENDIPLOMA_RESOURCE("diploma_school_phrase_text.dab");
  style->school_phrase = "";
  style->recipient_number = -1;
  style->diploma_key_text[0] = '\0';
  style->diploma_number_text[0] = '\0';
  style->signatory_label_font_path = GENDIPLOMA_RESOURCE("diploma_signatory_label_text.dab");
  style->signatory_name_font_path = GENDIPLOMA_RESOURCE("diploma_signatory_name_text.dab");
  style->signatory_title_font_path = GENDIPLOMA_RESOURCE("diploma_signatory_title_text.dab");
  style->signatory_count = 0;
  set_argb(&style->text_color, 0, 255, 120, 230);

  conf_get_int(config, &style->width, "Diploma.Width");
  conf_get_int(config, &style->height, "Diploma.Height");
  conf_get_int(config, &style->super_sampling, "Diploma.SuperSampling");
  conf_get_color(config, &style->background_color, "Diploma.Background");
  conf_get_color(config, &style->background_color, "Diploma.Background.Color");
  conf_get_color(config, &style->border_color, "Diploma.Border.Color");
  conf_get_color(config, &style->border_color, "Diploma.Liseret.Color");
  conf_get_int(config, &style->border_margin, "Diploma.Border.Margin");
  conf_get_int(config, &style->border_min_horizontal_length,
	       "Diploma.Border.MinHorizontalLength");
  conf_get_int(config, &style->border_max_horizontal_length,
	       "Diploma.Border.MaxHorizontalLength");
  conf_get_int(config, &style->border_min_vertical_length,
	       "Diploma.Border.MinVerticalLength");
  conf_get_int(config, &style->border_max_vertical_length,
	       "Diploma.Border.MaxVerticalLength");
  conf_get_int(config, &style->border_min_thickness, "Diploma.Border.MinThickness");
  conf_get_int(config, &style->border_max_thickness, "Diploma.Border.MaxThickness");
  conf_get_int(config, &style->border_min_lines, "Diploma.Border.MinLines");
  conf_get_int(config, &style->border_max_lines, "Diploma.Border.MaxLines");
  conf_get_int(config, &style->border_line_spacing, "Diploma.Border.LineSpacing");
  conf_get_int(config, &style->border_glow_min_style,
	       "Diploma.Border.GlowMinStyle");
  conf_get_int(config, &style->border_glow_max_style,
	       "Diploma.Border.GlowMaxStyle");

  conf_get_int(config, (int*)&style->botanical_enabled,
	       "Diploma.Botanical.Enabled");
  conf_get_color(config, &style->botanical_color, "Diploma.Botanical.Color");
  conf_get_int(config, &style->botanical_margin,
	       "Diploma.Botanical.Margin");
  conf_get_int(config, &style->botanical_min_scale,
	       "Diploma.Botanical.MinScale");
  conf_get_int(config, &style->botanical_max_scale,
	       "Diploma.Botanical.MaxScale");
  conf_get_int(config, &style->botanical_min_depth,
	       "Diploma.Botanical.MinDepth");
  conf_get_int(config, &style->botanical_max_depth,
	       "Diploma.Botanical.MaxDepth");
  conf_get_int(config, &style->botanical_min_thickness,
	       "Diploma.Botanical.MinThickness");
  conf_get_int(config, &style->botanical_max_thickness,
	       "Diploma.Botanical.MaxThickness");

  conf_get_string(config, &style->logo_path, "Diploma.Logo.Path");
  conf_get_string(config, &style->logo_path, "Diploma.Logo.File");
  conf_get_string(config, &style->logo_path, "Diploma.Assets.Logo");
  conf_get_string(config, &style->seal_path, "Diploma.Seal.Path");
  conf_get_string(config, &style->seal_path, "Diploma.Seal.File");
  conf_get_string(config, &style->font_path, "Diploma.Font.Path");
  conf_get_string(config, &style->font_path, "Diploma.Font.File");
  conf_get_string(config, &style->font_path, "Diploma.Assets.Font");
  conf_get_string(config, &style->main_text, "Diploma.Text.Main");
  conf_get_string(config, &style->main_text, "Diploma.Text.First");
  conf_get_string(config, &style->promotion_font_path,
		  "Diploma.Promotion.Font");
  conf_get_string_copy(config, style->promotion_text,
		       sizeof(style->promotion_text),
		       "Diploma.Promotion.Text");
  load_promotion_text(config, style);
  conf_get_string(config, &style->school_name_font_path,
		  "Diploma.School.NameFont");
  conf_get_string(config, &style->school_name_font_path,
		  "Diploma.SchoolName.Font");
  conf_get_string(config, &style->school_name, "Diploma.School.Name");
  conf_get_string(config, &style->school_name, "School.Name");
  conf_get_string(config, &style->recommendation_font_path,
		  "Diploma.Recommendation.Font");
  conf_get_string(config, &style->recommendation_text,
		  "Diploma.Recommendation.Text");
  conf_get_string(config, &style->recommendation_text,
		  "Diploma.Recommendation");
  conf_get_string(config, &style->recipient_name_font_path,
		  "Diploma.Recipient.NameFont");
  conf_get_string(config, &style->recipient_name_font_path,
		  "Diploma.RecipientName.Font");
  conf_get_string(config, &style->recipient_name, "Diploma.Recipient.Name");
  conf_get_string(config, &style->recipient_name, "Student.Name");
  conf_get_string(config, &style->recipient_alias_font_path,
		  "Diploma.Recipient.AliasFont");
  conf_get_string(config, &style->recipient_alias_font_path,
		  "Diploma.RecipientAlias.Font");
  conf_get_string(config, &style->recipient_alias, "Diploma.Recipient.Alias");
  conf_get_string(config, &style->recipient_alias, "Diploma.Recipient.Pseudo");
  conf_get_string(config, &style->recipient_alias, "Student.Alias");
  conf_get_string(config, &style->recipient_alias, "Student.Pseudo");
  conf_get_string(config, &style->recipient_alias, "Student.Nickname");
  conf_get_string(config, &style->recipient_birth_font_path,
		  "Diploma.Recipient.BirthFont");
  conf_get_string(config, &style->recipient_birth_font_path,
		  "Diploma.RecipientBirth.Font");
  conf_get_string(config, &style->recipient_birth_text,
		  "Diploma.Recipient.BirthText");
  conf_get_string(config, &style->recipient_birth_text,
		  "Student.BirthText");
  gender_recipient_birth_text(config, style);
  conf_get_string(config, &style->attribution_font_path,
		  "Diploma.Attribution.Font");
  conf_get_string(config, &style->attribution_text,
		  "Diploma.Attribution.Text");
  conf_get_string(config, &style->certification_font_path,
		  "Diploma.Certification.Font");
  conf_get_string(config, &style->certification_text,
		  "Diploma.Certification.Text");
  conf_get_string(config, &style->certification_text,
		  "Diploma.Legal.Text");
  conf_get_string(config, &style->signatory_label_font_path,
		  "Diploma.SignatoryFonts.LabelFont");
  conf_get_string(config, &style->signatory_name_font_path,
		  "Diploma.SignatoryFonts.NameFont");
  conf_get_string(config, &style->signatory_title_font_path,
		  "Diploma.SignatoryFonts.TitleFont");
  /* Backward compatibility with the earlier, invalid sample layout. */
  conf_get_string(config, &style->signatory_label_font_path,
		  "Diploma.Signatories.LabelFont");
  conf_get_string(config, &style->signatory_name_font_path,
		  "Diploma.Signatories.NameFont");
  conf_get_string(config, &style->signatory_title_font_path,
		  "Diploma.Signatories.TitleFont");
  load_signatories(config, style);
  conf_get_string(config, &style->diploma_number_font_path,
		  "Diploma.Footer.DiplomaNumber.Font");
  conf_get_string(config, &style->diploma_number_font_path,
		  "Diploma.Text.DiplomaNumber.Font");
  conf_get_string(config, &style->school_phrase_font_path,
		  "Diploma.Footer.SchoolPhrase.Font");
  conf_get_string(config, &style->school_phrase_font_path,
		  "Diploma.Text.SchoolPhrase.Font");
  conf_get_string(config, &style->school_phrase,
		  "Diploma.School.Characterization");
  conf_get_string(config, &style->school_phrase,
		  "School.Characterization");
  conf_get_string(config, &style->school_phrase,
		  "Diploma.Text.SchoolPhrase");
  conf_get_string_copy(config, style->diploma_number_text,
		       sizeof(style->diploma_number_text),
		       "Diploma.Text.DiplomaNumber");
  style->recipient_number = load_recipient_number(config);
  make_diploma_key(config, style, secret);
  if (style->diploma_number_text[0] == '\0'
      && style->promotion_year >= 0 && style->recipient_number >= 0)
    snprintf(style->diploma_number_text,
	     sizeof(style->diploma_number_text),
	     "%d-%05d-%s", style->promotion_year,
	     style->recipient_number, style->diploma_key_text);
  conf_get_color(config, &style->text_color, "Diploma.Text.Color");

  if (style->width <= 0)
    style->width = 3508;
  if (style->height <= 0)
    style->height = 2480;
  if (style->super_sampling <= 0)
    style->super_sampling = 1;
  if (style->super_sampling > 4)
    style->super_sampling = 4;
  if (style->border_margin < 0)
    style->border_margin = 0;

  if (style->border_min_horizontal_length < 1)
    style->border_min_horizontal_length = 1;
  if (style->border_max_horizontal_length < style->border_min_horizontal_length)
    style->border_max_horizontal_length = style->border_min_horizontal_length;

  if (style->border_min_vertical_length < 1)
    style->border_min_vertical_length = 1;
  if (style->border_max_vertical_length < style->border_min_vertical_length)
    style->border_max_vertical_length = style->border_min_vertical_length;

  if (style->border_min_vertical_length >= style->border_max_horizontal_length)
    style->border_min_vertical_length = style->border_max_horizontal_length > 1
      ? style->border_max_horizontal_length - 1
      : 1;
  if (style->border_max_vertical_length >= style->border_max_horizontal_length)
    style->border_max_vertical_length = style->border_max_horizontal_length > 1
      ? style->border_max_horizontal_length - 1
      : 1;
  if (style->border_max_vertical_length < style->border_min_vertical_length)
    style->border_max_vertical_length = style->border_min_vertical_length;

  if (style->border_max_thickness < style->border_min_thickness)
    style->border_max_thickness = style->border_min_thickness;
  if (style->border_max_lines < style->border_min_lines)
    style->border_max_lines = style->border_min_lines;
  if (style->border_min_lines < 1)
    style->border_min_lines = 1;
  if (style->border_max_lines > 12)
    style->border_max_lines = 12;
  if (style->border_line_spacing < 0)
    style->border_line_spacing = 0;

  if (style->border_glow_min_style < 0)
    style->border_glow_min_style = 0;
  if (style->border_glow_max_style < style->border_glow_min_style)
    style->border_glow_max_style = style->border_glow_min_style;
  if (style->border_glow_max_style > 8)
    style->border_glow_max_style = 8;

  if (style->botanical_margin < 0)
    style->botanical_margin = 0;
  if (style->botanical_min_scale < 1)
    style->botanical_min_scale = 1;
  if (style->botanical_max_scale < style->botanical_min_scale)
    style->botanical_max_scale = style->botanical_min_scale;
  if (style->botanical_min_depth < 0)
    style->botanical_min_depth = 0;
  if (style->botanical_max_depth < style->botanical_min_depth)
    style->botanical_max_depth = style->botanical_min_depth;
  if (style->botanical_max_depth > 8)
    style->botanical_max_depth = 8;
  if (style->botanical_min_thickness < 1)
    style->botanical_min_thickness = 1;
  if (style->botanical_max_thickness < style->botanical_min_thickness)
    style->botanical_max_thickness = style->botanical_min_thickness;
}

const char		*diploma_output_from_configuration(t_bunny_configuration *config,
						       const t_program_options *opt)
{
  const char		*output;

  if (opt->output_overridden)
    return (opt->output_file);
  if (conf_get_string(config, &output, "Diploma.Output"))
    return (output);
  if (conf_get_string(config, &output, "Output"))
    return (output);
  return (opt->output_file);
}

const char		*diploma_codename_from_configuration(t_bunny_configuration *config,
							 const char *fallback)
{
  const char		*codename;

  if (fallback != NULL)
    return (fallback);
  if (conf_get_string(config, &codename, "Diploma.Recipient.Codename"))
    return (codename);
  if (conf_get_string(config, &codename, "Recipient.Codename"))
    return (codename);
  if (conf_get_string(config, &codename, "Student.Codename"))
    return (codename);
  return ("");
}
