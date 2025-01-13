instances=$(ls /home/evellyn/instances_sop3/*)

for f in $instances
do
  echo "Processing $f file..."
  # take action on each file. $f store current file name
  #./ajns "$f"
  ./build/ajns $f 3 0 > $f.output
done