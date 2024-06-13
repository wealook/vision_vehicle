# make-deps
A lightweight cross-platform c++ dependency builder

The make-deps will by default download and build in you system $HOME_DIR (user folder), If you don't want it to build there, you can specify the working folder by set the environment variable MAKE_DEPS_WORKING_DIR.


    git clone https://github.com/seiriosai/make-deps.git
    cd make-deps
    export MAKE_DEPS_WORKING_DIR=~  #optional, and you can alter the path
    python make-deps.py vs2019

