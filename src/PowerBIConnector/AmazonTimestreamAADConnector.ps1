$ZipFile = '.\AmazonTimestreamAADConnector.zip'
$MezFile = '.\AmazonTimestreamAADConnector.mez'

if (Test-Path -Path $ZipFile) {
    Write-Host Removing previous AmazonTimestreamAADConnector.zip

    Remove-Item $ZipFile
}

Write-Host Creating AmazonTimestreamAADConnector.m file

Copy-Item '.\AmazonTimestreamAADConnector\AmazonTimestreamAADConnector.pq' '.\AmazonTimestreamAADConnector\AmazonTimestreamAADConnector.m'

Write-Host Creating AmazonTimestreamAADConnector.zip file

compress-archive -path '.\AmazonTimestreamAADConnector\*.png', '.\AmazonTimestreamAADConnector\*.pqm', '.\AmazonTimestreamAADConnector\*.resx', '.\AmazonTimestreamAADConnector\AmazonTimestreamAADConnector.m' -destinationpath $ZipFile -update -compressionlevel optimal

if (Test-Path -Path '.\AmazonTimestreamAADConnector\AmazonTimestreamAADConnector.m') {
    Write-Host Removing AmazonTimestreamAADConnector.m

    Remove-Item '.\AmazonTimestreamAADConnector\AmazonTimestreamAADConnector.m'
}

if (Test-Path -Path $MezFile) {
    Write-Host Removing previous AmazonTimestreamAADConnector.mez

    Remove-Item $MezFile
}

Write-Host Creating AmazonTimestreamAADConnector.mez

Move-Item $ZipFile $MezFile

Write-Host done
