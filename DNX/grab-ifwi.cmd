:: Command file to download a DNX IFWI from eMMC
::
:: %1 = DNX-IFWI image
:: %2 = DNX binary blob

DNX-dldrcli\dldrcli --command readbootmedia --fw_dnx "%~2" --path "%~1.bp1" --device 2 --idx 0 --start 0 --blocks 4096 --part 0
DNX-dldrcli\dldrcli --command readbootmedia --fw_dnx "%~2" --path "%~1.bp2" --device 2 --idx 0 --start 0 --blocks 4096 --part 1
copy /b /y "%~1.bp1" + "%~1.bp2" "%~1.bin"
