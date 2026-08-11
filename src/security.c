// Jason Brillante "Damdoshi"
// Hanged Bunny Studio 2014-2026
// EFRITS SAS 2022-2026
// Pentacle Technologie 2008-2026
//
// GenDiplome

#include		<stdint.h>
#include		<string.h>
#include		<stdio.h>
#include		"program.h"

static uint64_t		rotl64(uint64_t x,
			       int b)
{
  return ((x << b) | (x >> (64 - b)));
}

static uint64_t		read_le64(const unsigned char *p)
{
  return (((uint64_t)p[0])
	  | ((uint64_t)p[1] << 8)
	  | ((uint64_t)p[2] << 16)
	  | ((uint64_t)p[3] << 24)
	  | ((uint64_t)p[4] << 32)
	  | ((uint64_t)p[5] << 40)
	  | ((uint64_t)p[6] << 48)
	  | ((uint64_t)p[7] << 56));
}

static uint64_t		fnv1a64_with_seed(const char *str,
					 uint64_t seed)
{
  const unsigned char	*s;

  s = (const unsigned char*)str;
  if (s == NULL)
    return (seed);
  while (*s != '\0')
    {
      seed ^= (uint64_t)*s;
      seed *= 1099511628211ULL;
      ++s;
    }
  return (seed);
}

static uint64_t		splitmix64_next(uint64_t *state)
{
  uint64_t		x;

  *state += 0x9E3779B97F4A7C15ULL;
  x = *state;
  x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
  x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
  return (x ^ (x >> 31));
}

static void		derive_siphash_key(const char *secret,
				   uint64_t *k0,
				   uint64_t *k1)
{
  uint64_t		seed;

  seed = 1469598103934665603ULL;
  seed = fnv1a64_with_seed("GenDiplome secret v1", seed);
  seed = fnv1a64_with_seed(secret, seed);
  *k0 = splitmix64_next(&seed);
  *k1 = splitmix64_next(&seed);
}

static uint64_t		siphash24(const void *data,
				  size_t len,
				  uint64_t k0,
				  uint64_t k1)
{
  const unsigned char	*in;
  uint64_t		v0;
  uint64_t		v1;
  uint64_t		v2;
  uint64_t		v3;
  uint64_t		m;
  uint64_t		b;
  size_t		i;
  int			r;

  in = (const unsigned char*)data;
  v0 = 0x736f6d6570736575ULL ^ k0;
  v1 = 0x646f72616e646f6dULL ^ k1;
  v2 = 0x6c7967656e657261ULL ^ k0;
  v3 = 0x7465646279746573ULL ^ k1;
  for (i = 0; i + 8 <= len; i += 8)
    {
      m = read_le64(in + i);
      v3 ^= m;
      for (r = 0; r < 2; ++r)
	{
	  v0 += v1; v1 = rotl64(v1, 13); v1 ^= v0; v0 = rotl64(v0, 32);
	  v2 += v3; v3 = rotl64(v3, 16); v3 ^= v2;
	  v0 += v3; v3 = rotl64(v3, 21); v3 ^= v0;
	  v2 += v1; v1 = rotl64(v1, 17); v1 ^= v2; v2 = rotl64(v2, 32);
	}
      v0 ^= m;
    }
  b = ((uint64_t)len) << 56;
  if (len - i >= 7)
    b |= ((uint64_t)in[i + 6]) << 48;
  if (len - i >= 6)
    b |= ((uint64_t)in[i + 5]) << 40;
  if (len - i >= 5)
    b |= ((uint64_t)in[i + 4]) << 32;
  if (len - i >= 4)
    b |= ((uint64_t)in[i + 3]) << 24;
  if (len - i >= 3)
    b |= ((uint64_t)in[i + 2]) << 16;
  if (len - i >= 2)
    b |= ((uint64_t)in[i + 1]) << 8;
  if (len - i >= 1)
    b |= ((uint64_t)in[i]);
  v3 ^= b;
  for (r = 0; r < 2; ++r)
    {
      v0 += v1; v1 = rotl64(v1, 13); v1 ^= v0; v0 = rotl64(v0, 32);
      v2 += v3; v3 = rotl64(v3, 16); v3 ^= v2;
      v0 += v3; v3 = rotl64(v3, 21); v3 ^= v0;
      v2 += v1; v1 = rotl64(v1, 17); v1 ^= v2; v2 = rotl64(v2, 32);
    }
  v0 ^= b;
  v2 ^= 0xff;
  for (r = 0; r < 4; ++r)
    {
      v0 += v1; v1 = rotl64(v1, 13); v1 ^= v0; v0 = rotl64(v0, 32);
      v2 += v3; v3 = rotl64(v3, 16); v3 ^= v2;
      v0 += v3; v3 = rotl64(v3, 21); v3 ^= v0;
      v2 += v1; v1 = rotl64(v1, 17); v1 ^= v2; v2 = rotl64(v2, 32);
    }
  return (v0 ^ v1 ^ v2 ^ v3);
}

bool			diploma_secret_is_disabled(const char *secret)
{
  return (secret == NULL || secret[0] == '\0' || !strcmp(secret, "none"));
}

uint64_t		diploma_keyed_hash64(const char *secret,
				    const char *purpose,
				    const char *payload)
{
  char			buf[4096];
  uint64_t		k0;
  uint64_t		k1;
  uint64_t		h;

  snprintf(buf, sizeof(buf), "%s#%s",
	   purpose != NULL ? purpose : "",
	   payload != NULL ? payload : "");
  if (diploma_secret_is_disabled(secret))
    {
      h = (uint64_t)bunny_hash(BH_DJB2, buf, strlen(buf));
      if (h == 0)
	h = 0x9E3779B97F4A7C15ULL;
      h ^= h >> 12;
      h ^= h << 25;
      h ^= h >> 27;
      return (h * 0x2545F4914F6CDD1DULL);
    }
  derive_siphash_key(secret, &k0, &k1);
  return (siphash24(buf, strlen(buf), k0, k1));
}

void			diploma_keyed_alnum8(char out[9],
				    const char *secret,
				    const char *purpose,
				    const char *payload)
{
  static const char	base[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  uint64_t		h;
  int			i;

  h = diploma_keyed_hash64(secret, purpose, payload);
  if (h == 0)
    h = 0xD1B54A32D192ED03ULL;
  for (i = 0; i < 8; ++i)
    {
      h += 0x9E3779B97F4A7C15ULL;
      h = (h ^ (h >> 30)) * 0xBF58476D1CE4E5B9ULL;
      h = (h ^ (h >> 27)) * 0x94D049BB133111EBULL;
      h = h ^ (h >> 31);
      out[i] = base[h % 36];
    }
  out[8] = '\0';
}
