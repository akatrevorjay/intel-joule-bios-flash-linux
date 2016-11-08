@echo off
REM
REM Copyright (c) 2016 Intel Corporation.
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

set BATCH_FILE_VERSION=1.1.5
set PROJECT_NAME=Joule
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
set "DebugLogPath=%~dp0\Flash.log"
set DNX-binary-blob=DNXP_0x1
set "DnxPath=%~dp0"
set ExitCode=0
set ProdFlag=TRUE
set OldFlag=FALSE

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

if /i "%~1"=="-prod" (
  echo WARNING: -prod switch has been deprecated.
  shift
  goto OptLoop
)

REM
REM Figure out what DNX blob to use
REM
if /i "%OldFlag%" == "TRUE" (
  set DNX-binary-blob=%DNX-binary-blob%-old
)
if /i "%ProdFlag%" == "TRUE" (
  set DNX-binary-blob=%DNX-binary-blob%-prod
)
set DNX-binary-blob=%DNX-binary-blob%.bin

REM
REM Convert to relative firmware path
REM
call :RelativePath "%~1" FirmwareImagePath
set FirmwareImagePath=%CurrentDirectory%\%FirmwareImagePath%
if not exist "%FirmwareImagePath%" (
  REM We're clueless. Give up.
  echo ERROR: Firmware Image file %FirmwareImagePath% does not exist^^^!
  goto ErrorExit
)

REM
REM Switch to the right folder than flash the system
REM
if defined DnxPath pushd %DnxPath%
  REM
  REM Push out the Firmware image to the eMMC
  REM
  echo - Provisioning the eMMC...
  call provision-bxt-emmc.cmd "%FirmwareImagePath%" "%DNX-binary-blob%" > %DebugLogPath% 2>&1
  if ErrorLevel 1 (
    echo   - WARNING: Provisioning failed^^^!
    echo              Probably already provisioned.
  )

  REM
  REM Test to see if we have a RPMB
  REM
  echo - Checking for RPMB...
  call dumprpmb-bxt-emmc.cmd "%DNX-binary-blob%" >> %DebugLogPath% 2>&1
  if not ErrorLevel 1 (
    echo WARNING: Unit has a RPMB area and will not support the Slim version of the CSE^^^!
    echo          This means if you are trying to flash a BIOS version greater then #010, it will fail^^^!
    echo          This issue can NOT be fixed^^^!
  )
  if exist rpmb.bin del /q /f rpmb.bin >nul 2>&1

  REM
  REM Push out the Firmware image to the eMMC
  REM
  echo - Downloading the BIOS...
  call download-bxt-ifwi.cmd "%FirmwareImagePath%" "%DNX-binary-blob%" >> %DebugLogPath% 2>&1
  if ErrorLevel 1 (
    echo ERROR: Downloading failed^^^!
	goto ErrorExit
  )

  REM
  REM Clear the NvStorage area
  REM
  echo - Clearing NvStorage...
  call clearrpmb-bxt-emmc.cmd "%DNX-binary-blob%" >> %DebugLogPath% 2>&1
  if ErrorLevel 1 (
    echo ERROR: Clearing NvStorage failed^^^!
	goto ErrorExit
  )
if defined DnxPath popd
goto Exit

REM
REM Provide some usage info
REM
:Usage
  echo.
  echo %TOOL_NAME% [options] Firmware
  echo      -noprod Force use of non-production DNXP-0x1.bin
  echo      -old    Use CSE 1108 version of DNXP-0x1.bin (defaults to latest)
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

:ErrorExit
set ExitCode=1
popd

:Exit
echo ===============================================================================
(
  EndLocal
  exit /b %ExitCode%
)
