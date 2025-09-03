# How to build?

```console
cd ~
git clone https://github.com/kernelginar/cppp ~/cppp
cd ~/cppp
mkdir build
make
```

# How to use cppp (cp++)

```console
Usage: cppp [OPTIONS]

Options:
  -m, --mode           Specify the operation mode. (required)
  -i, --input          Source file or directory path. (required)
  -o, --output         Destination file or directory path. (required)
  -p, --parts          Specify the number of parts for copying.
      --insert-after   Insert data after the specified position.
      --select         Select specific parts for "select-merge" mode.
  -c, --disable-sha256 Disable verifying SHA-256 checksum after copying.
  -h, --help           Display this help message.
  -V, --version        Show program version information.

Notes:
- '--input', and '--output' parameters are required.
- The '--parts' option is specifies the number of chunks to split the input files into during copying. Default: 1
```

# Some examples

```console
cppp -i existing_dir existing_file -o non_existing_dir -p 4
# Result: non_existing_dir/{existing_dir/[files...],existing_file}

cppp -i existing_dir -o existing_another_dir -p 20
# Result: existing_another_dir/existing_dir[files...]

cppp -i existing_dir -o non_existing_name -p 20
# Result: non_existing_name/[existing_dir_files...]
```
