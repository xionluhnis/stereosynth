#!/usr/bin/env bash

while getopts g:o:s:r:t: OPT; do
  case $OPT in
  h)  echo "$USAGE"
      exit 0 ;;
  g)  GEOMETRY="-geometry $OPTARG" ;;
  o)  OUTPUT_FILE="$OPTARG" ;;
  r)  RESIZE="-resize $OPTARG" ;;
  s)  SIZE="-size $OPTARG" ;;
  t)  TILING="-tile $OPTARG" ;;
  \?) # invalid parameter
      echo "$USAGE" >&2
      exit 1 ;;
  esac
done
shift $((OPTIND-1))

[[ -z "$GEOMETRY" ]] && GEOMETRY="-geometry +0+0"
[[ -z "$OUTPUT_FILE" ]] && OUTPUT_FILE=output.png

IMAGES=$(echo $@ | tr " " "\n" | sort -R | tr "\n" " ")
montage -mode Concatenate -background none -gravity NorthWest $RESIZE $SIZE $TILING $GEOMETRY $IMAGES $OUTPUT_FILE
# montage -resize $TILE_SIZE -mode Concatenate -background none -gravity SouthWest -geometry $GEOMETRY -tile $TILING $IMAGES $OUTPUT_FILE
