birschick-bq/ad-781/dev-setup
$scriptPath = Split-Path -parent $PSCommandPath
$projectPath = Split-Path -parent $scriptPath

# ODBC SDK
# CMake

# Java SDK
Write-Output "Checking for dependency JDK"
$jdkFound = $false
if ( -not ([string]::IsNullOrEmpty($Env:JAVA_HOME)) `
	-and (Test-Path -Path $Env:JAVA_HOME) `
	-and (Test-Path -Path "${Env:JAVA_HOME}\include") ) {

	# Test Java in path and has minimum version
	$regex = "\d+[.]\d+[.]\d+"
	$javaVersionString = $(java --version)
	$javaVersion = $($javaVersionString | Select-String -Pattern $regex | % { $_.Matches } | % { $_.Value } | Get-Unique -AsString)
	$regexDigit = "\d+"
	$javaVersionArray = $($javaVersion | Select-String -Pattern $regexDigit -AllMatches | % { $_.Matches } | % { $_.Value } )
	if ( ($javaVersionArray.Count -ge 1) -and -not ([string]::IsNullOrEmpty($javaVersionArray[0])) ) {
		if ( "1" -eq $javaVersionArray[0] -and -not ([string]::IsNullOrEmpty($javaVersionArray[1])) ) {
			if ( $javaVersionArray[1] -eq "8" -or $javaVersionArray[1] -eq "9" ) {
				$jdkFound = $true
			}
		} elseif ( $javaVersionArray[0] -ge "10") {
			$jdkFound = $true
		}
	}
}
if ( -not $jdkFound ) {
	Write-Output "JDK not found. Installing new version"
	$jdksFolder     = "$env:USERPROFILE\.jdks"
	$jdkZipFileName = 'amazon-corretto-17-x64-windows-jdk.zip'
	$jdkDownloadUri = "https://corretto.aws/downloads/latest/$jdkZipFileName"
	$jdkZipFilePath = "$jdksFolder\$jdkZipFileName"

	Invoke-WebRequest $jdkDownloadUri -OutFile $jdkZipFilePath

	$tempFolderPath = Join-Path $Env:Temp $(New-Guid)
	New-Item -Type Directory -Path $tempFolderPath

	#Extract the zip file 
	Expand-Archive -Path $jdkZipFilePath $tempFolderPath


	# Copy the JDK zip file to C:\Program Files\Java
	$contentsOfTempFolder = Get-ChildItem -Path $tempFolderPath -Name
	if ( $contentsOfTempFolder.Count -eq 1 ) {
		$jdkFolderName = $contentsOfTempFolder[0]
		Copy-Item -Path "$tempFolderPath\*" -Destination $jdksFolder -force
		$Env:JAVA_HOME = "$jdksFolder\$jdkFolderName"
		# permanently set an environment variable for user...
		[System.Environment]::SetEnvironmentVariable('JAVA_HOME', $Env:JAVA_HOME, [System.EnvironmentVariableTarget]::User)
		# Update path for bin and server (jvm.dll)
	}
	#Delete the zip file after extraction
	Remove-Item $jdkZipFilePath -Force
	Remove-Item $tempFolderPath -Recurse -Force
}
Write-Output "Checking for dependency JDK complete"


# VCPKG (Windows)
if ( [string]::IsNullOrEmpty($Env:VCPKG_ROOT) ) {
	$Env:VCPKG_ROOT = "c:/vcpkg"
}
if ( -not ((Test-Path -Path $Env:VCPKG_ROOT)) ) {
	New-Item $Env:VCPKG_ROOT -ItemType Directory -Force
	cd $Env:VCPKG_ROOT
	git clone https://github.com/Bit-Quill/amazon-documentdb-odbc-driver-mirror.git
	./vcpkg integrate install
} else {
	cd $Env:VCPKG_ROOT
	git pull
	.\vcpkg.exe integrate install
}

# OpenSSL
cd $Env:VCPKG_ROOT
.\vcpkg.exe install openssl:x64-windows

# Boost SDK
cd $Env:VCPKG_ROOT
.\vcpkg.exe install `
	boost-test:x64-windows `
	boost-asio:x64-windows `
	boost-chrono:x64-windows `
	boost-interprocess:x64-windows `
	boost-regex:x64-windows `
	boost-system:x64-windows `
	boost-thread:x64-windows

# Mongo SDK
cd $Env:VCPKG_ROOT
.\vcpkg.exe install mongo-cxx-driver:x64-windows

# Mongo Server
cd $projectPath
./src/odbc-test/scripts/reinstall_mongodb.ps1

# WIX (Windows)
