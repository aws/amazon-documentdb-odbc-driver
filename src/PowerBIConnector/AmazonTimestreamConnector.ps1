$ZipFile = '.\AmazonTimestreamConnector.zip'
$MezFile = '.\AmazonTimestreamConnector.mez'

if (Test-Path -Path $ZipFile) {
    Write-Host Removing previous AmazonTimestreamConnector.zip

    Remove-Item $ZipFile
}

Write-Host Creating AmazonTimestreamConnector.m file

Copy-Item '.\AmazonTimestreamConnector\AmazonTimestreamConnector.pq' '.\AmazonTimestreamConnector\AmazonTimestreamConnector.m'

Write-Host Creating AmazonTimestreamConnector.zip file

compress-archive -path '.\AmazonTimestreamConnector\*.png', '.\AmazonTimestreamConnector\*.pqm', '.\AmazonTimestreamConnector\*.resx', '.\AmazonTimestreamConnector\AmazonTimestreamConnector.m' -destinationpath $ZipFile -update -compressionlevel optimal

if (Test-Path -Path '.\AmazonTimestreamConnector\AmazonTimestreamConnector.m') {
    Write-Host Removing AmazonTimestreamConnector.m

    Remove-Item '.\AmazonTimestreamConnector\AmazonTimestreamConnector.m'
}

if (Test-Path -Path $MezFile) {
    Write-Host Removing previous AmazonTimestreamConnector.mez

    Remove-Item $MezFile
}

Write-Host Creating AmazonTimestreamConnector.mez

Move-Item $ZipFile $MezFile

Write-Host done
