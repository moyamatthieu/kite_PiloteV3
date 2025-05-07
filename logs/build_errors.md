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

## Mise à jour du 7 mai 2025

### Problèmes de types LogLevel et journalisation

#### 1. Conflit entre types uint8_t et LogLevel
- **Problème** : Incompatibilité entre les fonctions déclarées avec `LogLevel` et implémentées avec `uint8_t`
- **Fichiers concernés** : 
  - `include/core/logging.h`
  - `src/core/logging.cpp`
- **Erreur** : `LogLevel' does not name a type; did you mean 'logSetLevel'` et autres erreurs de conversion de type
- **Solution** : 
  - Création d'une énumération `LogLevel` qui utilise les valeurs définies dans `config.h`
  - Ajout de casts explicites dans les macros de journalisation
  - Modification des signatures des fonctions pour utiliser le type `LogLevel` partout

#### 2. Redéfinition de MEM_HISTORY_SIZE
- **Problème** : `MEM_HISTORY_SIZE` défini à la fois dans `config.h` et redéfini dans `logging.cpp`
- **Fichiers concernés** :
  - `include/core/config.h`
  - `src/core/logging.cpp`
- **Solution** :
  - Suppression de la redéfinition dans `logging.cpp`
  - Utilisation exclusive de la définition de `config.h`

#### 3. Utilisation directe des constantes LOG_LEVEL_* 
- **Problème** : Dans `main.cpp`, appels directs à `logInit()` et `logPrint()` avec les constantes `LOG_LEVEL_*` non castées
- **Fichiers concernés** :
  - `src/core/main.cpp`
- **Erreur** : `invalid conversion from 'int' to 'LogLevel' [-fpermissive]`
- **Solution** :
  - Ajout de casts explicites vers `LogLevel` pour toutes les constantes `LOG_LEVEL_*` utilisées directement
  - Exemple : `logInit((LogLevel)LOG_LEVEL_INFO, 115200);`

### Centralisation des constantes

#### 1. Centralisation complète des constantes de journalisation
- **Problème** : Constantes de journalisation définies en partie dans `logging.h` et en partie dans `config.h`
- **Fichiers concernés** :
  - `include/core/config.h`
  - `include/core/logging.h`
- **Solution** :
  - Regroupement de toutes les constantes dans `config.h` dans une section dédiée "CONFIGURATION JOURNALISATION"
  - Modification de `logging.h` pour qu'il importe ces constantes depuis `config.h`
  - Ajout de sections thématiques dans `config.h` pour une meilleure organisation

#### 2. Résolution du conflit de LOG_BUFFER_SIZE
- **Problème** : `LOG_BUFFER_SIZE` défini différemment dans plusieurs fichiers
- **Fichiers concernés** :
  - `include/core/config.h` (192)
  - `include/core/logging.h` (256)
- **Solution** :
  - Définition unique dans `config.h` fixée à 192
  - Ajout de garde `#ifndef` pour éviter les redéfinitions
  - Prévention des avertissements de redéfinition lors de la compilation

#### 3. Centralisation des constantes de tâches FreeRTOS
- **Problème** : Constantes liées aux tâches FreeRTOS définies dans différents fichiers
- **Fichiers concernés** :
  - `include/core/config.h`
  - `include/core/task_manager.h`
- **Solution** :
  - Déplacement de `MAX_TASKS` et autres constantes dans `config.h`
  - Mise à jour de `task_manager.h` pour inclure `config.h` et utiliser ses définitions

### Implémentations manquantes

#### 1. Implémentation complète de TaskManager
- **Problème** : Méthodes manquantes dans la classe `TaskManager`
- **Fichiers concernés** :
  - `src/core/task_manager.cpp`
- **Erreur** : Références indéfinies à diverses méthodes de `TaskManager`
- **Solution** :
  - Implémentation complète de toutes les méthodes manquantes
  - Ajout des variables statiques requises
  - Implémentation des fonctions de tâches (displayTask, buttonTask, etc.)

### Statistiques de mémoire actuelles
- **RAM** : 14,6% utilisée (48 000 octets sur 327 680 octets)
- **Flash** : 81,9% utilisée (1 073 417 octets sur 1 310 720 octets)

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

# Guide de reprise du travail - Kite PiloteV3

## Statut actuel du projet (7 mai 2025)

### ✅ Corrections récemment implémentées
1. **Écran LCD sans scintillement** - Implémentation d'un système de buffer matriciel pour les mises à jour de l'écran LCD
2. **Monitoring système** - Ajout d'une tâche de surveillance qui vérifie l'état du système et affiche des statistiques
3. **Connexion potentiomètres-servomoteurs** - Implémentation de la connexion entre les entrées (potentiomètres) et les sorties (servomoteurs)
4. **Initialisation des servomoteurs** - Optimisation de la machine à états finis pour l'initialisation des servomoteurs

### 🚧 Problèmes en cours
1. **Servomoteurs** - Les servomoteurs sont maintenant fonctionnels mais pourraient bénéficier d'une calibration plus précise
2. **Monitoring avancé** - Le monitoring pourrait inclure des métriques plus détaillées sur l'utilisation CPU et mémoire

## Points de reprise pour la prochaine session

### 1️⃣ Améliorations de l'interface utilisateur
- [ ] Ajouter une page de calibration des servomoteurs dans l'interface web
- [ ] Améliorer l'affichage LCD pour montrer les valeurs actuelles des servomoteurs
- [ ] Implémenter des animations sur l'écran LCD pour les transitions entre menus

### 2️⃣ Optimisations des performances
- [ ] Optimiser davantage la fréquence de mise à jour des servomoteurs
- [ ] Réduire la consommation mémoire et CPU de la tâche de monitoring
- [ ] Implémenter une gestion d'économie d'énergie pour les périodes d'inactivité

### 3️⃣ Stabilité et robustesse
- [ ] Ajouter un système de sauvegarde/restauration des paramètres dans la mémoire flash
- [ ] Implémenter un mécanisme de détection et récupération des blocages système
- [ ] Améliorer la journalisation pour faciliter le diagnostic des problèmes

### 4️⃣ Fonctionnalités à développer
- [ ] Implémenter l'autopilote avec des trajectoires prédéfinies
- [ ] Ajouter le support pour l'IMU (capteur d'orientation)
- [ ] Développer la fonctionnalité de mesure de tension des lignes

## Guide rapide pour redémarrer le développement

1. **Vérification du système**
   ```bash
   pio run -t clean
   pio run
   ```

2. **Visualisation du monitoring**
   - Ouvrir le moniteur série pour vérifier le statut du système
   ```bash
   pio device monitor -b 115200
   ```

3. **Validation hardware**
   - S'assurer que les servomoteurs répondent aux commandes des potentiomètres
   - Vérifier l'affichage LCD pour confirmer qu'il reste sans scintillement

4. **Structure des fichiers clés**
   - `src/hardware/actuators/servo.cpp`: Contrôle des servomoteurs
   - `src/hardware/io/display_manager.cpp`: Gestion de l'affichage LCD
   - `src/core/task_manager.cpp`: Gestion des tâches FreeRTOS
   - `src/hardware/io/potentiometer_manager.cpp`: Gestion des potentiomètres

## Statistiques actuelles du système
- **RAM**: ~67% utilisée (217K/323K octets)
- **Mémoire libre minimale**: 217K octets
- **Bloc maximum allouable**: 110K octets

## Notes et rappels
- La tâche de monitoring envoie des statistiques toutes les 5 secondes
- Les servomoteurs sont initialisés avec les timers 0 et 1
- Le système de buffer LCD utilise une détection de différences pour minimiser les écritures