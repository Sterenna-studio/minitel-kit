#!/bin/sh
# Cherche dans le depot les valeurs qui ne doivent jamais etre publiees.
# Les motifs (une expression reguliere etendue par ligne) sont dans
# .motifs-prives, a la racine, gitignore : la liste elle-meme est privee.
# Sortie vide = rien de trouve. A lancer avant chaque push.
cd "$(dirname "$0")/.." || exit 2
[ -f .motifs-prives ] || { echo ".motifs-prives absent" >&2; exit 2; }
# Motifs generiques, publiables : adresses privees, chemins Windows, ports COM.
generiques='192\.168\.|10\.[0-9]+\.[0-9]+\.[0-9]+|172\.(1[6-9]|2[0-9]|3[01])\.|[A-Z]:\\|COM[0-9]+'
grep -rnIE --exclude-dir=.git --exclude=.motifs-prives --exclude=verifier-prive.sh \
  -e "$generiques" -f .motifs-prives . && exit 1
exit 0
