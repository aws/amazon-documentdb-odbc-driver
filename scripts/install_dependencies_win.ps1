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

# Include functions that perform checks and installations.
$currentPath = Get-Location
$scriptPath = Split-Path -parent $PSCommandPath
. $scriptPath\functions_win.ps1

##################################################################################
# Confirm and install missing dependencies
##################################################################################

try {
	if ( -not $(Confirm-PowerShellVersion) ) {
		return 1
	}
	elseif ( -not $(Enable-VsDevShell) ) {
		return 1
	}
	elseif ( -not $(Confirm-CmakeExists) ) {
		return 1
	}
	elseif ( -not $(Confirm-JavaJdk) ) {
		if ( -not $(Install-JavaJdk) ) {
			return 1
		}
	}
	elseif ( -not $(Install-Vcpkg) ) {
		return 1
	}
	$result = $(Install-VcpkgPackages)
	if ( -not $result ) {
		return 1
	}
	elseif ( -not $(Install-WixToolset) ) {
		return 1
	}
	elseif ( -not $(Install-MongoDb) ) {
		return 1
	}

	Write-Host "Installation completed."
	return
}
finally {
	Set-Location $currentPath
}
