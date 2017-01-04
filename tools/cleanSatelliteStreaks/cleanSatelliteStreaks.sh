#!/bin/bash

if [ -z "$1" ]; then
  echo "<Usage>: $0 <field_id>"
  exit 1
fi

# Use the field_id

field=$1

mkdir -p $field
obsTool --detections $field > $field/$field.out
astroclean outtype 1 linesupport 10 file $field/$field.out clean_file $field/$field.clean noise_file $field/$field.noise

DETECTIONS=`cut -f1 $field/$field.noise -d' ' | paste -sd ","`

echo ""
echo "Now control the results with:"
echo "mitiPlot --autoscale $field/$field.out" 
echo "mitiPlot --autoscale $field/$field.clean"
echo "mitiPlot --autoscale $field/$field.noise"
echo "... and then, if happy, run:"
cat >/dev/stdout <<EOF
echo "UPDATE detections SET status='I' WHERE det_id IN ($DETECTIONS)" | mopsql
EOF

