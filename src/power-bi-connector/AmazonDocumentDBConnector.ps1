$ZipFile = '.\AmazonDocumentDBConnector.zip'
$MezFile = '.\AmazonDocumentDBConnector.mez'

if (Test-Path -Path $ZipFile) {
    Write-Host Removing previous AmazonDocumentDBConnector.zip

    Remove-Item $ZipFile
}

Write-Host Creating AmazonDocumentDBConnector.m file

Copy-Item '.\AmazonDocumentDBConnector\AmazonDocumentDBConnector.pq' '.\AmazonDocumentDBConnector\AmazonDocumentDBConnector.m'

Write-Host Creating AmazonDocumentDBConnector.zip file

compress-archive -path '.\AmazonDocumentDBConnector\*.png', '.\AmazonDocumentDBConnector\*.pqm', '.\AmazonDocumentDBConnector\*.resx', '.\AmazonDocumentDBConnector\AmazonDocumentDBConnector.m' -destinationpath $ZipFile -update -compressionlevel optimal

if (Test-Path -Path '.\AmazonDocumentDBConnector\AmazonDocumentDBConnector.m') {
    Write-Host Removing AmazonDocumentDBConnector.m

    Remove-Item '.\AmazonDocumentDBConnector\AmazonDocumentDBConnector.m'
}

if (Test-Path -Path $MezFile) {
    Write-Host Removing previous AmazonDocumentDBConnector.mez

    Remove-Item $MezFile
}

Write-Host Creating AmazonDocumentDBConnector.mez

Move-Item $ZipFile $MezFile

Write-Host done
