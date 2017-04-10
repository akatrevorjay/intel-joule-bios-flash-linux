:: Command file to dump the RPMB on eMMC device
::
:: %1 = DNX binary blob

DNX-dldrcli\dldrcli.exe --command readbootmedia --fw_dnx "%~1" --path rpmb.bin --device 2 --idx 0 --start 0 --blocks 4096 --part 16