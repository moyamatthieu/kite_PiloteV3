# Règles de Codage pour le Projet Kite PiloteV3

## Principes Généraux

- **Clarté avant optimisation** : Privilégier un code clair et lisible avant d'optimiser les performances.
- **Documentation approfondie** : Chaque module et fonction doit être documenté avec son but, ses paramètres et valeurs de retour.
- **Tests systématiques** : Toute nouvelle fonctionnalité doit être accompagnée de tests unitaires et d'intégration.
- **Gestion des erreurs** : Anticiper et gérer toutes les erreurs possibles de manière robuste et documentée.
- **Approche non-bloquante** : Utiliser des machines à états finis (FSM) et éviter les appels bloquants comme `delay()`.
- **Programmation Orientée Objet (POO)** : Structurer le projet en utilisant des classes et des objets pour une meilleure modularité et réutilisabilité.

## Architecture des Fichiers

### Structure des Fichiers

```
/src
  /core
    - main.cpp                // Point d'entrée principal
    - task_manager.cpp        // Gestionnaire central des tâches FreeRTOS
    - module_registry.cpp     // Gestionnaire des modules (enregistrement, activation/désactivation)
    - system.cpp              // Gestion des fonctions système (surveillance, état global)
  /communication
    - wifi_manager.cpp        // Gestion des connexions WiFi
    - api_manager.cpp         // Gestion des API REST
    - webserver_manager.cpp   // Gestion du serveur web
  /hardware
    /actuators
      - servo.cpp             // Contrôle des servos
      - winch.cpp             // Gestion du treuil motorisé
    /io
      - display_manager.cpp   // Gestion de l'écran LCD
      - button_manager.cpp    // Gestion des boutons
      - potentiometer_manager.cpp // Gestion des potentiomètres
    /sensors
      - imu.cpp               // Gestion des capteurs IMU
      - wind_sensor.cpp       // Gestion des capteurs de vent
      - tension_sensor.cpp    // Gestion des capteurs de tension
      - line_length_sensor.cpp // Gestion des capteurs de longueur de ligne
  /control
    - autopilot.cpp           // Gestion du pilote automatique
    - pid_controller.cpp      // Contrôleur PID
    - safety_manager.cpp      // Gestion des sécurités
    - trajectory_manager.cpp  // Gestion des trajectoires
  /ui
    - dashboard.cpp           // Interface utilisateur locale (LCD)
    - web_interface.cpp       // Interface utilisateur web
  /utils
    - logging.cpp             // Gestion des logs
    - state_machine.cpp       // Gestion des machines à états
    - error_manager.cpp       // Gestion des erreurs
    - watchdog_manager.cpp    // Gestion du watchdog
/include
  /core
    - main.h
    - task_manager.h
    - module_registry.h
    - system.h
  /communication
    - wifi_manager.h
    - api_manager.h
    - webserver_manager.h
  /hardware
    /actuators
      - servo.h
      - winch.h
    /io
      - display_manager.h
      - button_manager.h
      - potentiometer_manager.h
    /sensors
      - imu.h
      - wind_sensor.h
      - tension_sensor.h
      - line_length_sensor.h
  /control
    - autopilot.h
    - pid_controller.h
    - safety_manager.h
    - trajectory_manager.h
  /ui
    - dashboard.h
    - web_interface.h
  /utils
    - logging.h
    - state_machine.h
    - error_manager.h
    - watchdog_manager.h
```

### Hiérarchie des Classes

```
- Module (classe de base abstraite)
  - Task (hérite de Module)
    - DisplayTask
    - ButtonTask
    - NetworkTask
    - SensorTask
    - ControlTask
  - SensorModule (hérite de Module)
    - IMUSensor
    - WindSensor
    - TensionSensor
    - LineLengthSensor
  - ActuatorModule (hérite de Module)
    - Servo
    - Winch
  - CommunicationModule (hérite de Module)
    - WiFiManager
    - APIManager
    - WebserverManager
  - ControlModule (hérite de Module)
    - Autopilot
    - PIDController
    - SafetyManager
    - TrajectoryManager
  - IOManager (hérite de Module)
    - DisplayManager
    - ButtonManager
    - PotentiometerManager
  - Utils
    - LoggingModule
    - StateMachine
    - ErrorManager
    - WatchdogManager
```

### Points Clés de l'Architecture

1. **Modularité** :
   - Chaque composant matériel ou logiciel est encapsulé dans une classe dérivée de `Module`.
   - Les tâches FreeRTOS sont encapsulées dans des classes dérivées de `Task`.

2. **Gestion Centralisée** :
   - Le `TaskManager` gère toutes les tâches FreeRTOS.
   - Le `ModuleRegistry` gère l'enregistrement et l'état des modules.

3. **Extensibilité** :
   - Ajouter un nouveau capteur ou actionneur nécessite simplement de créer une nouvelle classe dérivée de `SensorModule` ou `ActuatorModule`.

4. **Réutilisabilité** :
   - Les classes utilitaires comme `StateMachine` et `LoggingModule` sont génériques et peuvent être réutilisées dans d'autres projets.

5. **Performance Temps Réel** :
   - Les tâches FreeRTOS sont encapsulées pour garantir une gestion précise des priorités et des ressources.