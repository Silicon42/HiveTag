# HiveTag
Circular black and white fiducial markers of my own design. Intended for object tracking. 

# Cloning This Repo
This repo uses git submodules. To get the full source, you must either clone with
```git clone --recurse-submodules https://github.com/Silicon42/HiveTag```
or after cloning/downloading normally use
```git submodule update --init --recursive```

# TO-DO List
+ Fix gen_tags.c to use the new marker style and hex id naming scheme
+ consider new single dummy bit layout for 6 symbol markers

## Acknowledgements
+ croxton https://gist.github.com/croxton/7690270?short_path=ee7206f
++ I used his sample code for how to create svgs that can have parameters passed
 to them via query strings