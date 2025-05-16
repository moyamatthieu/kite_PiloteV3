# Plan de Reprise du Projet Kite PiloteV3

**Phase 1 : Résolution des Bloquants de Compilation et Finalisation de l'Intégration des Services de Base**

1.  **Résoudre les Problèmes de Chemins d'Inclusion (Prioritaire) :**
    *   **Objectif :** S'assurer que `actuator_service.h` et d'autres fichiers peuvent correctement inclure les en-têtes des pilotes HAL (par exemple, `servo_driver.h`, `winch_driver.h`).
    *   **Actions :**
        *   Vérifier les noms de fichiers exacts et les chemins des pilotes HAL dans `include/hal/drivers/`.
        *   Examiner la configuration du système de build (par exemple, `platformio.ini` ou `c_cpp_properties.json` dans VSCode) pour les `includePath` ou `lib_deps`. C'est la cause la plus probable des erreurs actuelles.
        *   Si la modification directe des fichiers de build n'est pas possible via les outils, essayer à nouveau les conventions de chemin d'inclusion standard (par exemple, `"hal/drivers/servo_driver.h"` depuis un fichier dans `include/services/`).
2.  **Finaliser l'Implémentation du Singleton `ActuatorService` :**
    *   **Objectif :** S'assurer que `ActuatorService` est un singleton correctement implémenté.
    *   **Actions :** Examiner `actuator_service.h` et `actuator_service.cpp` en les comparant au modèle de singleton de `SystemOrchestrator`.
3.  **Finaliser l'Intégration de `ControlService` avec `ActuatorService` :**
    *   **Objectif :** `ControlService` doit utiliser le singleton `ActuatorService` pour envoyer des commandes aux actionneurs.
    *   **Actions :**
        *   Dans `control_service.h`, s'assurer que `ActuatorService.h` est inclus (une fois les chemins corrigés).
        *   Dans `control_service.cpp`, obtenir l'instance de `ActuatorService` (probablement dans `initialize()`).
        *   Dans `applyActuatorCommands()`, utiliser l'instance de `ActuatorService` pour appeler ses méthodes de contrôle (par exemple, `setWinchSpeed()`, `setServoAngle()`).
4.  **Implémenter la Logique de `ControlService::executeControlLoop()` :**
    *   **Objectif :** Développer la logique pour chaque `ControlStrategy` (MANUAL, LINE_TENSION_HOLD, ALTITUDE_HOLD, etc.).
    *   **Actions :** Utiliser `latestSensorData` et `currentTargets`, ainsi que les contrôleurs PID, pour calculer les commandes d'actionneur.

**Phase 2 : Développement des Services Restants**

Pour chaque service (`CommunicationService`, `UIService`, `PowerManagementService`) :
1.  **Définir la Configuration :** Ajouter les constantes nécessaires à `core/config.h` (priorités de tâches, tailles de pile, paramètres spécifiques).
    *   Vérifier/ajouter les définitions pour `SYSTEM_MONITOR_STACK_SIZE`, `SYSTEM_MONITOR_PRIORITY` et `SYSTEM_MONITOR_INTERVAL_MS` utilisés dans `task_manager.cpp`.
2.  **Créer le Fichier d'En-tête (`include/services/nom_service.h`) :**
    *   Hériter de `ManagedComponent`.
    *   Définir les énumérations, structures de données/commandes spécifiques.
    *   Déclarer l'interface publique.
    *   Implémenter en tant que singleton si c'est un service central.
3.  **Créer le Fichier d'Implémentation (`src/services/nom_service.cpp`) :**
    *   Implémenter le constructeur, le destructeur, `initialize()`, `run()`, `shutdown()`.
    *   Implémenter l'interface publique.
    *   Interagir avec les pilotes HAL ou d'autres services.

    *   **Détails pour `CommunicationService` :**
        *   Gérer WiFi, ESP-NOW, WebServer.
        *   Traiter les commandes entrantes et la télémétrie sortante.
    *   **Détails pour `UIService` (ou `DisplayService`/`PresentationService`) :**
        *   Gérer l'écran LCD et les boutons.
        *   Interagir avec `DisplayDriverHAL`, `ButtonsDriverHAL`.
        *   Afficher les données d'autres services. Crucial pour corriger le bug de l'uptime LCD.
    *   **Détails pour `PowerManagementService` :**
        *   Surveiller la tension de la batterie.
        *   Fournir des données à `SystemOrchestrator`.
        *   Peut déclencher des états `POWER_SAVE`.

**Phase 3 : Refactorisation des Couches Application et Présentation**

1.  **Identifier et Refactoriser la Logique Applicative :**
    *   Déterminer la logique principale de l'application (par exemple, modes de vol avancés).
    *   Envisager de créer un `ApplicationManager` (héritant de `ManagedComponent`). `autopilot.h/.cpp` pourrait être une base.
2.  **Logique de Présentation :**
    *   Principalement couverte par `UIService` et `KiteWebserver`. S'assurer qu'ils sont bien des `ManagedComponent`.
    *   Les anciens `ui_manager.h`, `display_manager.h` seront probablement remplacés ou intégrés.

**Phase 4 : Mise à Jour de `main.cpp`**

1.  **Séquence d'Initialisation :**
    *   Initialiser les singletons principaux (`LoggingModule`, `ErrorHandler`, `SystemStateManager`, `SystemOrchestrator`, `TaskManager`).
    *   Créer les instances de tous les services et autres `ManagedComponent`.
    *   Enregistrer tous les `ManagedComponent` auprès du `TaskManager`.
    *   Appeler `TaskManager::begin()` et `TaskManager::startManagedTasks()`.
2.  **Boucle Principale :** Deviendra probablement très simple, s'appuyant sur l'ordonnancement de FreeRTOS.

**Phase 5 : Correction du Bug de l'Uptime LCD**

1.  **Localiser la Logique d'Affichage :** Dans le nouveau `UIService`.
2.  **Source des Données :** Utiliser `SystemOrchestrator::getInstance()->getSystemInfo().uptimeSeconds`.
3.  **Mécanisme de Mise à Jour :** S'assurer que `UIService` récupère et affiche périodiquement cette donnée.

**Phase 6 : Tests et Validation**

1.  **Compilation Complète.**
2.  **Analyse Statique et Correction des Avertissements.**
3.  **Tests Unitaires (si applicable).**
4.  **Tests d'Intégration sur Matériel/Simulateur :**
    *   Démarrage, flux de données capteurs, contrôle des actionneurs, transitions d'état, affichage LCD, communication.

**Phase 7 : Documentation et Nettoyage du Code**

1.  **Mettre à Jour `README.md` et `promptkite.prompt.md`.**
2.  **Commentaires dans le Code (Doxygen).**
3.  **Cohérence du Style de Code.**
4.  **Supprimer l'Ancien Code Inutilisé.**

**Phase 8 : Commit des Modifications**
