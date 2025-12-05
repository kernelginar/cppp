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
  -m, --mode           Specify the operation mode. (optional, default copy)
  -i, --input <path>   Source file or directory path. (optional)
  -o, --output <path>  Destination file or directory path. (optional)
  -p, --parts          Specify the number of parts for copying.
  -c, --check-sha256   Enable verifying SHA-256 checksum after copying.
  -h, --help           Display this help message.
  -v, --verbose        Enable verbose logging.
  -f, --force          Enable overwriting on existing file.
  -V, --version        Show program version information.
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
