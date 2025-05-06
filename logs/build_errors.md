# Journal des erreurs, corrections et orientations - Kite PiloteV3

## Choix et orientations de code

### Principes d'architecture
- **Architecture modulaire** : Organisation du code en modules indépendants avec des interfaces bien définies
- **Séparation des préoccupations** : Distinction claire entre les fonctionnalités matérielles, la logique métier et l'interface utilisateur
- **Approche orientée objet** : Utilisation de classes pour encapsuler les fonctionnalités et les états

### Stratégies de centralisation et modularité
- **Configuration centralisée** : Toutes les constantes de configuration sont regroupées dans `config.h`
- **Définitions de broches centralisées** : Toutes les définitions de broches hardware sont dans `config.h` pour éviter les conflits
- **Variables locales aux modules** : Les variables d'état internes restent encapsulées dans leurs modules respectifs
- **Interfaces standardisées** : Chaque module expose une API cohérente pour faciliter l'intégration

### Pratiques de codage
- **Documentation des fonctions** : Chaque fonction est documentée avec sa description, ses paramètres et sa valeur de retour
- **Gestion des erreurs cohérente** : Les fonctions d'initialisation retournent systématiquement un statut de succès/échec
- **Structures de données explicites** : Utilisation de structures et d'énumérations pour améliorer la lisibilité
- **Constantes nommées** : Utilisation de #define avec des noms explicites plutôt que des valeurs littérales

### Optimisations
- **Gestion de la mémoire** : Attention particulière à l'utilisation de la mémoire sur ESP32
- **Paramètres configurables** : Possibilité d'ajuster les paramètres de performance dans `config.h`
- **Multitâche FreeRTOS** : Utilisation du multitâche pour améliorer la réactivité du système

## Mise à jour du 6 mai 2025

### Implémentations manquantes corrigées

#### 1. Implémentation des fonctions dans DisplayManager
- **Problème** : Plusieurs fonctions de la classe `DisplayManager` n'étaient pas implémentées
- **Fichiers concernés** : `src/hardware/io/display_manager.cpp`
- **Solution** : Implémentation complète de toutes les méthodes de la classe DisplayManager, dont :
  - `setupI2C()`
  - `initLCD()`
  - `createCustomChars()`
  - `centerText()`
  - `displayMessage()`
  - et autres fonctions d'affichage

#### 2. Implémentation des fonctions système
- **Problème** : Plusieurs fonctions système référencées dans le code étaient manquantes
- **Fichiers concernés** : `src/core/system.cpp`
- **Solution** : Implémentation des fonctions système manquantes :
  - `systemRestart()`
  - `isSystemHealthy()`
  - `getSystemStatusString()`
  - `feedWatchdogs()`
  - `systemErrorToString()`

#### 3. Implémentation de WiFiManager
- **Problème** : Classe `WiFiManager` incomplète mais référencée dans le code
- **Fichiers concernés** : 
  - `include/communication/wifi_manager.h`
  - `src/communication/wifi_manager.cpp`
- **Solution** : Implémentation complète de l'interface WiFiManager et de ses méthodes :
  - `begin()`
  - `isConnected()`
  - `reconnect()`
  - Fonctions pour le mode point d'accès (AP)

#### 4. Implémentation des fonctions API
- **Problème** : Fonctions `apiInit` et `apiEnable` manquantes
- **Fichiers concernés** : 
  - `include/communication/api.h`
  - `src/communication/api.cpp`
- **Solution** : 
  - Création de l'interface API avec les fonctions nécessaires
  - Implémentation des gestionnaires REST pour les différentes routes

#### 5. Implémentation de servoInitAll
- **Problème** : Fonction `servoInitAll()` référencée mais non implémentée
- **Fichiers concernés** : 
  - `include/hardware/actuators/servo.h`
  - `src/hardware/actuators/servo.cpp`
- **Solution** : Implémentation complète de la gestion des servomoteurs

#### 6. Implémentation de ButtonUIManager
- **Problème** : Classe `ButtonUIManager` référencée mais non implémentée
- **Fichiers concernés** : 
  - `include/hardware/io/button_ui.h`
  - `src/hardware/io/button_ui.cpp`
- **Solution** : Implémentation complète de la gestion des boutons d'interface

### Problèmes de dépendances résolus

#### 1. Bibliothèque ArduinoJson manquante
- **Problème** : `ArduinoJson.h` introuvable lors de la compilation
- **Solution** : Installation de la bibliothèque via PlatformIO
  ```
  pio lib install ArduinoJson
  ```

#### 2. Ambiguïté dans la fonction apiEnable
- **Problème** : Deux versions de la fonction `apiEnable` avec la même signature
- **Solution** : Unification des signatures en une seule fonction retournant un booléen

#### 3. StaticJsonDocument déprécié
- **Problème** : Utilisation de `StaticJsonDocument` déprécié dans ArduinoJson v7.4.1
- **Solution** : Mise à jour du code pour utiliser `JsonDocument` à la place

### Centralisation des configurations

#### 1. Centralisation des définitions de broches
- **Problème** : Broches définies à différents endroits du code
- **Orientation** : Centralisation de toutes les définitions de broches dans `config.h`
- **Détails** :
  - Déplacement de `SERVO_LINEMOD_PIN` de `servo.h` vers `config.h`
  - Déplacement des pins de boutons de `button_ui.h` vers `config.h`
  - Élimination des redondances et harmonisation des noms

## Erreurs identifiées précédemment (5 mai 2025)

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