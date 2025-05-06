# Kite PiloteV3 - Autopilote pour Kite Générateur d'Électricité

## Description

Kite PiloteV3 est un système d'autopilote avancé pour kite générateur d'électricité basé sur ESP32. Ce projet a pour objectif de maximiser la production d'énergie renouvelable grâce à un cerf-volant de traction en contrôlant automatiquement sa trajectoire. Le système offre une interface web avec capacité de mise à jour OTA (Over The Air) et une interface utilisateur via écran LCD 2004, permettant un contrôle local ou distant. L'architecture multitâche robuste intègre des protections contre les problèmes de mémoire, des mécanismes de sécurité avancés et un système de journalisation complet.

## Architecture du projet

Le projet Kite PiloteV3 est construit selon l'architecture **MCP (Model-Controller-Presenter)**, une variante moderne et adaptée aux systèmes embarqués IoT:

- **Model**: Gère les données et la logique métier du système
  - Définit les structures de données du système (ex: `AutopilotParameters`, `AutopilotState`)
  - Implémente les algorithmes de calcul et traitement des données
  - Assure la persistance et l'intégrité des données

- **Controller**: Contrôle le comportement du système et orchestre les interactions
  - Implémente les algorithmes d'autopilotage (ex: module `autopilot.h`)
  - Gère les réactions aux événements système (capteurs, utilisateur)
  - Coordonne les différents composants matériels et logiciels

- **Presenter**: Prépare et présente les données à l'utilisateur
  - Gère l'interface LCD et les boutons physiques
  - Implémente l'interface web pour l'accès à distance
  - Transforme les données brutes en informations compréhensibles

Cette architecture MCP offre plusieurs avantages pour un projet IoT complexe:
- **Séparation claire des responsabilités**: facilite la maintenance et les tests
- **Modularité**: permet de modifier une couche sans impacter les autres
- **Adaptabilité**: particulièrement adaptée aux ressources limitées des systèmes embarqués
- **Testabilité**: chaque composant peut être testé indépendamment

## Objectifs du Projet

- **Maximiser la production d'énergie renouvelable** en optimisant les trajectoires de vol du kite
- **Assurer une sécurité maximale** avec des mécanismes de détection et réponse aux situations dangereuses
- **Offrir une interface utilisateur intuitive** tant sur l'écran LCD local que via l'interface web
- **Permettre une adaptation aux conditions changeantes** grâce aux algorithmes adaptatifs
- **Fournir des diagnostics avancés** pour le dépannage et l'optimisation des performances

## Fonctionnalités

- **Algorithmes d'autopilote avancés** : Calcul et optimisation des trajectoires de vol pour maximiser la génération d'énergie
- **Modes de vol automatiques et manuels** : Différentes stratégies de vol adaptées aux conditions météorologiques
- **Système de sécurité intégré** : Détection automatique des situations dangereuses avec procédures d'urgence
- **Interface utilisateur avec écran LCD** : Interface intuitive avec écran LCD 2004 et boutons de navigation
- **Architecture multitâche robuste** : Utilisation de FreeRTOS avec protections mémoire avancées
- **Système de journalisation avancé** : Différents niveaux de log (ERROR, WARNING, INFO, DEBUG) avec formatage couleur
- **Interface web responsive** : Interface web moderne et adaptative accessible à distance
- **Mise à jour OTA** : Possibilité de mettre à jour le firmware sans connexion physique
- **Serveur web asynchrone** : Réponse rapide et efficace aux requêtes web
- **Gestion optimisée de la mémoire** : Buffers statiques et protection contre les corruptions heap
- **Retour d'état en temps réel** : Affichage d'informations système et télémétrie sur l'écran LCD
- **Menus interactifs** : Navigation entre différents écrans pour la configuration et le monitoring
- **Surveillance des ressources système** : Monitoring continu de l'utilisation mémoire et performances
- **Répartition des tâches sur deux cœurs** : Optimisation des performances avec le dual-core de l'ESP32
- **Configuration centralisée** : Fichier config.h pour simplifier la maintenance et les modifications

## Matériel requis

- **Carte principale** : ESP32 DevKit C v4 ou équivalent
- **Écran** : LCD 2004 (20 caractères x 4 lignes) avec module I2C
- **Composants** :
  - 1 LED rouge (indicateur d'état)
  - 1 Résistance 220Ω (pour la LED)
  - 4 Boutons poussoirs (bleu, vert, rouge, et jaune)
- **Alimentation** : Via USB ou source externe 5V

## Schéma de connexion

- **Écran LCD 2004 avec module I2C** :
  - SDA → GPIO21 de l'ESP32
  - SCL → GPIO22 de l'ESP32
  - VCC → 5V
  - GND → GND
- **LED d'état** :
  - Anode (+) → GPIO2 via résistance 220Ω
  - Cathode (-) → GND
- **Boutons poussoirs** :
  - Bouton bleu → GPIO15 et GND
  - Bouton vert → GPIO16 et GND

## Schéma de connexion optimisé

### Composants requis
- 1× ESP32 DevKit C v4
- 1× Écran LCD 2004 avec module I2C
- 1× LED rouge indicatrice
- 1× Résistance 220Ω
- 3× Potentiomètres à glissière:
  - 1× horizontal (10kΩ) pour la direction
  - 2× verticaux (10kΩ) pour le trim et la longueur des lignes
- 2× Boutons poussoirs (bleu et vert)
- 2× Servomoteurs
- 1× Moteur pas à pas avec driver

### Connexions détaillées
- **Écran LCD 2004 avec module I2C**:
  - SDA → GPIO21 de l'ESP32
  - SCL → GPIO22 de l'ESP32
  - VCC → 5V
  - GND → GND
- **LED d'état**:
  - Anode (+) → GPIO2 via résistance 220Ω
  - Cathode (-) → GND
- **Boutons poussoirs**:
  - Bouton bleu → GPIO15 et GND
  - Bouton vert → GPIO16 et GND
- **Potentiomètres à glissière**:
  - Potentiomètre horizontal (DIRECTION):
    - Pin central → GPIO34 (ADC1_CH6)
    - Pin gauche → GND
    - Pin droit → 3.3V
  - Potentiomètre vertical 1 (TRIM):
    - Pin central → GPIO35 (ADC1_CH7)
    - Pin inférieur → GND
    - Pin supérieur → 3.3V
  - Potentiomètre vertical 2 (LONGUEUR):
    - Pin central → GPIO32 (ADC1_CH4)
    - Pin inférieur → GND
    - Pin supérieur → 3.3V
- **Servomoteurs**:
  - Servomoteur direction:
    - Signal → GPIO25
    - VCC → 5V externe
    - GND → GND
  - Servomoteur trim:
    - Signal → GPIO26
    - VCC → 5V externe
    - GND → GND
- **Moteur pas à pas (pour la longueur des lignes)**:
  - IN1 → GPIO5
  - IN2 → GPIO18
  - IN3 → GPIO19
  - IN4 → GPIO23
  - VCC → 5V externe
  - GND → GND

## Installation

### Prérequis

- [PlatformIO](https://platformio.org/) (comme extension VSCode ou en ligne de commande)
- Connexion Internet pour télécharger les dépendances

### Étapes d'installation

1. Cloner ce dépôt

```bash
git clone https://github.com/votre-nom/kite_PiloteV3.git
cd kite_PiloteV3
```

2. Vérifier que PlatformIO est installé dans VSCode :
   - Ouvrir VSCode
   - Aller dans l'onglet Extensions (Ctrl+Shift+X)
   - Rechercher et installer "PlatformIO IDE" si ce n'est pas déjà fait

3. Ouvrir le projet dans PlatformIO (VSCode)

```bash
code .
```

3. Compiler et téléverser le code

```bash
# Compiler le projet
pio run

# Téléverser sur l'ESP32
pio run --target upload

# Surveiller le port série
pio device monitor
```

4. Après le premier téléversement, les mises à jour suivantes peuvent se faire via OTA en accédant à:

```
http://[adresse-ip-esp32]/update
```

## Configuration

### Paramètres WiFi et autres configurations

Pour personnaliser votre système, modifiez les constantes dans le fichier `include/config.h`. Ce fichier centralisé regroupe tous les paramètres configurables :

```cpp
// Configuration WiFi
#define WIFI_SSID         "Votre-SSID"
#define WIFI_PASSWORD     "Votre-Mot-De-Passe"
#define SERVER_PORT       80

// Niveaux de journalisation (0=NONE, 1=ERROR, 2=WARNING, 3=INFO, 4=DEBUG)
#define LOG_LEVEL         3  // Niveau INFO par défaut

// Configuration mémoire
#define USE_STATIC_MEMORY          true    // Utiliser des allocations statiques quand possible
#define STRING_BUFFER_SIZE         128     // Taille des buffers pour les chaînes de caractères
#define LOG_BUFFER_SIZE            256     // Taille du buffer pour les messages de journalisation

// Pins matérielles, couleurs, dimensions d'écran, etc.
// Consultez le fichier config.h pour toutes les options disponibles
```

### Dépendances

Le projet utilise les bibliothèques suivantes, configurées dans `platformio.ini` :

- **ElegantOTA** : Pour les mises à jour OTA
- **ESPAsyncWebServer** et **AsyncTCP** : Pour le serveur web asynchrone
- **WiFi**, **SPI** et **Wire** : Pour la connectivité WiFi, SPI et I2C
- **FreeRTOS** : Pour la gestion multitâche (intégré à l'ESP32)

### Niveau de journalisation

Le système de journalisation dispose de 5 niveaux configurables :

- **LOG_NONE (0)** : Pas de journalisation
- **LOG_ERROR (1)** : Erreurs critiques uniquement (rouge)
- **LOG_WARNING (2)** : Erreurs et avertissements (jaune) 
- **LOG_INFO (3)** : Informations générales (vert)
- **LOG_DEBUG (4)** : Messages de débogage détaillés (cyan)

Le niveau peut être configuré dans `platformio.ini` :
```ini
build_flags = 
  -DLOG_LEVEL=3  # 0=NONE, 1=ERROR, 2=WARNING, 3=INFO, 4=DEBUG
```

## Utilisation

1. **Démarrage** : 
   - Au démarrage, le système initialise le module de journalisation
   - Il affiche un message de bienvenue sur l'écran LCD
   - Il tente de se connecter au réseau WiFi configuré
   - En cas de succès, l'adresse IP attribuée s'affiche à l'écran
   - L'interface est initialisée avec le menu principal

2. **Interface à boutons** :
   - **Menu principal** : 
     - Tableau de bord : Accès aux données et graphiques
     - Paramètres : Configuration du système
     - Informations : Statut et version du système
   - **Navigation** :
     - Bouton bleu : Retour au menu principal
     - Bouton vert : Navigation vers le haut
     - Bouton rouge : Validation/Sélection
     - Bouton jaune : Navigation vers le bas

3. **Interface web** :
   - Page d'accueil : `http://[adresse-ip-esp32]/`
   - Interface de mise à jour OTA : `http://[adresse-ip-esp32]/update`

4. **Mise à jour OTA** :
   - Accéder à l'URL de mise à jour dans un navigateur
   - Sélectionner le nouveau fichier firmware (.bin)
   - L'écran LCD affiche la progression pendant la mise à jour

5. **Surveillance du système** :
   - Le système surveille en permanence l'utilisation de la mémoire
   - Ces informations sont affichées via le système de journalisation
   - Les erreurs sont clairement marquées dans les logs avec un code couleur

## Simulation avec Wokwi

Le projet inclut les fichiers nécessaires pour une simulation dans l'environnement Wokwi :

- `diagram.json` : Configuration du circuit et des connexions
- `wokwi.toml` : Configuration de la simulation

Pour lancer la simulation :
1. Installer l'extension Wokwi pour VS Code
2. Ouvrir le projet dans VS Code
3. Démarrer la simulation avec la commande "Wokwi: Start Simulator"

## Structure du projet
/workspaces/kite_PiloteV3/
├── include/                      # Fichiers d'en-tête C++
│   ├── core/                     # Module noyau
│   │   ├── config.h              # Configuration globale du système
│   │   ├── task_manager.h        # Gestion des tâches FreeRTOS
│   │   └── system.h              # Fonctions système générales
│   ├── hardware/                 # Interface matérielle
│   │   ├── sensors/              # Capteurs
│   │   │   ├── imu.h             # Interface IMU
│   │   │   ├── tension.h         # Capteur de tension
│   │   │   ├── wind.h            # Anémomètre/girouette
│   │   │   └── line_length.h     # Capteur longueur de ligne
│   │   └── actuators/            # Actionneurs
│   │       ├── servos.h          # Contrôle des servomoteurs
│   │       ├── generator.h       # Contrôle du générateur
│   │       └── winch.h           # Contrôle du treuil
│   ├── communication/            # Module de communication
│   │   ├── espnow_manager.h      # Gestion de la communication ESP-NOW
│   │   ├── wifi_manager.h        # Gestion du WiFi
│   │   └── protocols.h           # Définition des protocoles
│   ├── control/                  # Algorithmes de contrôle
│   │   ├── autopilot.h           # Autopilote principal
│   │   ├── pid.h                 # Contrôleurs PID
│   │   ├── trajectory.h          # Gestion des trajectoires
│   │   └── safety.h              # Système de sécurité
│   ├── ui/                       # Interface utilisateur
│   │   ├── dashboard.h           # Interface tableau de bord
│   │   ├── display.h             # Gestion de l'affichage LCD
│   │   ├── inputs.h              # Gestion des entrées utilisateur
│   │   └── webserver.h           # Serveur web pour interface
│   └── utils/                    # Utilitaires
│       ├── logging.h             # Système de journalisation
│       ├── diagnostics.h         # Outils de diagnostic
│       ├── terminal.h            # Terminal distant
│       └── data_storage.h        # Stockage de données
├── src/                          # Fichiers source C++
│   ├── core/                     # Implémentation du noyau
│   │   ├── config.cpp            # Implémentation configuration
│   │   ├── task_manager.cpp      # Implémentation gestionnaire tâches
│   │   ├── main.cpp              # Point d'entrée du programme
│   │   └── system.cpp            # Implémentation fonctions système
│   ├── hardware/                 # Implémentation matérielle
│   │   ├── sensors/              # Implémentation capteurs
│   │   │   ├── imu.cpp           # Implémentation IMU
│   │   │   ├── tension.cpp       # Implémentation capteur tension
│   │   │   ├── wind.cpp          # Implémentation capteur vent
│   │   │   └── line_length.cpp   # Implémentation capteur longueur
│   │   └── actuators/            # Implémentation actionneurs
│   │       ├── servos.cpp        # Implémentation servomoteurs
│   │       ├── generator.cpp     # Implémentation générateur
│   │       └── winch.cpp         # Implémentation treuil
│   ├── communication/            # Implémentation communication
│   │   ├── espnow_manager.cpp    # Implémentation ESP-NOW
│   │   ├── wifi_manager.cpp      # Implémentation WiFi
│   │   └── protocols.cpp         # Implémentation protocoles
│   ├── control/                  # Implémentation contrôle
│   │   ├── autopilot.cpp         # Implémentation autopilote
│   │   ├── pid.cpp               # Implémentation PID
│   │   ├── trajectory.cpp        # Implémentation trajectoires
│   │   └── safety.cpp            # Implémentation sécurité
│   ├── ui/                       # Implémentation UI
│   │   ├── dashboard.cpp         # Implémentation tableau de bord
│   │   ├── display.cpp           # Implémentation affichage
│   │   ├── inputs.cpp            # Implémentation entrées
│   │   └── webserver.cpp         # Implémentation serveur web
│   └── utils/                    # Implémentation utilitaires
│       ├── logging.cpp           # Implémentation journalisation
│       ├── diagnostics.cpp       # Implémentation diagnostic
│       ├── terminal.cpp          # Implémentation terminal distant
│       └── data_storage.cpp      # Implémentation stockage données
├── data/                         # Fichiers pour SPIFFS (web)
│   ├── css/                      # Styles CSS
│   │   └── style.css             # Feuille de style principale
│   ├── js/                       # Scripts JavaScript
│   │   ├── dashboard.js          # Scripts pour tableau de bord
│   │   └── controls.js           # Scripts pour contrôles
│   ├── img/                      # Images
│   │   └── logo.png              # Logo du projet
│   ├── fonts/                    # Polices
│   ├── dashboard.html            # Interface principale
│   ├── config.html               # Page de configuration
│   ├── diagnostic.html           # Page de diagnostic
│   └── index.html                # Page d'accueil
├── test/                         # Tests unitaires et d'intégration
│   ├── test_sensors.cpp          # Tests des capteurs
│   ├── test_actuators.cpp        # Tests des actionneurs
│   ├── test_control.cpp          # Tests des algorithmes de contrôle
│   └── test_communication.cpp    # Tests de communication
├── lib/                          # Bibliothèques externes
├── platformio.ini                # Configuration PlatformIO
└── docs-fr/                      # Documentation
    ├── projet_kite_complet.md    # Documentation complète
    ├── manuel_utilisateur.md     # Guide utilisateur
    ├── guide_developpeur.md      # Guide développeur
    └── schemas/                  # Diagrammes et schémas

## Optimisations et améliorations

Le projet inclut plusieurs améliorations pour assurer la performance et la stabilité :

1. **Système de journalisation avancé** :
   - Plusieurs niveaux de journalisation (ERROR, WARNING, INFO, DEBUG)
   - Affichage coloré pour distinguer les niveaux de gravité
   - Horodatage précis des événements
   - Affichage automatique de l'utilisation mémoire
   - Buffers statiques pour éviter les problèmes d'allocation

2. **Architecture multitâche robuste** :
   - Tâches séparées avec priorités et tailles de pile optimisées
   - Exécution sur les deux cœurs de l'ESP32 pour une meilleure performance
   - Protection contre les accès concurrents via sémaphores
   - Mécanisme de messages inter-tâches avec buffers statiques

3. **Gestion optimisée de la mémoire** :
   - Utilisation systématique de buffers statiques pour éviter la fragmentation
   - Contrôle strict des allocations dynamiques
   - Surveillance continue de l'utilisation de la heap
   - Vérification des limites pour tous les accès tableau
   - Utilisation de `snprintf` au lieu de concaténations String

4. **Protection contre les corruptions de mémoire** :
   - Vérifications de validité des pointeurs et indices de tableaux
   - Gestion des exceptions pour la sécurité des opérations
   - Actions différées pour les opérations non-critiques
   - Validation des données avant traitement

5. **Interface web améliorée** :
   - Design moderne et responsive
   - Utilisation de CSS pour une meilleure présentation
   - Génération HTML optimisée (sans SPIFFS)
   - Buffers de taille fixe pour les réponses HTTP

6. **Interface à boutons fiable** :
   - Anti-rebond matériel et logiciel pour les boutons
   - Vérifications strictes des limites des tableaux
   - Elimination des `delay()` bloquants
   - Navigation intuitive dans les menus LCD

## Développement

### Système de journalisation

Le nouveau système de journalisation permet d'afficher des messages formatés avec différents niveaux de gravité :

```cpp
// Initialisation du système de log
logInit(LOG_INFO);  // Niveau INFO par défaut

// Exemples d'utilisation
LOG_ERROR("TAG", "Une erreur critique s'est produite: %d", code);
LOG_WARNING("TAG", "Attention: %s", message);
LOG_INFO("TAG", "Information: La valeur est %d", valeur);
LOG_DEBUG("TAG", "Débogage: Variable x = %f", x);

// Affichage de l'utilisation mémoire
logMemoryUsage("TAG");
```

### Assistants IA

Le projet utilise des assistants d'intelligence artificielle pour faciliter le développement :

- **Cline** : Assistant IA intégré utilisé pour l'analyse de code et les suggestions
- **GitHub Copilot** : Assistant IA pour la génération de code et les recommandations

Pour interagir efficacement avec ces assistants IA, consultez les instructions dans `.clinerules/prompt01`. Ces instructions incluent des conventions pour formuler les demandes de code et les bonnes pratiques à suivre.

### Bonnes pratiques

Le projet suit les conventions de codage définies dans `/docs-fr/`. Quelques principes clés :

- Utiliser le français pour les commentaires
- Indentation de 2 espaces
- Éviter les `delay()` bloquants, préférer `millis()` ou les fonctions FreeRTOS
- Utiliser des buffers statiques plutôt que des allocations dynamiques
- Toujours vérifier les limites des tableaux avant d'y accéder
- Préférer `snprintf()` aux concaténations de chaînes
- Diviser le code en petites fonctions à responsabilité unique
- Vérifier l'utilisation mémoire régulièrement
- Protéger les accès concurrents avec des sémaphores

### Architecture logicielle

- **Système de journalisation** :
  - Gestion des niveaux de logs (ERROR, WARNING, INFO, DEBUG)
  - Formatage avec horodatage et codes couleur
  - Surveillance de l'utilisation mémoire
- **Architecture multitâche** : 
  - Tâche d'affichage (core 1, priorité élevée)
  - Tâche de gestion des boutons (core 1, priorité élevée)
  - Tâche de surveillance WiFi (core 0, priorité basse)
  - Tâche de surveillance système (core 0, priorité basse)
- **Communication inter-tâches** :
  - File d'attente de messages avec buffers statiques
  - Sémaphores pour la synchronisation
- **Interface utilisateur tactile** :
  - Système de menus avec écrans multiples
  - Mécanisme d'action différée pour éviter les corruptions mémoire
  - Vérifications strictes des limites
- **Gestion mémoire optimisée** :
  - Buffers statiques prédimensionnés
  - Allocation statique prioritaire
  - Surveillance continue de l'utilisation heap

## Dépannage

### Problèmes courants

- **Écran LCD non fonctionnel** : Vérifier le câblage I2C (SDA, SCL) et les connexions de l'écran LCD
- **Interface non réactive** : Vérifier que le module I2C est bien reconnu
- **Problèmes de tâches FreeRTOS** : Vérifier les tailles de pile allouées dans config.h
- **Connexion WiFi impossible** : Vérifier les identifiants dans le fichier config.h
- **Port série sans affichage** : S'assurer que la vitesse est configurée à 115200 bauds
- **Corruption de heap** : Utiliser le système de journalisation pour identifier l'utilisation mémoire excessive

### Utilisation des journaux

Le système de journalisation avancé peut être utilisé pour diagnostiquer les problèmes :

- Définir `LOG_LEVEL=4` dans platformio.ini pour obtenir tous les détails de débogage
- Observer les messages colorés dans le moniteur série
- Surveiller les rapports d'utilisation mémoire (`Free: X/Y bytes (Z%)`) 
- Rechercher les messages d'erreur en rouge pour identifier les problèmes critiques
- Analyser les statistiques des tâches FreeRTOS pour détecter d'éventuels dépassements de pile

## Contributions

Les contributions sont bienvenues. Merci de suivre ces étapes :

1. Fork du projet
2. Création d'une branche (`git checkout -b feature/amazing-feature`)
3. Commit de vos changements (`git commit -m 'Ajout d'une fonctionnalité'`)
4. Push sur la branche (`git push origin feature/amazing-feature`)
5. Ouverture d'une Pull Request

## Licence

Ce projet est sous licence MIT. Voir le fichier LICENSE pour plus de détails.

## Auteurs

- Contributeurs du projet Kite PiloteV3

## Remerciements

- Bibliothèque ElegantOTA pour les mises à jour sans fil
- LiquidCrystal_I2C pour l'interface LCD
- Communauté ESP32 et PlatformIO
- Framework FreeRTOS pour la gestion multitâche

## Date de dernière mise à jour

1 mai 2025

# Instructions pour les Interactions Futures

## Contexte du Projet

- **Date actuelle** : 6 mai 2025
- **Système d'exploitation** : Linux
- **Structure du projet** :
  - Le projet est organisé en plusieurs dossiers : `src`, `include`, `docs-fr`, `data`, etc.
  - Les fichiers principaux incluent des modules pour la communication, le contrôle, le matériel, l'interface utilisateur et les utilitaires.
  - Une documentation détaillée est disponible dans le dossier `docs-fr`.

## Directives Générales

1. **Respect de la Structure** :
   - Toute modification doit respecter l'architecture existante du projet.
   - Les fichiers doivent être placés dans les dossiers appropriés (ex : `src/communication` pour les modules de communication).

2. **Documentation** :
   - Ajouter des commentaires clairs et descriptifs pour chaque modification.
   - Mettre à jour la documentation dans `docs-fr` si nécessaire.

3. **Tests** :
   - Vérifier que les tests dans le dossier `test/` couvrent les nouvelles fonctionnalités ou corrections.
   - Ajouter de nouveaux tests si nécessaire.

4. **Gestion Git** :
   - Utiliser les tâches Git disponibles pour ajouter, valider et pousser les modifications.
   - Respecter les conventions de commit (ex : `[Feature] Ajout d'une nouvelle fonctionnalité`).

5. **Collaboration** :
   - Toutes les interactions doivent être basées sur le contexte actuel du projet.
   - Les réponses doivent être précises, concises et adaptées au projet.

## Utilisation des Outils

- **Exploration du Projet** :
  - Utiliser les outils disponibles pour rechercher, lire ou modifier les fichiers.
  - S'assurer que les modifications sont conformes aux bonnes pratiques.

- **Exécution des Tâches** :
  - Utiliser les tâches prédéfinies pour gérer les interactions avec Git.
  - Vérifier l'état du projet après chaque modification.

- **Communication** :
  - Fournir des explications détaillées pour chaque action ou modification.
  - Répondre aux questions en utilisant le contexte du projet.

## Objectif

L'objectif est de garantir que toutes les interactions futures respectent les directives ci-dessus et contribuent à l'amélioration continue du projet.
