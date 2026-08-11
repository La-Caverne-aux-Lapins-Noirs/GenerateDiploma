# Format du fichier Dabsic de GenDiplome

Ce document décrit le format de configuration utilisé par **GenDiplome**.

- le moteur lit un ou plusieurs fichiers Dabsic passés sur la ligne de commande ;
- les fichiers sont fusionnés dans l'ordre ;
- le secret de sécurité **n'est pas** stocké dans le Dabsic : il doit être fourni par `--secret`.

Exemple minimal :

```sh
./gendiploma default.dab --secret 'mon-secret-long-et-prive' -o diplome.png
```

Pour une génération volontairement non sécurisée :

```sh
./gendiploma default.dab --secret none -o exemple.png
```

---

## 1. Structure générale

Le scope racine est généralement :

```dabsic
[Diploma
  ...
]
```

Les sous-scopes les plus utiles sont :

- `Diploma`
- `Diploma.Border`
- `Diploma.Botanical`
- `Diploma.Logo`
- `Diploma.Font`
- `Diploma.Promotion`
- `Diploma.School`
- `Diploma.Recommendation`
- `Diploma.Recipient`
- `Diploma.Attribution`
- `Diploma.Text`
- `Diploma.Certification`
- `Diploma.Seal`
- `Diploma.SignatoryFonts`
- `Diploma.Signatories` (tableau)
- `Diploma.Footer`

Le fichier `default.dab` fourni à la racine du dépôt est un exemple complet et neutre.

---


## 1 bis. Fichier d'overlay étudiant

Le fichier `default_student.dab` est un exemple de **petit fichier d'override** destiné à être fusionné avec `default.dab`.

Exemple d'usage :

```sh
./gendiploma default.dab default_student.dab --secret 'mon-secret-long-et-prive' -o student.png
```

Dans ce mode :

- `default.dab` porte l'identité visuelle et les textes institutionnels ;
- `default_student.dab` porte les informations variables de l'étudiant.

Les champs les plus naturels à placer dans un overlay étudiant sont :

- `Diploma.Output`
- `Diploma.Promotion.Year`
- `Diploma.Recipient.Promo`
- `Diploma.Recipient.Number`
- `Diploma.Recipient.Codename`
- `Diploma.Recipient.Name`
- `Diploma.Recipient.Alias`
- `Diploma.Recipient.BirthText`
- le premier bloc de `Diploma.Signatories` pour adapter `Le titulaire` / `La titulaire` et le nom affiché.

---

## 2. Valeurs globales

### `Diploma`

| Champ | Type | Rôle |
|---|---:|---|
| `Width` | entier | largeur finale du diplôme |
| `Height` | entier | hauteur finale du diplôme |
| `SuperSampling` | entier | facteur de suréchantillonnage avant réduction |
| `Background` | couleur | couleur de fond du diplôme |
| `Output` | chaîne | fichier de sortie par défaut si `-o` n'est pas utilisé |

### Couleurs

Les couleurs peuvent être données en entier ou sous forme hexadécimale :

- `#RRGGBB`
- `#RRGGBBAA`

Exemple :

```dabsic
Background = "#101419"
Color = "#FFFFFFE6"
```

---

## 3. Bordure

### `Diploma.Border`

| Champ | Type | Rôle |
|---|---:|---|
| `Color` | couleur | couleur du liseret |
| `Margin` | entier | marge intérieure du cadre |
| `MinHorizontalLength` | entier | longueur min des segments horizontaux |
| `MaxHorizontalLength` | entier | longueur max des segments horizontaux |
| `MinVerticalLength` | entier | longueur min des segments verticaux |
| `MaxVerticalLength` | entier | longueur max des segments verticaux |
| `MinThickness` | entier | épaisseur min |
| `MaxThickness` | entier | épaisseur max |
| `MinLines` | entier | nombre min de cadres répétés |
| `MaxLines` | entier | nombre max de cadres répétés |
| `LineSpacing` | entier | espacement entre cadres |
| `GlowMinStyle` | entier | style de glow min |
| `GlowMaxStyle` | entier | style de glow max |

---

## 4. Motif botanique

### `Diploma.Botanical`

| Champ | Type | Rôle |
|---|---:|---|
| `Enabled` | booléen | active/désactive le motif botanique |
| `Color` | couleur | couleur du motif |
| `Margin` | entier | marge du motif |
| `MinScale` / `MaxScale` | entier | échelle min/max |
| `MinDepth` / `MaxDepth` | entier | profondeur récursive min/max |
| `MinThickness` / `MaxThickness` | entier | épaisseur min/max |

---

## 5. Assets

### `Diploma.Logo`

| Champ | Type | Rôle |
|---|---:|---|
| `Path` ou `File` | chaîne | chemin du logo |

### `Diploma.Seal`

| Champ | Type | Rôle |
|---|---:|---|
| `Path` ou `File` | chaîne | chemin du sceau |

### `Diploma.Font`

| Champ | Type | Rôle |
|---|---:|---|
| `Path` ou `File` | chaîne | fichier `.dab` de la police du titre principal |

> Les autres blocs texte ont chacun leur propre fichier `.dab` de police.

---

## 6. Textes principaux

### `Diploma.Text`

| Champ | Type | Rôle |
|---|---:|---|
| `Main` | chaîne | titre principal du diplôme |
| `First` | chaîne | alias accepté pour `Main` |
| `Color` | couleur | actuellement informatif pour certains styles ; la couleur effective vient surtout des `.dab` |

`Main` peut contenir `\n` pour forcer un retour à la ligne.

Exemple :

```dabsic
[Text
  Main = "Diplôme d'études\nen technologies numériques"
]
```

### `Diploma.Promotion`

| Champ | Type | Rôle |
|---|---:|---|
| `Year` | entier | année de promotion canonique |
| `Text` | chaîne | texte personnalisé ; si `Year` est présent, le moteur reconstruit `Promotion XXXX` |
| `Font` | chaîne | `.dab` utilisé pour ce texte |

### `Diploma.School`

| Champ | Type | Rôle |
|---|---:|---|
| `Name` | chaîne | nom long de l'établissement |
| `NameFont` | chaîne | `.dab` du nom de l'établissement |
| `Characterization` | chaîne | phrase courte en bas à droite |

### `Diploma.Recommendation`

| Champ | Type | Rôle |
|---|---:|---|
| `Font` | chaîne | `.dab` utilisé |
| `Text` | chaîne | phrase de recommandation |

### `Diploma.Attribution`

| Champ | Type | Rôle |
|---|---:|---|
| `Font` | chaîne | `.dab` utilisé |
| `Text` | chaîne | texte d'introduction avant le titre |

### `Diploma.Certification`

| Champ | Type | Rôle |
|---|---:|---|
| `Font` | chaîne | `.dab` utilisé |
| `Text` | chaîne | paragraphe juridique / descriptif |

---

## 7. Destinataire du diplôme

### `Diploma.Recipient`

| Champ | Type | Rôle |
|---|---:|---|
| `Name` | chaîne | nom civil affiché |
| `NameFont` | chaîne | `.dab` du nom |
| `Alias` | chaîne | pseudo optionnel affiché sous le nom |
| `AliasFont` | chaîne | `.dab` du pseudo |
| `BirthText` | chaîne | texte de naissance |
| `BirthFont` | chaîne | `.dab` du texte de naissance |
| `Promo` / `Promotion` | entier | source de repli pour l'année de promo |
| `Number` / `Id` / `ID` | entier | numéro étudiant |
| `Codename` | chaîne | codename de l'étudiant, utilisé pour la génération procédurale |

Le moteur accepte aussi certaines clés `Student.*` comme alias de lecture (`Student.Name`, `Student.Promo`, `Student.Id`, `Student.Codename`, etc.).

### Numéro du diplôme

Si `Diploma.Text.DiplomaNumber` n'est pas fourni, le moteur génère automatiquement une valeur de forme :

```text
2030-00001-AB12CD34
```

avec :

- l'année de promotion,
- le numéro étudiant sur 5 chiffres,
- une clé alphanumérique courte sur 8 caractères.

---

## 8. Signataires

### Polices des signataires

#### `Diploma.SignatoryFonts`

| Champ | Type | Rôle |
|---|---:|---|
| `LabelFont` | chaîne | `.dab` pour la première ligne (statut) |
| `NameFont` | chaîne | `.dab` pour la seconde ligne (identité) |
| `TitleFont` | chaîne | `.dab` utilisé selon la mise en page interne |

### Tableau `Diploma.Signatories`

Les signataires sont fournis sous forme de tableau Dabsic :

```dabsic
{Signatories
  [
    Label = "Le titulaire"
    Name = "Prénom NOM"
    Title = ""
    Signature = ""
  ],
  [
    Label = ""
    Name = "Prénom NOM"
    Title = "Directeur général"
    Signature = ""
  ]
}
```

Champs disponibles par entrée :

| Champ | Type | Rôle |
|---|---:|---|
| `Label` | chaîne | première ligne / statut |
| `Role` | chaîne | alias accepté pour `Label` |
| `Name` | chaîne | identité |
| `Title` | chaîne | titre / fonction |
| `Signature` / `SignaturePath` | chaîne | image de signature |

---

## 9. Footer

### `Diploma.Footer.DiplomaNumber`

| Champ | Type | Rôle |
|---|---:|---|
| `Font` | chaîne | `.dab` utilisé pour le code du diplôme |

### `Diploma.Footer.SchoolPhrase`

| Champ | Type | Rôle |
|---|---:|---|
| `Font` | chaîne | `.dab` utilisé pour la phrase de caractérisation |

---

## 10. Ligne de commande

### Fichiers Dabsic

Le programme accepte un ou plusieurs fichiers Dabsic :

```sh
./gendiploma default.dab eleve.dab --secret 'mon-secret' -o sortie.png
```

Les fichiers sont chargés dans l'ordre.

### Options CLI

| Option | Rôle |
|---|---|
| `-o`, `--output <fichier>` | chemin de sortie |
| `-n`, `--codename <codename>` | surcharge du codename |
| `--secret <valeur>` | secret obligatoire |

Le secret est **obligatoire**. Pour désactiver explicitement la sécurité :

```sh
./gendiploma default.dab --secret none -o test.png
```

---

## 11. Conseils pour un dépôt open source

Pour publier GenDiplome sans publier l'identité visuelle réelle d'un établissement :

- publier le moteur ;
- publier un `default.dab` neutre ;
- garder privé le vrai `.dab` de l'établissement ;
- garder privé le logo réel, les signatures et le sceau ;
- garder privé le secret passé à `--secret`.

C'est exactement l'objectif du fichier `default.dab` fourni ici.
