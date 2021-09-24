$ZipFile = '.\TimestreamConnector.zip'
$MezFile = '.\TimestreamConnector.mez'

if (Test-Path -Path $ZipFile) {
    Write-Host Removing previous TimestreamConnector.zip

    Remove-Item $ZipFile
}

Write-Host Creating TimestreamConnector.m file

Copy-Item '.\TimestreamConnector\TimestreamConnector.pq' '.\TimestreamConnector\TimestreamConnector.m'

Write-Host Creating TimestreamConnector.zip file

compress-archive -path '.\TimestreamConnector\*.png', '.\TimestreamConnector\*.pqm', '.\TimestreamConnector\*.resx', '.\TimestreamConnector\TimestreamConnector.m' -destinationpath $ZipFile -update -compressionlevel optimal

if (Test-Path -Path '.\TimestreamConnector\TimestreamConnector.m') {
    Write-Host Removing TimestreamConnector.m

    Remove-Item '.\TimestreamConnector\TimestreamConnector.m'
}

if (Test-Path -Path $MezFile) {
    Write-Host Removing previous TimestreamConnector.mez

    Remove-Item $MezFile
}

Write-Host Creating TimestreamConnector.mez

Move-Item $ZipFile $MezFile

Write-Host done
