virtual disk system demo

Usage:

1. Display file system information
    ./diskinfo test.img

2. Displays the contents of the root directory
    ./disklist test.img /
output:

   Displays the contents of a given sub directory
    ./disklist test.img /sub_dir

3. Copies a file from the file system to the current directory in Linux.
  ./diskget test.img /sub_dir/foo2.txt foo.txt

And there is an image in the file system, to copy it.
  ./diskget test.img /sub_dir/aa.jpg pic.jpg

4. Copies a file from the current Linux directory into the file system.
  If the sub-directory does not exist, it will create a new sub-directory
    ./diskput test.img foo.txt /sub_dir/foo3.txt
