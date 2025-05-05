# Liste des erreurs de build - 5 mai 2025

## Erreurs identifiées

### 1. Erreur dans task_manager.h
- Problème : Fichier `../include/kite_webserver.h` introuvable
- Localisation : `include/core/task_manager.h`
- Solution proposée : 
  - Vérifier le chemin d'inclusion correct vers `kite_webserver.h`
  - Le fichier devrait être dans `include/communication/kite_webserver.h`

### 2. Erreurs dans autopilot.cpp
- Problème : Structure `IMUData` incorrecte
- Localisation : `src/control/autopilot.cpp`
- Détails : Les membres `gyroX`, `gyroY`, `gyroZ` n'existent pas dans la structure
- Solution proposée : 
  - Utiliser `rollRate`, `pitchRate`, `yawRate` à la place
  - Vérifier la définition de la structure dans `include/hardware/sensors/imu.h`

### 3. Erreurs de redéfinition de macros
- Problème : Redéfinition de `MEMORY_OPTIMIZATION_ENABLED`
- Localisation : Multiple fichiers dont `include/core/config.h`
- Solution proposée :
  - Ajouter des gardes de redéfinition (#ifndef)
  - Unifier les définitions dans un seul fichier

### 4. Problème avec Wire.h
- Problème : `TwoWire` et `Wire` non reconnus
- Localisation : `include/hardware/sensors/imu.h`
- Solution proposée :
  - Ajouter l'inclusion de `<Wire.h>`
  - Vérifier la configuration de PlatformIO pour l'ESP32

### 5. Problèmes de chemins d'inclusion
- Problème : Plusieurs chemins d'inclusion incorrects
- Localisation : Multiples fichiers
- Solution proposée :
  - Standardiser les chemins d'inclusion relatifs
  - Utiliser des chemins relatifs depuis la racine du projet

## Structure de dépendances identifiée
```
core/
  ├── config.h         # Dépendances globales
  ├── task_manager.h   # Dépend de display.h, kite_webserver.h
  └── system.h         # Dépend de config.h

hardware/
  └── sensors/
      └── imu.h        # Dépend de Wire.h (Arduino)

control/
  └── autopilot.h      # Dépend de imu.h
```

## Prochaines étapes
1. Corriger les chemins d'inclusion dans task_manager.h
2. Mettre à jour la structure IMUData dans autopilot.cpp
3. Résoudre les conflits de redéfinition de macros
4. Ajouter les inclusions manquantes pour Wire.h
5. Standardiser tous les chemins d'inclusion