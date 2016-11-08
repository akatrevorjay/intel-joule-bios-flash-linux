:: Command file to clear the RPMB on eMMC device
::
:: %1 = DNX binary blob

DNX-dldrcli\dldrcli --command clearrpmb --fw_dnx "%~1" --device 2 --idx 0
