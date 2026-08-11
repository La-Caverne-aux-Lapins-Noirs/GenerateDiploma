// Jason Brillante "Damdoshi"
// Hanged Bunny Studio 2014-2026
// EFRITS SAS 2022-2026
// Pentacle Technologie 2008-2026
//
// GenDiplome

#include		"program.h"

double			clamp(double				v)
{
  if (v < 0.0)
    return (0.0);
  if (v > 1.0)
    return (1.0);
  return (v);
}

