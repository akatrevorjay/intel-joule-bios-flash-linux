@echo off
REM
REM Copyright (c) 2016 - 2017 Intel Corporation.
REM
REM Redistribution and use in source and binary forms, with or without
REM modification, are permitted provided that the following conditions
REM are met:
REM
REM * Redistributions of source code must retain the above copyright
REM notice, this list of conditions and the following disclaimer.
REM * Redistributions in binary form must reproduce the above copyright
REM notice, this list of conditions and the following disclaimer in
REM the documentation and/or other materials provided with the
REM distribution.
REM * Neither the name of Intel Corporation nor the names of its
REM contributors may be used to endorse or promote products derived
REM from this software without specific prior written permission.
REM
REM THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
REM "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
REM LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
REM A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
REM OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
REM SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
REM LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
REM DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
REM THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
REM (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
REM OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
REM
SetLocal EnableDelayedExpansion EnableExtensions

set BATCH_FILE_VERSION=1.1.15
set PROJECT_NAME=
set "TOOL_NAME=%~nx0"
echo ===============================================================================
echo  Intel(R) %PROJECT_NAME% %TOOL_NAME% version #%BATCH_FILE_VERSION%
echo -------------------------------------------------------------------------------

REM
REM Set default settings
REM
set CurrentDirectory=%CD%
if "%CurrentDirectory:~-1%" == "\" (
  set CurrentDirectory=%CurrentDirectory:~0,-1%
)
set "DebugLogPath=%~dp0Flash.log"
set DNX-binary-blob=DNXP_0x1
set "DnxPath=%~dp0"
set ExitCode=0

REM
REM Set default flags
REM
set ClearRpmbFlag=FALSE
set OldFlag=FALSE
set PpvFlag=FALSE
set ProdFlag=TRUE
set ResetFlag=FALSE
set ResetFlagNumber=6

REM
REM Sanity checks
REM
if ["%~1"] == [""] (
  echo ERROR: Missing Firmware Image file parameter^^^!
  goto ErrorExit
)

REM
REM Optional arguments
REM
:OptLoop
if /i "%~1"=="" goto Usage
if /i "%~1"=="/?" goto Usage
if /i "%~1"=="-?" goto Usage

if /i "%~1"=="-clear" (
  set ClearRpmbFlag=TRUE
  shift
  goto OptLoop
)

if /i "%~1"=="-noclear" (
  echo WARNING: -noclear switch has now been depracated.
  set ClearRpmbFlag=FALSE
  shift
  goto OptLoop
)

if /i "%~1"=="-noprod" (
  set ProdFlag=FALSE
  shift
  goto OptLoop
)

if /i "%~1"=="-old" (
  set OldFlag=TRUE
  shift
  goto OptLoop
)

if /i "%~1"=="-ppv" (
  echo WARNING: -ppv switch is only valid with builds prior to #183.
  echo          Builds #183 and beyond use no switches like the rest of the images.
  set PpvFlag=TRUE
  shift
  goto OptLoop
)

if /i "%~1"=="-prod" (
  echo WARNING: -prod switch has been deprecated.
  shift
  goto OptLoop
)

if /i "%~1"=="-reset" (
  set FlashTemp=%~2
  set ResetFlag=TRUE
  shift
  if "!FlashTemp:~0,1!" == "-" (
    echo WARNING: -reset flag number set to %ResetFlagNumber%.
  ) else (
    call :IsNumber !FlashTemp! FlashTempFlag
    if /i "!FlashTempFlag!" == "TRUE" (
      set ResetFlagNumber=!FlashTemp!
      shift
    )
  )
  set FlashTemp=
  set FlashTempFlag=
  goto OptLoop
)

REM
REM Figure out what DNX blob to use
REM
if /i "%PpvFlag%" == "TRUE" (
  set DNX-binary-blob=!DNX-binary-blob!-ppv
) else (
  if /i "%OldFlag%" == "TRUE" (
    set DNX-binary-blob=!DNX-binary-blob!-old
  )
  if /i "%ProdFlag%" == "TRUE" (
    set DNX-binary-blob=!DNX-binary-blob!-prod
  )
)
set DNX-binary-blob=%DNX-binary-blob%.bin

REM
REM Convert to relative firmware path
REM
call :RelativePath "%~1" FirmwareImagePath
if not "%FirmwareImagePath:~1,1%" == ":" (
  set FirmwareImagePath=%CurrentDirectory%\%FirmwareImagePath%
)
if not exist "%FirmwareImagePath%" (
  REM We're clueless. Give up.
  echo ERROR: Firmware Image file %FirmwareImagePath% does not exist^^^!
  goto ErrorExit
)

REM
REM Switch to the right folder than flash the system
REM
set ErrorCode=
if defined DnxPath pushd "%DnxPath%"
  REM
  REM Push out the Firmware image to the eMMC
  REM
  echo - Provisioning the eMMC...
  call provision-emmc.cmd "%FirmwareImagePath%" "%DNX-binary-blob%" > "%DebugLogPath%" 2>&1
  call :ExtractErrorCode "%DebugLogPath%" "ErrorCode"
  if "!ErrorCode!" == "0x80000036" (
    echo   - WARNING: Already provisioned^^^!
  ) else if not "!ErrorCode!" == "" (
    echo   - ERROR: Provisioning failed^^^!
    call :ExplainError "!ErrorCode!"
  )

  REM
  REM Test to see if we have a RPMB
  REM
  echo - Checking for RPMB...
  call dumprpmb-emmc.cmd "%DNX-binary-blob%" >> "%DebugLogPath%" 2>&1
  if ErrorLevel 1 (
    call :ExtractErrorCode "%DebugLogPath%" "ErrorCode"
    if not "!ErrorCode!" == "0x80000031" (
      echo   - ERROR: RPMB check failed^^^!
      call :ExplainError "!ErrorCode!"
    )
  ) else (
    echo   - WARNING: Unit has a RPMB area and will not support the Slim version of the CSE^^^!
    echo              This means if you are trying to flash a BIOS version greater then #010, it will fail^^^!
    echo              This issue can NOT be fixed^^^!
  )
  if exist rpmb.bin del /q /f rpmb.bin >nul 2>&1

  REM
  REM Push out the Firmware image to the eMMC
  REM
  echo - Downloading the BIOS...
  call download-ifwi.cmd "%FirmwareImagePath%" "%DNX-binary-blob%" >> "%DebugLogPath%" 2>&1
  if ErrorLevel 1 (
    call :ExtractErrorCode "%DebugLogPath%" "ErrorCode"
    echo   - ERROR: Downloading failed^^^!
    call :ExplainError "!ErrorCode!"
	goto ErrorExit
  )

  REM
  REM Clear the NvStorage area
  REM
  if /i "%clearRpmbFlag%" == "TRUE" (
    echo - Clearing NvStorage...
    call clearrpmb-emmc.cmd "%DNX-binary-blob%" >> "%DebugLogPath%" 2>&1
    if ErrorLevel 1 (
      call :ExtractErrorCode "%DebugLogPath%" "ErrorCode"
      echo   - ERROR: Clearing NvStorage failed^^^!
      call :ExplainError "!ErrorCode!"
      goto ErrorExit
    )
  )

  REM
  REM Reset the system
  REM
  if /i "%ResetFlag%" == "TRUE" (
    echo - Resetting the system...
    call reset-system.cmd "%ResetFlagNumber%" >> "%DebugLogPath%" 2>&1
    if ErrorLevel 1 (
      call :ExtractErrorCode "%DebugLogPath%" "ErrorCode"
      echo   - ERROR: Resetting the system failed^^^!
      call :ExplainError "!ErrorCode!"
      goto ErrorExit
    )
  )
if defined DnxPath popd
goto Exit

REM
REM Provide some usage info
REM
:Usage
  echo.
  echo %TOOL_NAME% [options] Firmware
  echo      -clear       Clear the RPMB area
  echo      -noclear     Do not clear the RPMB area
  echo      -noprod      Force use of non-production DNXP-0x1.bin
  echo      -old         Use CSE 1108 version of DNXP-0x1.bin (defaults to latest)
  echo      -ppv         Flash a PPV IFWI
  echo      -reset [#]   After flashing, reset the system using flag # (defaults to 6)
  echo.
  goto Exit

REM
REM Returns the relative path of %1 in %2
REM
REM Usage: call :RelativePath SrcVariable DstVariable
:RelativePath
  set input-path=%~1
  if defined input-path call set output-temp=%%input-path:*%cd%=%%
  if defined output-temp call set %2=%Output-temp%
  set input-path=
  set output-temp=
goto :EOF

REM
REM Returns the last error code found in the log file specified in %1 by setting it in %2
REM
REM Usage: call :ExtractErrorCode LogFilePath ReturnVariable
:ExtractErrorCode
  set log-path=%~1
  set temp-return=
  if exist "%log-path%" (
    for /f "usebackq tokens=*" %%l in ("%log-path%") do (
      set Line=%%l
      if not "!Line!" == "!Line:Error code: =!" (
        set temp-return=!Line:Error code: =!
      )
    )
    set Line=
  )
  call :ToLower !temp-return! %~2
  set log-path=
  set temp-return=
goto :EOF

REM
REM Convert string %1 to lower case and store in %2
REM
:ToLower
  if not "%~1" == "" (
    set ToLowerTemp=%~1
    for %%a in ("A=a" "B=b" "C=c" "D=d" "E=e" "F=f" "G=g" "H=h" "I=i"
                "J=j" "K=k" "L=l" "M=m" "N=n" "O=o" "P=p" "Q=q" "R=r"
                "S=s" "T=t" "U=u" "V=v" "W=w" "X=x" "Y=y" "Z=z" "Ä=ä"
                "Ö=ö" "Ü=ü") do (
        call set ToLowerTemp=!ToLowerTemp:%%~a!
    )
    set %~2=!ToLowerTemp!
    set ToLowerTemp=
  )
goto :EOF

REM
REM Is %1 a number? Boolean returned in %2
REM
:IsNumber
  if not "%~1" == "" (
    set IsNumberTemp=%~1
    for %%a in ("0=" "1=" "2=" "3=" "4=" "5=" "6=" "7=" "8=" "9=") do (
      if not "!IsNumberTemp!" == "" (
        call set IsNumberTemp=!IsNumberTemp:%%~a!
      )
    )
    if "!IsNumberTemp!" == "" (
      set %~2=TRUE
    ) else (
      set %~2=FALSE
    )
    set IsNumberTemp=
  )
goto :EOF

REM
REM Prints out explanation of ErrorCode passed in %1
REM
REM Usage: call :ExplainError ErrorCode
:ExplainError
  if not "%~1" == "" (
    echo      - %~1
    if "%~1" == "0x30000" (
      echo        - basic_ios::clear
      echo          Usually caused by a missing file.
      echo          Be sure you passed in the complete file name and the file exists.
      echo          File names containing a space must be surrounded by double quotes.
    ) else if "%~1" == "0x30003" (
      echo        - Device not found.
      echo          Be sure that the unit is in DnX mode and the OS has enuremated it.
      echo          Also check that the USB cable is seated completely.
    ) else if "%~1" == "0x20000007" (
      echo        - Internal system error.
      echo          This is usually caused mismatched DnX SW and DnX driver.
      echo          Try installing the DnX driver that came with this IFWI release.
    ) else if "%~1" == "0x80000000" (
      echo        - Media not present.
      echo          Hard strap choosen boot device is not present.
    ) else if "%~1" == "0x80000008" (
      echo        - Invalid public key.
      echo          The DnX module [DNXP_0x1*.bin] is signed with the wrong key.
      echo          This usually happens when trying to load a debug signed image onto
      echo          production hardware.
      echo          Try adding the -prod switch.
    ) else if "%~1" == "0x80000021" (
      echo        - Invalid partition header.
      echo          This means that the DnX module [DNXP_0x1*.bin] you are trying to flash
      echo          has an invalid header.
      echo          Try matching the FIT with the DnX module [DNXP_0x1*.bin].
      echo          If trying to flash a PPV image, try the -ppv switch.
    ) else if "%~1" == "0x8000002e" (
      echo        - Invalid partition header.
      echo          If you get this error when running a data clear command, it means
      echo          there's no GPP4 and no RPMB partitions on the flash, so there's no
      echo          data to be cleared.
    ) else if "%~1" == "0x80000031" (
      echo        - RPMB not provisioned.
      echo          If you get this error when trying to read data from RPMB it means
      echo          that the RPMB isn't provisioned.
    ) else if "%~1" == "0x80000035" (
      echo        - Image and descriptor mismatch.
      echo          This happens when trying to flash a SPI IFWI that has different
      echo          regions than the IFWI already flashed on the device. This flow isn't
      echo          supported. SPI DnX only supports flashing IFWIs that have the exact
      echo          same regions as the already flashed IFWI.
    ) else if "%~1" == "0x80000036" (
      echo        - Already configured.
      echo          - eMMC has already been partitioned.
    ) else if "%~1" == "0x80000039" (
      echo        - Invalid regions.
      echo          This means your DnX IFWI doesn't match the NVM used. Usually this
      echo          happens when hard straps are set to one NVM [e.g. SPI] and trying to
      echo          flash an IFWI that is for a different NVM [e.g. EMMC].
    ) else if "%~1" == "0x8000003a" (
      echo        - Unsupported storage device.
      echo          Flash initialization failed. Check that your flash is connected
      echo          properly to your platform, all the hard straps are set correctly and
      echo          you have all the needed reworks.
    ) else if "%~1" == "0x80000043" (
      echo        - Invalid image layout.
      echo          This means that the DnX IFWI you are trying to flash has an invalid
      echo          layout. This happens when using a DNX module that supports a new DnX
      echo          IFWI layout and trying to flash a DNX IFWI with an older layout. If
      echo          you see this error - match the versions of FIT used to generate the
      echo          DnX IFWI and the DnX module [DNXP_0x1*.bin].
    ) else if "%~1" == "0x80000045" (
      echo        - No DnX IFWI key in OEM key manifest.
      echo          This means that the DnX IFWI created doesn't support DnX since it
      echo          misses the DnX IFWI usage in the OEM key manifest. The DnX IFWI usage
      echo          needs to be added to the OEM key manifest in the IFWI.
    ) else (
      echo        - Unknown error. See %DebugLogPath% for more details.
    )
  )
goto :EOF

:ErrorExit
set ExitCode=1
if defined DnxPath popd

:Exit
echo ===============================================================================
(
  EndLocal
  exit /b %ExitCode%
)
