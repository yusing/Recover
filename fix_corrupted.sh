#!/bin/bash
cd found
# fix corrupted documents
for file in *.* ; do
    echo "fixing $file"
    zip -FF $file --out "${file}.fixed" -q
    echo "unzipping $file"
    unzip $file -d "$file.unpacked"
    echo "repacking $file"
    cd "$file.unpacked"
    zip -r "../fixed_${file%.unpacked}" .
    cd ..
done
rm ./*.fixed
rm -rf ./*.unpacked