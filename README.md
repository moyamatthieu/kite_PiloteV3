# Kite PiloteV3

## Description

Kite PiloteV3 est un système de contrôle basé sur ESP32 offrant une interface web avec capacité de mise à jour OTA (Over The Air) et une interface utilisateur tactile via écran TFT ILI9341. Ce système est conçu pour surveiller et contrôler des équipements à distance, avec une interface visuelle interactive directe et une possibilité de mise à jour sans connexion physique. Le système utilise une architecture multitâche robuste avec des protections contre les problèmes de mémoire et des mécanismes de journalisation avancés.

## Fonctionnalités

- **Interface utilisateur tactile capacitive** : Interface tactile intuitive avec écran TFT ILI9341 et contrôleur capacitif FT6206
- **Architecture multitâche robuste** : Utilisation de FreeRTOS avec protections mémoire avancées
- **Système de journalisation avancé** : Différents niveaux de log (ERROR, WARNING, INFO, DEBUG) avec formatage couleur
- **Interface web responsive** : Interface web moderne et adaptative accessible à distance
- **Mise à jour OTA** : Possibilité de mettre à jour le firmware sans connexion physique
- **Serveur web asynchrone** : Réponse rapide et efficace aux requêtes web
- **Gestion optimisée de la mémoire** : Buffers statiques et protection contre les corruptions heap
- **Retour d'état en temps réel** : Affichage d'informations système sur l'écran tactile
- **Menus interactifs** : Navigation entre différents écrans pour la configuration et le monitoring
- **Rotation automatique des informations** : Présentation séquentielle des données système
- **Surveillance des ressources système** : Monitoring continu de l'utilisation mémoire et performances
- **Robustesse** : Gestion avancée des erreurs et tentatives multiples d'initialisation
- **Répartition des tâches sur deux cœurs** : Optimisation des performances avec le dual-core de l'ESP32
- **Configuration centralisée** : Fichier config.h pour simplifier la maintenance et les modifications

## Matériel requis

- **Carte principale** : ESP32 DevKit C v4 ou équivalent
- **Écran** : TFT ILI9341 240x320 pixels (portrait) avec écran tactile capacitif FT6206
- **Composants** :
  - 1 LED rouge (indicateur d'état)
  - 1 Résistance 220Ω (pour la LED)
  - 2 Boutons poussoirs (bleu et vert)
- **Alimentation** : Via USB ou source externe 5V

## Schéma de connexion

- **Écran TFT ILI9341** :
  - MOSI → GPIO23 de l'ESP32
  - MISO → GPIO19 de l'ESP32
  - CLK → GPIO18 de l'ESP32
  - CS → GPIO5 de l'ESP32
  - DC → GPIO27 de l'ESP32
  - RST → GPIO33 de l'ESP32
  - VCC → 3.3V
  - GND → GND
- **Écran tactile capacitif FT6206** :
  - SDA → GPIO21 de l'ESP32
  - SCL → GPIO22 de l'ESP32
  - IRQ → GPIO4 de l'ESP32 (optionnel)
  - VCC → 3.3V
  - GND → GND
- **LED d'état** :
  - Anode (+) → GPIO2 via résistance 220Ω
  - Cathode (-) → GND
- **Boutons poussoirs** :
  - Bouton bleu → GPIO15 et GND
  - Bouton vert → GPIO16 et GND

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
- **Adafruit GFX** et **Adafruit ILI9341** : Pour l'écran TFT
- **Adafruit FT6206** : Pour l'écran tactile capacitif
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
   - Il affiche un message de bienvenue sur l'écran TFT
   - Il tente de se connecter au réseau WiFi configuré
   - En cas de succès, l'adresse IP attribuée s'affiche à l'écran
   - L'interface tactile est initialisée avec le menu principal

2. **Interface tactile** :
   - **Menu principal** : 
     - Tableau de bord : Accès aux données et graphiques
     - Paramètres : Configuration du système
     - Informations : Statut et version du système
   - **Écran paramètres** :
     - WiFi : Configuration de la connexion sans fil
     - Affichage : Réglage de l'écran et des thèmes
     - Système : Options générales du système
     - OTA : Options de mise à jour
     - Retour : Retour au menu principal
   - **Écran tableau de bord** :
     - Graphiques : Visualisation des données
     - Données : Affichage des valeurs numériques
     - Alarmes : État des alertes système
     - Log : Journal des événements
     - Retour : Retour au menu principal

3. **Interface web** :
   - Page d'accueil : `http://[adresse-ip-esp32]/`
   - Interface de mise à jour OTA : `http://[adresse-ip-esp32]/update`

4. **Mise à jour OTA** :
   - Accéder à l'URL de mise à jour dans un navigateur
   - Sélectionner le nouveau fichier firmware (.bin)
   - L'écran TFT affiche la progression pendant la mise à jour

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

```plaintext
kite_PiloteV3/
├── src/                    # Code source
│   ├── main.cpp            # Programme principal
│   ├── display.cpp         # Module d'affichage
│   ├── touch_ui.cpp        # Module d'interface tactile
│   ├── task_manager.cpp    # Module de gestion des tâches
│   ├── webserver.cpp       # Module du serveur web
│   ├── logging.cpp         # Module de journalisation avancée
│   └── fallback_html.h     # HTML de secours
├── include/                # Fichiers d'en-tête
│   ├── config.h            # Configuration centralisée
│   ├── display.h           # En-tête du module d'affichage
│   ├── touch_ui.h          # En-tête du module d'interface tactile
│   ├── task_manager.h      # En-tête du module de gestion des tâches
│   ├── kite_webserver.h    # En-tête du module serveur web
│   └── logging.h           # En-tête du module de journalisation
├── lib/                    # Bibliothèques spécifiques
├── backup/                 # Sauvegardes de code
├── docs-fr/                # Documentation en français
├── platformio.ini          # Configuration PlatformIO
├── diagram.json            # Configuration du circuit pour Wokwi
├── wokwi.toml              # Configuration de simulation Wokwi
└── README.md               # Ce fichier
```

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
   - Gestion des exceptions dans les opérations tactiles
   - Actions différées pour l'interface tactile
   - Validation des données avant traitement

5. **Interface web améliorée** :
   - Design moderne et responsive
   - Utilisation de CSS pour une meilleure présentation
   - Génération HTML optimisée (sans SPIFFS)
   - Buffers de taille fixe pour les réponses HTTP

6. **Interface tactile plus fiable** :
   - Mécanisme d'action différée pour éviter les corruptions mémoire
   - Protection contre les appels récursifs de callbacks
   - Vérifications strictes des limites des tableaux
   - Elimination des `delay()` bloquants

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
  - Tâche de gestion tactile (core 1, priorité élevée)
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

- **Écran tactile non fonctionnel** : Vérifier le câblage I2C (SDA, SCL) et les connexions de l'écran tactile
- **Interface tactile non réactive** : Vérifier que le contrôleur FT6206 est bien reconnu
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
- Adafruit pour les bibliothèques d'écran TFT et tactile FT6206
- Communauté ESP32 et PlatformIO
- Framework FreeRTOS pour la gestion multitâche

## Date de dernière mise à jour

1 mai 2025
