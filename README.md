# GenDiplome

Générateur procédural de fond de diplôme.

## Assets attendus

- `res/placeholder_logo.png` sert de logo temporaire.
- `res/font_diplome.dab` configure la `t_bunny_font` utilisée pour le texte principal.
- Copie le vrai fichier TTF dans `res/font_diplome.ttf` avant génération.

Le Dabsic principal référence la configuration de font via :

```dabsic
[Font
  Path = "res/font_diplome.dab"
]
```

## Secret de génération

La génération authentifiée exige un secret fourni explicitement en ligne de
commande. Il n'est pas lu depuis le Dabsic et ne doit pas être stocké dans le
dépôt public.

Exemple :

```sh
./gendiploma efrits.dab --secret 'phrase-secrete-longue-et-privee' -o diplome.png
```

Pour générer volontairement un exemple reproductible sans protection :

```sh
./gendiploma efrits.dab --secret none -o exemple.png
```

Avec un vrai secret, les variations procédurales et la clé courte du diplôme
sont dérivées d'un hachage SipHash-2-4 clé. Sans le secret, on peut compiler le
moteur, mais pas reproduire exactement un diplôme émis avec la clé de l'école.

## Exemple générique et documentation

- `default.dab` fournit un exemple générique et minimaliste, sans identité EFRITS.
- `docs/FORMAT.md` documente les champs Dabsic pris en charge par GenDiplome.

Exemple :

```sh
./gendiploma default.dab --secret 'mon-secret-long-et-prive' -o default.png
```

## Overlay étudiant

- `default_student.dab` fournit un exemple d'overlay étudiant minimal.
- Il est pensé pour être utilisé **en plus** de `default.dab`.

Exemple :

```sh
./gendiploma default.dab default_student.dab --secret 'mon-secret-long-et-prive' -o student.png
```
