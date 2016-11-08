:: Command file to provision GPP partitions on the eMMC device
::
:: %1 = DNX-IFWI image
:: %2 = DNX binary blob

DNX-dldrcli\dldrcli --command configpart --fw_dnx "%~2" --path dnx-dldrcli\cfgpart.xml --device 2 --idx 0 "%~1"
