Flashing the BIOS on an Intel Joule 570x from Linux
---------------------------------------------------

Intel says you need Windows to flash the BIOS on this guy.

The thing is, Intel included the tool needed `dldrcli` as a deb! They just didn't write any scripts around it or write
docs.

`flash.sh`: This is just a script that acts similarly to `Flash.cmd`, just for Linux instead.

Usage
-----

Connemt you're Joule and boot it into DNX mode by holding the DNX button down as you plug it in.
It's the one closest to the pins, separate from the other two.

Install `dnx.deb` / `dnx.rpm` / `dnx.tar.gz` accordingly.

Deb distros for example:

```sh
sudo dpkg -i dnx.deb
```

After that, just run `flash.sh` with the firmware you're flashing.
I have symlinked all required files to the root dir for ease of use.

```sh
./flash.sh Joule_C0-X64-Release-131-Public_DNX.bin
```

