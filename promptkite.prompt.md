# Règles de Codage pour le Projet Kite PiloteV3

## Principes Généraux

- **Clarté avant optimisation** : Privilégier un code clair et lisible avant d'optimiser les performances.
- **Documentation approfondie** : Chaque composant et fonction doit être documenté avec son but, ses paramètres et valeurs de retour.
- **Tests systématiques** : Toute nouvelle fonctionnalité doit être accompagnée de tests unitaires et d'intégration.
- **Gestion des erreurs** : Anticiper et gérer toutes les erreurs possibles de manière robuste et documentée, en utilisant les `ErrorCode` définis dans `common/global_enums.h`.
- **Approche non-bloquante** : Utiliser des machines à états finis (FSM) et éviter les appels bloquants comme `delay()`. Les états des composants sont gérés via `ComponentState` de `common/global_enums.h`.
- **Programmation Orientée Objet (POO)** : Structurer le projet en utilisant des classes et des objets pour une meilleure modularité et réutilisabilité. La classe de base `ManagedComponent` (`core/component.h`) est centrale.
- **Communication Inter-Tâches (IPC)** : Utiliser la structure `IPCMessage` et les `MessageType` (`common/ipc_message.h` et `common/global_enums.h`) pour les échanges entre tâches FreeRTOS.

## Résumé de la Refactorisation (Mai 2025)

**Objectif Principal :** Refactorisation complète du projet Kite PiloteV3 pour améliorer la clarté, la robustesse, la maintenabilité et s'aligner sur une nouvelle architecture modulaire orientée services, centrée sur `ManagedComponent`.

**Architecture Cible :**
*   Couches : HAL, Services, Application, Présentation.
*   Composant central : `ManagedComponent` pour tous les éléments gérables.
*   Configuration : `core/config.h` pour les valeurs par défaut, configuration spécifique à l'instanciation.
*   Activation des composants : Gérée par leur instanciation et initialisation via la nouvelle API.

**État d'Avancement (Finalisé) :**

*   **Réalisé :**
    *   Définition de l'architecture complète basée sur `ManagedComponent`.
    *   Création et implémentation des énumérations globales (`ComponentState`, `ErrorCode`, `MessageType`, `TaskPriority`, `LogLevel`).
    *   Création et implémentation de la classe `ManagedComponent` comme base pour tous les composants.
    *   Création et implémentation de la couche HAL avec les classes `HALComponent`, `SensorComponent`, `ActuatorComponent`, `InputComponent`, `OutputComponent`.
    *   Implémentation des pilotes HAL pour tous les périphériques (IMU, servo, display, buttons, potentiometers, etc.).
    *   Refactorisation du `TaskManager` pour gérer le cycle de vie des `ManagedComponent`.
    *   Refactorisation du `SystemOrchestrator` pour coordonner les différents composants.
    *   Suppression des flags d'activation globaux au profit d'une gestion dynamique des composants.
    *   Mise en place d'un système robuste de communication entre composants via `IPCMessage`.
    *   Mise à jour de la documentation pour refléter la nouvelle architecture.

## Architecture Cible (Détaillée)

L'architecture est organisée en couches distinctes :

1. **Couche d'Abstraction Matérielle (HAL)** : 
   
   Pilotes de bas niveau qui abstrait tous les périphériques matériels. Chaque pilote hérite d'une classe spécifique selon sa fonction :
   - `HALComponent` : Classe de base pour tous les composants matériels.
   - `SensorComponent` : Pour les capteurs (IMU, tension, vent, etc.).
   - `ActuatorComponent` : Pour les actionneurs (servos, treuil, etc.).
   - `InputComponent` : Pour les périphériques d'entrée (boutons, potentiomètres, etc.).
   - `OutputComponent` : Pour les périphériques de sortie (écran LCD, LEDs, etc.).

2. **Couche Services Système & Cœur** :
   
   Composants transversaux qui assurent le fonctionnement du système :
   - `TaskManager` : Gestion des tâches FreeRTOS pour les `ManagedComponent`.
   - `SystemOrchestrator` : Coordination et séquencement des opérations du système.
   - `SystemStateManager` : Gestion de l'état global du système.
   - `LoggingModule` : Journalisation à différents niveaux (ERROR, WARNING, INFO, DEBUG).
   - `ErrorHandler` : Gestion centralisée des erreurs.

3. **Couche Services Applicatifs** :
   
   Services implémentant la logique métier :
   - `SensorService` : Agrégation et traitement des données capteurs.
   - `ControlService` : Algorithmes de contrôle et logique de pilotage.
   - `UIService` : Gestion de l'interface utilisateur.
   - `CommunicationService` : Gestion des communications (WiFi, API, etc.).

4. **Couche Application** :
   
   Logique spécifique à l'application du cerf-volant :
   - `Autopilot` : Algorithmes de pilotage automatique.
   - `SafetyMonitor` : Surveillance et gestion de la sécurité.
   - `DataRecorder` : Enregistrement des données et statistiques.

5. **Couche Présentation** :
   
   Gestion de l'interface utilisateur :
   - `UIManager` : Gestion globale de l'interface utilisateur.
   - `DisplayPresenter` : Affichage sur écran LCD.
   - `DashboardPresenter` : Interface web pour le tableau de bord.

## Structure des Fichiers

La structure des fichiers reflète l'architecture en couches :

```
/include
  /common
    - global_enums.h       // Énumérations globales
    - ipc_message.h        // Structure pour communication inter-tâches
  /core
    - component.h          // Classe de base ManagedComponent
    - config.h             // Configuration globale
    - task_manager.h       // Gestionnaire des tâches
    - system_orchestrator.h // Orchestrateur du système
    - logging.h           // Module de journalisation centralisé
    - system.h             // Fonctions système de haut niveau
  /hal
    - hal_component.h      // Classes de base pour HAL
    /drivers
      - imu_driver.h       // Pilote IMU
      - servo_driver.h     // Pilote Servo
      // ... autres pilotes
  /services
    - sensor_service.h     // Service capteurs
    - control_service.h    // Service contrôle
    // ... autres services
  /ui
    - dashboard.h          // Interface tableau de bord
    // ... autres interfaces

/src
  /core
    - main.cpp             // Point d'entrée principal
    - config.cpp
    - task_manager.cpp
    - system_orchestrator.cpp
    - system.cpp
  /hal
    /drivers
      - imu_driver.cpp
      - servo_driver.cpp
      // ... autres pilotes
  /services
    - sensor_service.cpp
    - control_service.cpp
    // ... autres services
  /ui
    - dashboard.cpp
    // ... autres interfaces
```

## Bonnes Pratiques d'Implémentation

1. **Initialisation des Composants** :
   - Toujours utiliser la méthode `initialize()` qui retourne un `ErrorCode`.
   - Vérifier le résultat de l'initialisation avant d'utiliser un composant.

2. **Gestion des Erreurs** :
   - Utiliser les codes d'erreur définis dans `global_enums.h`.
   - Journaliser les erreurs avec le bon niveau de log.
   - Ne jamais laisser une erreur non gérée.

3. **Communication Entre Composants** :
   - Utiliser `IPCMessage` pour la communication asynchrone.
   - Éviter les dépendances circulaires entre composants.
   - Préférer les interfaces bien définies aux appels directs.

4. **Ressources Matérielles** :
   - Accéder aux ressources matérielles uniquement via les pilotes HAL.
   - Initialiser et libérer correctement les ressources.
   - Gérer les conflits d'accès avec mutex ou sémaphores si nécessaire.

5. **Tâches FreeRTOS** :
   - Respecter les priorités définies dans `global_enums.h`.
   - Utiliser `TaskManager` pour gérer les tâches des composants.
   - Éviter le blocage prolongé des tâches critiques.