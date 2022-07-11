##################################################
# Assumptions
# 1. Chocolately (choco) is installed and on the path.
# 2. Environment variables DOC_DB_USER_NAME and DOC_DB_PASSWORD must be defined.
# 3. Running in PowerShell version 6 or greater.
##################################################

# Uninstall the mongodb server so we can reset the authorization.
choco uninstall mongodb mongodb.install -y
choco upgrade mongodb mongodb-shell mongodb-database-tools -y
Restart-Service MongoDB -Force

# By reinstalling, we've disabled authorization - so we can reset the authorization from scratch.
mongosh --quiet --eval "db.dropAllUsers()" "admin"
mongosh --quiet --eval "db.createUser({ user: '$env:DOC_DB_USER_NAME', pwd: '$env:DOC_DB_PASSWORD', roles: [ { role: 'root', db: 'admin' } ] })" "admin"
mongosh --quiet --eval "db.createUser({ user: 'docDbRestricted', pwd: '$env:DOC_DB_PASSWORD', roles: [ { role: 'readAnyDatabase', db: 'admin' } ] })" "admin"

# Get the path to the configuration file (requires PowerShell v6+)
$(Get-Service -Name MongoDB).BinaryPathName | Select-String -Pattern "--config\s+[""]([^""]+)[""]" | Foreach-Object { $env:MONGO_CONFIG_FILE = $_.Matches[0].Groups[1].Value }
if( -not [string]::IsNullOrEmpty($env:MONGO_CONFIG_FILE) ) {
    # This enables authorization - once service is restarted
    Add-Content -LiteralPath "$env:MONGO_CONFIG_FILE" -Value 'security:'
    Add-Content -LiteralPath "$env:MONGO_CONFIG_FILE" -Value '    authorization: "enabled"'
    Restart-Service MongoDB -Force
} else {
    Write-Host "MongoDB configuration file not found."
}
