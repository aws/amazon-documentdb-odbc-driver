#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

$powerShellVersion = $PSVersionTable.PSVersion.Major
$scriptPath = Split-Path -parent $PSCommandPath
$projectPath = Split-Path -parent $scriptPath

function Confirm-PowerShellVersion {
	[OutputType([Boolean])]
	Param()

	# Validate that we have the correct version of PowerShell
	if ( [string]::IsNullOrEmpty($powerShellVersion) ) {
		Write-Host "Unable to determine PowerShell version"
		return $false
	}
	if ( $powerShellVersion -lt 7 ) {
		Write-Host "PowerShell version must be version 7 or greater. Found version: $powerShellVersion"
		return $false
	}
	return $true
}

function Confirm-RunAsAdministrator {
	[OutputType([Boolean])]
	Param()

	$currentPrincipal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
	if ( -not ($currentPrincipal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator) ) ) {
		return $false
	}
	Write-Host "Running as administrator."
	return $true
}

function Invoke-RunAsAdministrator {
	[OutputType([Boolean])]
	Param(
		[Parameter(Mandatory=$true, ValueFromPipeline=$false)]
		[string]$CommandPath,
		[Parameter(Mandatory=$false, ValueFromPipeline=$false)]
		[string]$Arguments,
		[switch]$Wait
	)

	if ( -not $(Confirm-PowerShellVersion) ){
		return $false
	}

	if ( -not $(Confirm-RunAsAdministrator) ) {
		# Elevate the command
		if ([int](Get-CimInstance -Class Win32_OperatingSystem | Select-Object -ExpandProperty BuildNumber) -ge 6000) {
			Write-Host "Starting command `"$CommandPath`" in new elevated process."
			if ($Wait) {
				Start-Process -Wait -FilePath $CommandPath -Verb Runas -ArgumentList "$Arguments"
			} 
			else {
				Start-Process -FilePath $CommandPath -Verb Runas -ArgumentList "$Arguments"
			}
			return $true
		}
	}
	else {
		
	}

	return $false
}

function Invoke-ScriptRunAsAdministrator {
	[OutputType([Boolean])]
	Param(
		[Parameter(Mandatory=$true, ValueFromPipeline=$false)]
		[string]$CommandPath,
		[Parameter(Mandatory=$false, ValueFromPipeline=$false)]
		[string]$Arguments,
		[switch]$Wait,
		[switch]$NoExit
	)

	if ( -not $(Confirm-PowerShellVersion) ){
		return $false
	}

	# Self-elevate the script if required
	if ([int](Get-CimInstance -Class Win32_OperatingSystem | Select-Object -ExpandProperty BuildNumber) -ge 6000) {
		$CommandArguments = "-File `"" + $CommandPath + "`" " + $Arguments
		if ($NoExit) {
			$CommandArguments = "-noe $CommandArguments"
		}
		if ($Wait) {
			Invoke-RunAsAdministrator -Wait "pwsh.exe" $CommandArguments
		} 
		else {
			Invoke-RunAsAdministrator -Wait $CommandArguments
		}
		return $true
	}

	return $false
}

function Enable-VsDevShell {
	[OutputType([Boolean])]
	Param()

	# Enable VsDevShell explicitly
	$vsInstance = $(Get-CimInstance MSFT_VSInstance)
	$devShellLocation = $(Get-ChildItem -Path $vsInstance.InstallLocation -Filter "Microsoft.VisualStudio.DevShell.dll" -Recurse -ErrorAction SilentlyContinue -Force)
	if ( ([string]::IsNullOrEmpty($devShellLocation)) ) {
		Write-Host "Unable to locate Visual Studio DevShell."
		return $false
	}
	Import-Module $devShellLocation
	Enter-VsDevShell $vsInstance.IdentifyingNumber
	Write-Host "Entered Visual Studio DevShell."
	return $true
}

function Confirm-CmakeExists {
	[OutputType([Boolean])]
	Param()

	# Test that we have access to CMake - i.e., Visual Studio dev shell.
	$cmakeVersion = $(cmake --version)
	if ( [string]::IsNullOrEmpty($cmakeVersion) ) {
		Write-Host "Unable to find 'cmake'. "
		Write-Host "Please ensure Visual Studio and C++ tools are installed."
		return $false
	}
	Write-Host "Valid CMake found. Version: $cmakeVersion"
	return $true
}

# ODBC SDK

function Confirm-JavaJdk {
	[OutputType([Boolean])]
	param ()

	# Java SDK
	Write-Host "Checking for dependency JDK"
	$jdkFound = $false
	# Checks that JAVA_HOME is define, path exists and is a JDK (i.e., include subdirectory)
	if ( -not ([string]::IsNullOrEmpty($Env:JAVA_HOME)) `
		-and (Test-Path -Path $Env:JAVA_HOME) `
		-and (Test-Path -Path "${Env:JAVA_HOME}\include") ) {

		# Test Java in path and has minimum version
		$regex = "\d+[.]\d+[.]\d+"
		try {
			$javaVersionString = $(java --version)
			$javaVersion = $($javaVersionString | Select-String -Pattern $regex | ForEach-Object { $_.Matches } | ForEach-Object { $_.Value } | Get-Unique -AsString)
			$javaVersionArray = $javaVersion.Split(".")
			if ( ($javaVersionArray.Count -ge 1) -and -not ([string]::IsNullOrEmpty($javaVersionArray[0])) ) {
				if ( "1" -eq $javaVersionArray[0] -and -not ([string]::IsNullOrEmpty($javaVersionArray[1])) ) {
					if ( $javaVersionArray[1] -eq "8" -or $javaVersionArray[1] -eq "9" ) {
						$jdkFound = $true
					}
				} elseif ( $javaVersionArray[0] -ge "10") {
					$jdkFound = $true
				}
			}
			if ( $jdkFound ) {
				Set-JdkPath
			}
		}
		catch {
			Write-Host $_
		}
	}

	return $jdkFound
}

function Install-JavaJdk {
	[OutputType([Boolean])]
	param ()
	
	Write-Host "Installing Java JDK"
	$jdksFolder     = "$env:USERPROFILE\.jdks"
	$jdkZipFileName = 'amazon-corretto-17-x64-windows-jdk.zip'
	$jdkDownloadUri = "https://corretto.aws/downloads/latest/$jdkZipFileName"
	$jdkZipFilePath = "$jdksFolder\$jdkZipFileName"
	$tempFolderPath = Join-Path $Env:Temp $(New-Guid)

	try {
		# Download the JDK
		Invoke-WebRequest $jdkDownloadUri -OutFile $jdkZipFilePath
		#Extract the zip file 
		New-Item -Type Directory -Path $tempFolderPath
		Expand-Archive -Path $jdkZipFilePath $tempFolderPath

		# Ensure unique folder
		$contentsOfTempFolder = Get-ChildItem -Path $tempFolderPath -Directory -Name
		if ( $contentsOfTempFolder.Count -ne 1 ) {
			Write-Host "Unable to insall Java JDK. Unexpected distribution does not have unique folder."
			return $false
		}
		# Place JDK in user's folder
		$jdkFolderName = $contentsOfTempFolder
		$Env:JAVA_HOME = "$jdksFolder\$jdkFolderName"
		if ( -not (Test-Path -Path $Env:JAVA_HOME) ) {
			Copy-Item -Path "$tempFolderPath\*" -Destination $jdksFolder -Recurse -Force
		}

		# Update PATH for bin (java.exe) and server (jvm.dll)
		Set-JdkPath
	}
	catch {
		Write-Host "Error installing Java JDK."
		Write-Host $_
		return $false
	}
	finally {
		#Delete the zip file after extraction
		Remove-Item $jdkZipFilePath -Force
		Remove-Item $tempFolderPath -Recurse -Force
	}
	Write-Host "Finished installing Java JDK"
	return $true
}

function Set-JdkPath {
	[OutputType([Boolean])]
	param ()

	$updatePath = $false

	# Update PATH for bin (java.exe) and server (jvm.dll)
	$javaFile = Get-ChildItem -Path $Env:JAVA_HOME -Recurse -Filter "java.exe"
	$binPath = $javaFile.Directory
	$jvmFile = Get-ChildItem -Path $Env:JAVA_HOME -Recurse -Filter "jvm.dll"
	$jvmPath = $jvmFile.Directory

	$paths = $Env:Path.Split(";")
	if ( -not $paths.Contains($jvmPath) ) {
		$Env:Path = "${jvmPath};${Env:Path}"
		$updatePath = $true
	}
	$paths = $Env:Path.Split(";")
	if ( -not $paths.Contains($binPath) ) {
		$Env:Path = "${binPath};${Env:Path}"
		$updatePath = $true
	}

	# permanently set an environment variable for user...
	[System.Environment]::SetEnvironmentVariable('JAVA_HOME', $Env:JAVA_HOME, [System.EnvironmentVariableTarget]::User)
	if ( $updatePath ) {
		[System.Environment]::SetEnvironmentVariable('PATH', $Env:Path, [System.EnvironmentVariableTarget]::User)
	}
}

function Install-Vcpkg {
	[OutputType([Boolean])]
	Param()

	# VCPKG (Windows)
	if ( [string]::IsNullOrEmpty($Env:VCPKG_ROOT) ) {
		$Env:VCPKG_ROOT = "c:\vcpkg"
		[System.Environment]::SetEnvironmentVariable('VCPKG_ROOT', $Env:VCPKG_ROOT, [System.EnvironmentVariableTarget]::User)
		Write-Host "Setting environment variable VCPKG_ROOT=$Env:VCPKG_ROOT"
	}
	if ( -not ((Test-Path -Path $Env:VCPKG_ROOT)) ) {
		Write-Host "Cloning VCPKG GitHub repository into $Env:VCPKG_ROOT"
		New-Item $Env:VCPKG_ROOT -ItemType Directory -Force
		Set-Location $Env:VCPKG_ROOT
		git clone https://github.com/microsoft/vcpkg.git $Env:VCPKG_ROOT
		.\bootstrap-vcpkg | Write-Host
		.\vcpkg.exe integrate install | Write-Host
	} else {
		Write-Host "Updating VCPKG GitHub local repository in $Env:VCPKG_ROOT"
		Set-Location $Env:VCPKG_ROOT
		git pull
		.\vcpkg.exe integrate install | Write-Host
	}
	return $true
}

function Install-VcpkgPackages {
	[OutputType([Boolean])]
	Param()

	# OpenSSL
	Set-Location $Env:VCPKG_ROOT
	.\vcpkg.exe install openssl:x64-windows | Write-Host
	if ($LASTEXITCODE -ne 0) {
		Write-Host "Unable to install openssl library"
		return $false
	}

	# Boost SDK
	.\vcpkg.exe install `
		boost-test:x64-windows `
		boost-asio:x64-windows `
		boost-chrono:x64-windows `
		boost-interprocess:x64-windows `
		boost-regex:x64-windows `
		boost-system:x64-windows `
		boost-thread:x64-windows | Write-Host
	if ($LASTEXITCODE -ne 0) {
		Write-Host "Unable to install openssl library"
		return $false
	}

	# Mongo SDK
	.\vcpkg.exe install mongo-cxx-driver:x64-windows | Write-Host
	if ($LASTEXITCODE -ne 0) {
		Write-Host "Unable to install openssl library"
		return $false
	}

	return $true
}

function Install-MongoDb {
	[OutputType([Boolean])]
	Param()

	# Install/upgrade Mongo Server
	if ( -not $(Confirm-RunAsAdministrator) ) {
		Invoke-ScriptRunAsAdministrator "$projectPath\scripts\reinstall_mongodb.ps1" -Wait
	}
	else {
		"$projectPath\scripts\reinstall_mongodb.ps1"
	}

	return $true
}

function Install-WixToolset {
	[OutputType([Boolean])]
	Param()

	if ( -not $(Confirm-RunAsAdministrator) ) {
		Invoke-RunAsAdministrator -Wait "choco" "upgrade wixtoolset -y"
	}
	else {
		choco upgrade wixtoolset -y
	}
	return $true
}
