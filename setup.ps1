# Script d'installation automatisée pour Kite PiloteV3
# À exécuter en tant qu'administrateur

# Fonction pour vérifier si un programme est installé
function Test-CommandExists {
    param ($command)
    $oldPreference = $ErrorActionPreference
    $ErrorActionPreference = 'stop'
    try {
        if (Get-Command $command) { return $true }
    } catch { return $false }
    finally { $ErrorActionPreference = $oldPreference }
}

Write-Host "Installation de l'environnement Kite PiloteV3..." -ForegroundColor Green

# Vérifier si VS Code est installé
if (-not (Test-CommandExists "code")) {
    Write-Host "Installation de Visual Studio Code..." -ForegroundColor Yellow
    # Télécharger VS Code
    $vscodePath = "$env:TEMP\vscode_installer.exe"
    Invoke-WebRequest -Uri "https://code.visualstudio.com/sha/download?build=stable&os=win32-x64-user" -OutFile $vscodePath
    # Installer VS Code
    Start-Process -FilePath $vscodePath -Args "/VERYSILENT /NORESTART /MERGETASKS=!runcode" -Wait
    Remove-Item $vscodePath
}

# Vérifier si Git est installé
if (-not (Test-CommandExists "git")) {
    Write-Host "Installation de Git..." -ForegroundColor Yellow
    # Télécharger Git
    $gitPath = "$env:TEMP\git_installer.exe"
    Invoke-WebRequest -Uri "https://github.com/git-for-windows/git/releases/download/v2.41.0.windows.1/Git-2.41.0-64-bit.exe" -OutFile $gitPath
    # Installer Git
    Start-Process -FilePath $gitPath -Args "/VERYSILENT /NORESTART" -Wait
    Remove-Item $gitPath
}

# Configurer Git
git config --global user.name "moyamatthieu"
git config --global user.email "matthieu_moya@hotmail.fr"

# Installer les extensions VS Code requises
Write-Host "Installation des extensions VS Code..." -ForegroundColor Yellow
code --install-extension platformio.platformio-ide
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cpptools-extension-pack

# Cloner le projet si le dossier n'existe pas
$projectPath = ".\kite_PiloteV3"
if (-not (Test-Path $projectPath)) {
    Write-Host "Clonage du projet..." -ForegroundColor Yellow
    git clone https://github.com/moyamatthieu/kite_PiloteV3.git
}

# Ouvrir VS Code dans le dossier du projet
Write-Host "Ouverture du projet dans VS Code..." -ForegroundColor Yellow
code $projectPath

Write-Host "Installation terminée !" -ForegroundColor Green
Write-Host "Veuillez redémarrer VS Code pour que toutes les extensions soient activées." -ForegroundColor Yellow
Write-Host "Ensuite, ouvrez le fichier setup.md pour la suite des instructions." -ForegroundColor Yellow