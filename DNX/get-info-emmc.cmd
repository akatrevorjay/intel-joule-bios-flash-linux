:: Command file to get eMMC device info
::
:: %1 = DNX binary blob

DNX-dldrcli\dldrcli --command getcardinfo --fw_dnx "%~1" --device 2 --idx 0
