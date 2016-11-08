:: Command file to download a Broxton DNX IFWI to eMMC
::
:: %1 = DNX-IFWI image
:: %2 = DNX binary blob

DNX-dldrcli\dldrcli --command downloadfwos  --fw_dnx "%~2" --device emmc --idx 0 --fw_image "%~1"
