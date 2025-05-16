# Kite PiloteV3 - Autopilote pour Kite Générateur d'Électricité

## Description

Kite PiloteV3 est un système d'autopilote avancé pour kite générateur d'électricité basé sur ESP32. Ce projet a pour objectif de maximiser la production d'énergie renouvelable grâce à un cerf-volant de traction en contrôlant automatiquement sa trajectoire. Le système offre une interface web avec capacité de mise à jour OTA (Over The Air) et une interface utilisateur via écran LCD 2004, permettant un contrôle local ou distant. L'architecture multitâche robuste intègre des protections contre les problèmes de mémoire, des mécanismes de sécurité avancés et un système de journalisation complet.

**Note de Refactorisation (Mai 2025):** Ce projet est en cours d'une refactorisation majeure vers une architecture basée sur des composants. L'ancienne architecture orientée modules a été remplacée par une architecture à couches utilisant les concepts de HAL (Hardware Abstraction Layer) et de composants gérés (`ManagedComponent`).

## Architecture (Post-Refactorisation)

L'architecture est maintenant organisée en couches clairement définies, avec une séparation nette entre l'abstraction matérielle, les services et la logique applicative.

- **Couches Clés :**
    1. **HAL (Hardware Abstraction Layer) :** Pilotes de bas niveau pour les périphériques (IMU, servos, LCD, etc.), héritant de `HALComponent` (ou `SensorComponent`, `ActuatorComponent`, `InputComponent`, `OutputComponent`).
    2. **Services Système & Cœur :** Composants transversaux comme `TaskManager` (gestion des tâches FreeRTOS pour les `ManagedComponent`), `SystemStateManager` (état global du système), `LoggingModule` (journalisation), `ErrorHandler` (gestion des erreurs).
    3. **Services Applicatifs :** Logique métier encapsulée dans des services (ex: `SensorService`, `ControlService`, `UIService`), héritant de `ManagedComponent`.
    4. **Application :** Orchestration de haut niveau des services applicatifs via le `SystemOrchestrator`.
    5. **Présentation :** Gestion des interfaces utilisateur (ex: `UIManager` pour l'écran LCD).

- **`ManagedComponent` :** Classe de base pour tous les éléments logiciels gérables, fournissant une interface commune pour l'initialisation, le cycle de vie, la gestion d'état et la récupération d'informations.
- **Configuration :** Centralisée dans `include/core/config.h` pour les valeurs par défaut, avec possibilité de configuration spécifique lors de l'instanciation des composants.
- **Gestion des Tâches :** Le `TaskManager` est responsable de la création et de la gestion des tâches FreeRTOS associées aux `ManagedComponent` qui nécessitent leur propre thread d'exécution.
- **Communication Inter-Composants :** Utilisation de files de messages FreeRTOS avec des structures `IPCMessage` standardisées.
- **Énumérations Globales :** `ComponentState`, `ErrorCode`, `MessageType`, `TaskPriority`, `LogLevel` définies dans `include/common/global_enums.h`.
- **LoggingModule :** Le module de journalisation centralisé est désormais situé dans `include/core/logging.h` et `src/core/logging.cpp` (anciennement dans utils, tous les fichiers obsolètes ont été supprimés et les inclusions corrigées dans tout le projet).

## Implémentation HAL

La nouvelle couche HAL (Hardware Abstraction Layer) offre plusieurs avantages :
- **Indépendance du matériel** : Le code de plus haut niveau n'a pas besoin de connaître les détails du matériel utilisé.
- **Testabilité accrue** : Les composants HAL peuvent être facilement remplacés par des mocks pour les tests.
- **Cohérence d'interface** : Tous les pilotes matériels suivent la même structure et comportement.

Les principaux composants HAL incluent :
- **IMUDriver** : Capteur de mouvement (accéléromètre et gyroscope)
- **ServoDriver** : Contrôle des servomoteurs
- **DisplayDriver** : Gestion de l'écran LCD
- **ButtonsDriver** : Gestion des boutons d'interface
- **PotentiometerDriver** : Lecture des potentiomètres
- **LineLeugthDriver** : Mesure de la longueur de ligne
- **TensionDriver** : Mesure de la tension de la ligne
- **WindDriver** : Mesure du vent

## Composants Système

- **SystemOrchestrator** : Orchestration globale du système et séquencement des opérations
- **TaskManager** : Gestion des tâches FreeRTOS pour les composants
- **ErrorHandler** : Gestion centralisée des erreurs
- **LoggingModule** : Journalisation à différents niveaux (ERROR, WARNING, INFO, DEBUG)

## Interfaces

- Interface LCD avec menus dynamiques
- API REST pour le contrôle à distance
- Dashboard web responsive
- Mise à jour OTA (Over The Air)

## Statut du Projet

- Refactorisation majeure en cours
- Architecture HAL et Component implémentée
- Tests de compatibilité avec le matériel existant en cours
- Documentation en cours de mise à jour
