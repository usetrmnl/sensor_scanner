#! /usr/bin/env bash
# save the current directory
  pushd .
cd ~
mkdir Projects
cd Projects
git clone https://github.com/bitbank2/bb_scd41
cd bb_scd41/Linux
make
sudo apt install -y i2c-tools libi2c-dev
sudo raspi-config nonint do_i2c 0

# restore the original directory
  popd
make

