Flashing the BIOS on an Intel Joule 570x from Linux
---------------------------------------------------

Intel says you need Windows to flash the BIOS on this guy.

The thing is, Intel included the tool needed `dldrcli` as a deb! They just didn't write any scripts around it or write
docs.

I took all the logic in their Windows batch script and rewrote it for bash. It's much cleaner if you ask me, but then again I'm biased as hell here.

It's located at `flash.sh`, take a look!


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
./flash.sh Joule_C0-X64-Release-1D1-Public_DNX.bin
```

If you are unable to found `dldrcli` when runing the `./flash.sh`, you can edit `~/.bashrc` and add the following to the end of file.

```sh
export PATH=$PATH:/opt/bxt_dnx_tool/bin
```

Flashing an older BIOS
----------------------

I've kept a git tag for each version I've used as I've upgraded, as well as kept an `./archives` folder more recently.

IOW, Extract away or simply `git checkout`. Choice is yours.
