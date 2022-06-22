
$scriptPath = Split-Path -parent $PSCommandPath
$projectPath = Split-Path -parent $scriptPath

# ODBC SDK
# CMake

# Java SDK
$jdkFound = $false
if ( -not ([string]::IsNullOrEmpty($Env:JAVA_HOME)) `
	-and (Test-Path -Path $Env:JAVA_HOME) `
	-and (Test-Path -Path ${Env:JAVA_HOME}/include) ) {

	# Test Java in path and has minimum version
	$regex = "\d+[.]\d+[.]\d+"
	$javaVersionString = $(java --version)
	$javaVersion = $($javaVersionString | Select-String -Pattern $regex | % { $_.Matches } | % { $_.Value } | Get-Unique -AsString)
	$regexDigit = "^\d+"
	$javaVersionArray = $($javaVersion | Select-String -Pattern $regexDigit -AllMatches | % { $_.Matches } | % { $_.Value } )
	if ( ($javaVersionArray.Count -ge 1) -and -not ([string]::IsNullOrEmpty($javaVersionArray[0])) ) {
		if ( "1" -eq $javaVersionArray[0] -and -not ([string]::IsNullOrEmpty($javaVersionArray[1])) ) {
			if ( $javaVersionArray[1] -eq "8" -or $javaVersionArray[1] -eq "9" ) {
				$jdkFound = $true
			}
		} else if ( $javaVersionArray[0] -ge "10") {
			$jdkFound = $true
		}
	}
}
if ( -not $jdkFound ) {
	$jdksFolder     = "$env:USERPROFILE/.jdks"
	$jdkZipFileName = 'amazon-corretto-17-x64-windows-jdk.zip'
	$jdkDownloadUri = "https://corretto.aws/downloads/latest/$jdkZipFileName"
	$jdkZipFilePath = "$jdksFolder/$jdkZipFileName"

	Invoke-WebRequest $jdkDownloadUri -OutFile $jdkZipFilePath

	$tempFolderPath = Join-Path $Env:Temp $(New-Guid)
	$tempFolder = New-Item -Type Directory -Path $tempFolderPath

	#Extract the zip file 
	Expand-Archive -Path $jdkZipFilePath $tempFolderPath

	# Copy the JDK zip file to C:\Program Files\Java
	Copy-Item -Path '.\*.zip' -Destination $ExpandArchiveDestinationPath -force

	#Delete the zip file after extraction
	Remove-Item $jdkZipFilePath -Force
}

# Example of how to permanently set an environment variable for a user...
#[System.Environment]::SetEnvironmentVariable('GoHugo','C:\Hugo\bin',[System.EnvironmentVariableTarget]::User)

# VCPKG (Windows)
if ( -not ([string]::IsNullOrEmpty($Env:VCPKG_ROOT)) ) {
	$Env:VCPKG_ROOT = "c:/vcpkg"
}
if ( -not ((Test-Path -Path $Env:VCPKG_ROOT)) ) {
	New-Item $Env:VCPKG_ROOT -ItemType Directory
	cd $Env:VCPKG_ROOT
	git clone https://github.com/Bit-Quill/amazon-documentdb-odbc-driver-mirror.git
	./vcpkg integrate install
} else {
	cd $Env:VCPKG_ROOT
	git pull
	./vcpkg integrate install
}

# OpenSSL
cd $Env:VCPKG_ROOT
./vcpkg install openssl:x64-windows

# Boost SDK
cd $Env:VCPKG_ROOT
./vcpkg install `
	boost-test:x64-windows `
	boost-asio:x64-windows `
	boost-chrono:x64-windows `
	boost-interprocess:x64-windows `
	boost-regex:x64-windows `
	boost-system:x64-windows `
	boost-thread:x64-windows

# Mongo SDK
cd $Env:VCPKG_ROOT
./vcpkg install mongo-cxx-driver:x64-windows `

# Mongo Server
./src/odbc-test/scripts/reinstall_mongodb.ps1

# WIX (Windows)
