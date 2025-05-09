# Kite PiloteV3 - Autopilote pour Kite Générateur d'Électricité

## Description

Kite PiloteV3 est un système d'autopilote avancé pour kite générateur d'électricité basé sur ESP32. Ce projet a pour objectif de maximiser la production d'énergie renouvelable grâce à un cerf-volant de traction en contrôlant automatiquement sa trajectoire. Le système offre une interface web avec capacité de mise à jour OTA (Over The Air) et une interface utilisateur via écran LCD 2004, permettant un contrôle local ou distant. L'architecture multitâche robuste intègre des protections contre les problèmes de mémoire, des mécanismes de sécurité avancés et un système de journalisation complet.

## Architecture

- Architecture orientée objet (OOP) : tous les modules (capteurs, actionneurs, communication) héritent d'une base `Module`.
- Activation/désactivation dynamique des modules à l'exécution (menu LCD, API REST, web).
- Chaque module encapsule sa propre FSM (StateMachine), gestion d'erreur (ErrorManager) et configuration.
- Enregistrement automatique des modules dans un `ModuleRegistry` centralisé.
- Tâches FreeRTOS démarrées/arrêtées dynamiquement selon l'état des modules.

## Modules principaux

- Capteurs : IMU, vent, longueur de ligne, tension (tous héritent de `SensorModule`)
- Actionneurs : Servo, Treuil (héritent de `ActuatorModule`)
- Communication : WiFi, API REST, Webserver (héritent de `Module`)

## Interfaces

- Menu LCD dynamique : activation/désactivation des modules, affichage de leur état.
- API REST `/api/v1/modules` : lister, activer/désactiver les modules à distance.
- Dashboard web : visualisation de l'état système et des modules.

## FSM & gestion d'erreur

- Chaque module possède sa propre machine à états non-bloquante.
- Gestion d'erreur dédiée par module, reporting centralisé.

## Extensibilité

- Ajout de nouveaux modules (capteurs/actionneurs/communication) très simple par héritage.
- Prêt pour la configuration dynamique (JSON, web, etc.).

## Statut

- Fonctionnement partiel validé : activation/désactivation dynamique, supervision, FSM, gestion d'erreur, API REST, UI LCD.
- Migration OOP complète pour tous les modules principaux.
- Voir `logs/discussions.md` pour l'historique détaillé des refontes.

## Améliorations récentes

### 1. Machine à États Finis (FSM) Non-Bloquante
Implémentation d'une approche par machine à états finis pour remplacer l'utilisation de `delay()` dans tout le projet :
- **Initialisation asynchrone des servomoteurs** : La fonction servoInitAll() utilise une FSM pour éviter tout délai bloquant
- **Communication WiFi non-bloquante** : Gestionnaire WiFi redessiné avec FSM pour maintenir la réactivité du système
- **Interfaces utilisateur fluides** : Écran LCD et entrées utilisateur gérés sans bloquer le thread principal

### 2. Architecture FreeRTOS Optimisée
- **TaskManager initialisé en priorité** : Le gestionnaire de tâches est maintenant démarré avant les autres composants
- **Définition claire des priorités** : Hiérarchie des priorités cohérente pour toutes les tâches
- **Optimisation de la communication inter-tâches** : Files de messages et sémaphores pour une coordination efficace

### 3. Documentation et Standards de Code Améliorés
- **En-têtes de fichiers standardisés** : Chaque fichier inclut maintenant une section "FONCTIONNEMENT" qui explique le code
- **Interdépendances documentées** : Les relations entre modules sont clairement documentées
- **Bonnes pratiques centralisées** : Nouveau fichier best_practices.md pour guider le développement

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

4. Compiler et téléverser le code

```bash
# Compiler le projet
pio run

# Téléverser sur l'ESP32
pio run --target upload

# Surveiller le port série
pio device monitor
```

5. Après le premier téléversement, les mises à jour suivantes peuvent se faire via OTA en accédant à:

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

### Interface à boutons

1. **Démarrage** : 
   - Au démarrage, le système initialise le module de journalisation
   - Il affiche un message de bienvenue sur l'écran LCD
   - Il tente de se connecter au réseau WiFi configuré
   - En cas de succès, l'adresse IP attribuée s'affiche à l'écran
   - L'interface est initialisée avec le menu principal

2. **Menu principal** : 
   - Tableau de bord : Accès aux données et graphiques
   - Paramètres : Configuration du système
   - Informations : Statut et version du système
   
3. **Navigation** :
   - Bouton bleu : Retour au menu principal
   - Bouton vert : Navigation vers le haut
   - Bouton rouge : Validation/Sélection
   - Bouton jaune : Navigation vers le bas

### Interface web

- Page d'accueil : `http://[adresse-ip-esp32]/`
- Interface de mise à jour OTA : `http://[adresse-ip-esp32]/update`
- API JSON pour l'accès aux données : `http://[adresse-ip-esp32]/api/status`

### Mise à jour OTA

- Accéder à l'URL de mise à jour dans un navigateur
- Sélectionner le nouveau fichier firmware (.bin)
- L'écran LCD affiche la progression pendant la mise à jour

### Surveillance du système

- Le système surveille en permanence l'utilisation de la mémoire
- Ces informations sont affichées via le système de journalisation
- Les erreurs sont clairement marquées dans les logs avec un code couleur

## Structure du projet

```
/kite_PiloteV3/
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
│   ├── hardware/                 # Implémentation matérielle
│   ├── communication/            # Implémentation communication
│   ├── control/                  # Implémentation contrôle
│   ├── ui/                       # Implémentation UI
│   └── utils/                    # Implémentation utilitaires
├── data/                         # Fichiers pour SPIFFS (web)
│   ├── css/                      # Styles CSS
│   ├── js/                       # Scripts JavaScript
│   ├── img/                      # Images
│   ├── fonts/                    # Polices
│   └── html/                     # Pages HTML
├── docs-fr/                      # Documentation
│   ├── projet_kite_complet.md    # Documentation complète
│   ├── best_practices.md         # Bonnes pratiques de développement
│   └── schemas/                  # Diagrammes et schémas
├── test/                         # Tests unitaires et d'intégration
├── platformio.ini                # Configuration PlatformIO
└── README.md                     # Documentation principale
```

## Simulation avec Wokwi

Le projet inclut les fichiers nécessaires pour une simulation dans l'environnement Wokwi :

- `diagram.json` : Configuration du circuit et des connexions
- `wokwi.toml` : Configuration de la simulation

Pour lancer la simulation :
1. Installer l'extension Wokwi pour VS Code
2. Ouvrir le projet dans VS Code
3. Démarrer la simulation avec la commande "Wokwi: Start Simulator"

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

3. **Approche non-bloquante par machine à états finis (FSM)** :
   - Remplacement des delay() par des FSM asynchrones
   - Meilleure réactivité du système
   - Utilisation optimale des ressources CPU
   - Modèle standardisé pour toutes les opérations asynchrones

4. **Gestion optimisée de la mémoire** :
   - Utilisation systématique de buffers statiques pour éviter la fragmentation
   - Contrôle strict des allocations dynamiques
   - Surveillance continue de l'utilisation de la heap
   - Vérification des limites pour tous les accès tableau
   - Utilisation de `snprintf` au lieu de concaténations String

5. **Interface web améliorée** :
   - Design moderne et responsive
   - Utilisation de CSS pour une meilleure présentation
   - Génération HTML optimisée (sans SPIFFS)
   - Buffers de taille fixe pour les réponses HTTP

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

### Principe de conception non-bloquant

Un des principes fondamentaux de ce projet est l'utilisation systématique de machines à états finis (FSM) pour remplacer les appels à `delay()`. Cette approche non-bloquante est essentielle pour un système temps réel multitâche.

**Exemple d'implémentation FSM :**
```cpp
// Au lieu de ceci (bloquant) :
void initDevice() {
    sendCommand();
    delay(100);  // Bloque le thread pendant 100ms
    readResponse();
    delay(50);   // Bloque à nouveau
    configure();
}

// Utiliser cette approche (non-bloquante) :
bool initDevice() {
    static uint8_t state = 0;
    static unsigned long lastActionTime = 0;
    
    switch (state) {
        case 0:  // Envoi de la commande
            sendCommand();
            lastActionTime = millis();
            state = 1;
            return false;  // Pas terminé
            
        case 1:  // Attente et lecture de la réponse
            if (millis() - lastActionTime >= 100) {
                readResponse();
                lastActionTime = millis();
                state = 2;
            }
            return false;  // Pas terminé
            
        case 2:  // Attente et configuration
            if (millis() - lastActionTime >= 50) {
                configure();
                state = 0;  // Réinitialisation pour le prochain appel
                return true;  // Terminé
            }
            return false;  // Pas terminé
    }
    
    return false;
}
```

Consultez le fichier `docs-fr/best_practices.md` pour plus d'informations sur l'implémentation de FSM.

## Dépannage

### Problèmes courants

- **Écran LCD non fonctionnel** : Vérifier le câblage I2C (SDA, SCL) et les connexions de l'écran LCD
- **Interface non réactive** : Vérifier que le module I2C est bien reconnu
- **Problèmes de tâches FreeRTOS** : Vérifier les tailles de pile allouées dans config.h
- **Connexion WiFi impossible** : Vérifier les identifiants dans le fichier config.h
- **Port série sans affichage** : S'assurer que la vitesse est configurée à 115200 bauds
- **Corruption de heap** : Utiliser le système de journalisation pour identifier l'utilisation mémoire excessive
- **Opérations bloquantes** : Vérifier que toutes les fonctions longues utilisent une approche FSM non-bloquante

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

### Règles de codage
Le projet suit des règles de codage strictes documentées dans `promptkite.prompt.md` et `docs-fr/best_practices.md`. Veillez à les consulter avant de contribuer.

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

7 mai 2025
