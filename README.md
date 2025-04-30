# Kite PiloteV3

## Description
Kite PiloteV3 est un système de contrôle basé sur ESP32 offrant une interface web avec capacité de mise à jour OTA (Over The Air) et une interface utilisateur locale via écran OLED. Ce système est conçu pour surveiller et contrôler des équipements à distance, avec une interface visuelle directe et une possibilité de mise à jour sans connexion physique.

## Fonctionnalités
- **Interface utilisateur double** : Affichage OLED local et interface web accessible à distance
- **Mise à jour OTA** : Possibilité de mettre à jour le firmware sans connexion physique
- **Serveur web asynchrone** : Réponse rapide et efficace aux requêtes web
- **Retour d'état en temps réel** : Affichage d'informations système sur l'écran OLED
- **Rotation automatique des informations** : Présentation séquentielle des données système
- **Surveillance et journalisation** : Retour détaillé sur le port série
- **Robustesse** : Gestion des erreurs et tentatives multiples d'initialisation

## Matériel requis
- **Carte principale** : ESP32 DevKit C v4 ou équivalent
- **Écran** : OLED SSD1306 128x64 pixels (I2C)
- **Composants** :
  - 1 LED rouge (indicateur d'état)
  - 1 Résistance 220Ω (pour la LED)
  - 2 Boutons poussoirs (bleu et vert)
- **Alimentation** : Via USB ou source externe 5V

## Schéma de connexion
- **Écran OLED SSD1306** :
  - SDA → GPIO21 de l'ESP32
  - SCL → GPIO22 de l'ESP32
  - VCC → 3.3V
  - GND → GND
- **LED d'état** :
  - Anode (+) → GPIO2 via résistance 220Ω
  - Cathode (-) → GND
- **Boutons poussoirs** :
  - Bouton bleu → GPIO4 et GND
  - Bouton vert → GPIO5 et GND

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

2. Ouvrir le projet dans PlatformIO (VSCode)
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

### Paramètres WiFi
Modifiez les variables suivantes dans `src/main.cpp` pour configurer la connexion WiFi :
```cpp
const char* ssid = "Votre-SSID";
const char* password = "Votre-Mot-De-Passe";
```

### Dépendances
Le projet utilise les bibliothèques suivantes, configurées dans `platformio.ini` :
- **ElegantOTA** : Pour les mises à jour OTA
- **Adafruit GFX** et **Adafruit SSD1306** : Pour l'écran OLED
- **ESPAsyncWebServer** et **AsyncTCP** : Pour le serveur web asynchrone
- **WiFi** et **Wire** : Pour la connectivité WiFi et I2C

## Utilisation

1. **Démarrage** : 
   - Au démarrage, le système affiche un message de bienvenue sur l'écran OLED
   - Il tente de se connecter au réseau WiFi configuré
   - En cas de succès, l'adresse IP attribuée s'affiche à l'écran

2. **Affichage OLED** :
   - Les informations s'affichent en rotation toutes les 5 secondes
   - État du système et SSID WiFi
   - Adresse IP de l'ESP32
   - Instructions pour accéder à l'interface OTA
   - Temps de fonctionnement du système

3. **Interface web** :
   - Page d'accueil : `http://[adresse-ip-esp32]/`
   - Interface de mise à jour OTA : `http://[adresse-ip-esp32]/update`

4. **Mise à jour OTA** :
   - Accéder à l'URL de mise à jour dans un navigateur
   - Sélectionner le nouveau fichier firmware (.bin)
   - L'écran OLED affiche la progression pendant la mise à jour

## Simulation avec Wokwi

Le projet inclut les fichiers nécessaires pour une simulation dans l'environnement Wokwi :
- `diagram.json` : Configuration du circuit et des connexions
- `wokwi.toml` : Configuration de la simulation

Pour lancer la simulation :
1. Installer l'extension Wokwi pour VS Code
2. Ouvrir le projet dans VS Code
3. Démarrer la simulation avec la commande "Wokwi: Start Simulator"

## Structure du projet
```
kite_PiloteV3/
├── src/                # Code source
│   └── main.cpp        # Programme principal
│   └── display.cpp     # Module d'affichage
├── include/            # Fichiers d'en-tête
│   └── display.h       # En-tête du module d'affichage
├── lib/                # Bibliothèques spécifiques
├── backup/             # Sauvegardes de code
├── docs-fr/            # Documentation en français
├── platformio.ini      # Configuration PlatformIO
├── diagram.json        # Configuration du circuit pour Wokwi
├── wokwi.toml          # Configuration de simulation Wokwi
└── README.md           # Ce fichier
```

## Développement

### Assistants IA
Le projet utilise des assistants d'intelligence artificielle pour faciliter le développement :
- **Cline** : Assistant IA intégré utilisé pour l'analyse de code et les suggestions
- **GitHub Copilot** : Assistant IA pour la génération de code et les recommandations

Pour interagir efficacement avec ces assistants IA, consultez les instructions dans `.clinerules/prompt01`. Ces instructions incluent des conventions pour formuler les demandes de code et les bonnes pratiques à suivre.

### Bonnes pratiques
Le projet suit les conventions de codage définies dans `/docs-fr/`. Quelques principes clés :
- Utiliser le français pour les commentaires
- Indentation de 2 espaces
- Éviter les `delay()` bloquants, préférer `millis()` pour la temporisation
- Diviser le code en fonctions avec des responsabilités clairement définies
- Séparer les préoccupations dans des modules distincts (affichage, réseau, etc.)

### Architecture logicielle
- **Initialisation** : Configuration du matériel et des communications
- **Connexion réseau** : Établissement de la liaison WiFi
- **Serveur web** : Configuration du serveur et des endpoints
- **Mise à jour OTA** : Gestion des callbacks pour les mises à jour
- **Affichage** : Rotation des informations sur l'écran OLED

## Fonctionnement
1. **Initialisation du système** : Configuration des périphériques et communications
2. **Connexion réseau** : Tentative de connexion au WiFi avec affichage de l'état
3. **Démarrage du serveur web** : Configuration et lancement du serveur asynchrone
4. **Boucle principale** : 
   - Traitement des requêtes OTA via `ElegantOTA.loop()`
   - Rotation des informations sur l'écran OLED toutes les 5 secondes

## Dépannage

### Problèmes courants
- **Écran non fonctionnel** : Vérifier le câblage I2C (pins 21/22) et l'adresse I2C (0x3C)
- **Connexion WiFi impossible** : Vérifier les identifiants dans le code
- **Port série sans affichage** : S'assurer que la vitesse est configurée à 115200 bauds

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
- Adafruit pour les bibliothèques d'écran OLED
- Communauté ESP32 et PlatformIO

## Date de dernière mise à jour
30 avril 2025
