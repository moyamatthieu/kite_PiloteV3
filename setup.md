# Guide d'installation - Kite PiloteV3

Ce guide explique comment configurer l'environnement de développement complet pour le projet Kite PiloteV3.

## Prérequis logiciels

1. **VS Code**
   - Télécharger et installer VS Code depuis https://code.visualstudio.com/
   - Version recommandée : dernière version stable

2. **Extensions VS Code requises**
   - PlatformIO IDE (`platformio.ide`)
   - C/C++ (`ms-vscode.cpptools`)
   - C/C++ Extension Pack (`ms-vscode.cpptools-extension-pack`)

3. **Git**
   - Télécharger et installer Git depuis https://git-scm.com/
   - Configuration minimale :
     ```bash
     git config --global user.name "moyamatthieu"
     git config --global user.email "matthieu_moya@hotmail.fr"
     ```

## Configuration du projet

1. **Cloner le projet**
   ```bash
   git clone https://github.com/moyamatthieu/kite_PiloteV3.git
   cd kite_PiloteV3
   ```

2. **Configuration PlatformIO**
   - Plateforme : ESP32 (espressif32)
   - Board : ESP32 Dev Module
   - Framework : Arduino

3. **Dépendances matérielles**
   - ESP32 Dev Module
   - LCD I2C 20x4
   - 4 boutons poussoirs
   - 3 potentiomètres
   - IMU MPU6050
   - Servomoteurs
   - Module WiFi intégré à l'ESP32

## Dépendances logicielles

Les dépendances sont gérées automatiquement par PlatformIO dans `platformio.ini` :

- ElegantOTA v3.1.7 (mises à jour OTA)
- LiquidCrystal_I2C v1.1.4 (affichage LCD)
- ESPAsyncWebServer (serveur web asynchrone)
- AsyncTCP (communication TCP asynchrone)
- ESP32Servo (contrôle des servomoteurs)
- FastAccelStepper (contrôle moteurs pas à pas)

## Configuration de l'environnement

1. **Ouvrir le projet**
   - Lancer VS Code
   - File > Open Folder > Sélectionner le dossier kite_PiloteV3
   - Attendre que PlatformIO initialise le projet

2. **Vérifier l'installation**
   ```bash
   pio run --target clean
   pio run
   ```

3. **Configuration spécifique**
   - Fréquence CPU : 240MHz
   - Fréquence Flash : 80MHz
   - Mode Flash : DIO
   - Vitesse moniteur série : 115200 baud
   - Vitesse upload : 921600 baud

## Options de build

Les options de build sont préconfigurées dans `platformio.ini` avec :
- Optimisations mémoire activées
- Niveau de debug ESP32 minimal
- Tracking heap FreeRTOS activé
- Niveau de log INFO (3)
- Optimisation pour la taille du code (-Os)

## Structure des dossiers

La structure du projet est organisée comme suit :
```
kite_PiloteV3/
├── include/          # En-têtes C++
├── src/             # Sources C++
├── test/            # Tests
├── data/            # Fichiers web
└── docs-fr/         # Documentation
```

## Configuration IDE recommandée

Les paramètres VS Code sont préconfigurés dans `.vscode/settings.json` :
- Langue : Français
- Association de fichiers C++
- Configuration du formateur de code

## Problèmes connus et solutions

1. **Erreur de connexion série**
   - Vérifier les permissions du port série
   - Utiliser `sudo usermod -a -G dialout $USER` sur Linux

2. **Erreurs de build**
   - Voir `logs/build_errors.md` pour les solutions courantes

## Scripts utiles

Des tâches VS Code sont configurées dans `.vscode/tasks.json` pour :
- Build et upload
- Gestion Git
- Tests

## Sauvegarde

Pour sauvegarder la configuration :
1. Les fichiers de configuration sont versionés dans Git
2. Le fichier `platformio.ini` contient toutes les dépendances
3. `.vscode/` contient la configuration de l'IDE

## Support

En cas de problème :
1. Consulter `docs-fr/`
2. Vérifier les logs dans `logs/`
3. Ouvrir une issue sur GitHub