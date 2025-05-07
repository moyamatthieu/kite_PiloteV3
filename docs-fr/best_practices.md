# Bonnes Pratiques de Développement pour Kite PiloteV3

Ce document centralise les bonnes pratiques de développement pour le projet Kite PiloteV3, avec un accent particulier sur les techniques de programmation non-bloquante adaptées aux systèmes temps réel basés sur FreeRTOS.

## Table des matières
1. [Machines à États Finis (FSM)](#machines-à-états-finis)
2. [FreeRTOS - Bonnes pratiques](#freertos---bonnes-pratiques)
3. [Gestion de la mémoire](#gestion-de-la-mémoire)
4. [Documentation standardisée](#documentation-standardisée)
5. [Utilisation du système de logging](#utilisation-du-système-de-logging)
6. [Modèles de conception](#modèles-de-conception)

## Machines à États Finis (FSM)

### Pourquoi utiliser des FSM plutôt que delay()

Dans un système temps réel multitâche, l'utilisation de `delay()` présente plusieurs problèmes majeurs :
- Blocage du thread entier pendant la durée du délai
- Impossibilité d'exécuter d'autres opérations pendant le délai
- Réduction de la réactivité globale du système
- Incompatibilité avec la nature même des systèmes multitâches

Les machines à états finis (FSM) sont l'alternative recommandée :
- Permettent l'exécution d'opérations asynchrones sans bloquer
- Maintiennent la réactivité du système
- Divisent les opérations complexes en étapes simples et gérables
- Facilitent la gestion des erreurs et des timeouts
- Permettent une meilleure utilisation des ressources du système

### Structure d'une FSM typique

```cpp
enum State {
  STATE_IDLE,
  STATE_START,
  STATE_WAIT_RESPONSE,
  STATE_PROCESS,
  STATE_COMPLETE,
  STATE_ERROR
};

bool processFSM() {
  static State currentState = STATE_IDLE;
  static unsigned long lastStateTime = 0;
  static uint8_t retryCount = 0;
  const unsigned long timeout = 5000; // 5 secondes de timeout
  
  // Obtenir le temps actuel une seule fois par appel
  unsigned long currentTime = millis();
  
  switch (currentState) {
    case STATE_IDLE:
      // Initialisation
      lastStateTime = currentTime;
      retryCount = 0;
      currentState = STATE_START;
      return false; // Pas terminé
      
    case STATE_START:
      // Lancer une opération
      if (startOperation()) {
        lastStateTime = currentTime;
        currentState = STATE_WAIT_RESPONSE;
      } else {
        // Gestion d'erreur
        if (++retryCount > 3) {
          currentState = STATE_ERROR;
        }
      }
      return false; // Pas terminé
      
    case STATE_WAIT_RESPONSE:
      // Vérifier si assez de temps s'est écoulé
      if (currentTime - lastStateTime >= 100) { // 100ms d'attente
        // Vérifier si une réponse est disponible
        if (isResponseAvailable()) {
          lastStateTime = currentTime;
          currentState = STATE_PROCESS;
        } else if (currentTime - lastStateTime >= timeout) {
          // Timeout
          currentState = STATE_ERROR;
        }
      }
      return false; // Pas terminé
      
    case STATE_PROCESS:
      // Traiter la réponse
      if (processResponse()) {
        currentState = STATE_COMPLETE;
      } else {
        currentState = STATE_ERROR;
      }
      return false; // Pas terminé
      
    case STATE_COMPLETE:
      // Opération terminée avec succès
      currentState = STATE_IDLE; // Réinitialiser pour la prochaine utilisation
      return true; // Terminé avec succès
      
    case STATE_ERROR:
      // Gérer l'erreur
      LOG_ERROR("FSM", "Erreur dans la FSM après %d tentatives", retryCount);
      currentState = STATE_IDLE; // Réinitialiser pour la prochaine utilisation
      return false; // Terminé avec erreur
      
    default:
      // État inconnu
      currentState = STATE_IDLE;
      return false;
  }
}
```

### Bonnes pratiques pour l'implémentation des FSM

1. **Nommage explicite des états** : Utilisez des enums ou des constantes pour nommer clairement les états
   ```cpp
   enum WifiState {
     WIFI_IDLE,
     WIFI_CONNECTING,
     WIFI_CONNECTED,
     WIFI_ERROR
   };
   ```

2. **Variables d'état statiques** : Utilisez des variables static pour conserver l'état entre les appels
   ```cpp
   static WifiState currentState = WIFI_IDLE;
   static unsigned long lastStateTime = 0;
   ```

3. **Timeouts systématiques** : Implémentez des timeouts pour chaque état qui attend une condition externe
   ```cpp
   if (currentTime - lastStateTime >= CONNECTION_TIMEOUT) {
     currentState = WIFI_ERROR;
     LOG_WARNING("WIFI", "Timeout de connexion");
   }
   ```

4. **Tentatives multiples** : Prévoyez des mécanismes de retry avec compteur
   ```cpp
   if (++retryCount <= MAX_RETRIES) {
     currentState = WIFI_CONNECTING;
     LOG_INFO("WIFI", "Nouvelle tentative %d/%d", retryCount, MAX_RETRIES);
   } else {
     currentState = WIFI_ERROR;
   }
   ```

5. **Réinitialisation explicite** : Réinitialisez l'état après succès ou échec pour permettre une réutilisation
   ```cpp
   // Avant de quitter la fonction
   if (currentState == WIFI_CONNECTED || currentState == WIFI_ERROR) {
     currentState = WIFI_IDLE;
   }
   ```

6. **Valeurs de retour cohérentes** : Utilisez un système cohérent pour indiquer l'état d'achèvement
   ```cpp
   // Retourne true uniquement quand la FSM est complètement terminée avec succès
   return (currentState == WIFI_CONNECTED);
   ```

7. **Logging détaillé** : Documentez les transitions d'état importantes via le système de logging
   ```cpp
   LOG_DEBUG("FSM", "Transition: %s -> %s", 
             getStateName(previousState), getStateName(currentState));
   ```

8. **Évitez les paramètres inutiles** : Préférez les variables statiques internes plutôt que des paramètres

9. **Anticipez les wraparounds** : Utilisez des comparaisons qui gèrent correctement le wraparound de millis()
   ```cpp
   // Bon (gère le wraparound)
   if (currentTime - lastStateTime >= TIMEOUT) { ... }
   
   // Mauvais (problème si millis() fait un wraparound)
   if (currentTime >= lastStateTime + TIMEOUT) { ... }
   ```

10. **Une seule action par état** : Limitez chaque état à une seule action significative

### Exemples concrets de FSM dans le projet

#### Exemple 1: Initialisation d'un servomoteur

Voir l'implémentation dans `src/hardware/actuators/servo.cpp` - La fonction `servoInitAll()` utilise une FSM pour:
1. Rechercher des timers PWM disponibles
2. Allouer des timers
3. Configurer les servomoteurs
4. Attacher et initialiser les servos de direction, trim et modulation de ligne

#### Exemple 2: Connexion WiFi non-bloquante

Voir l'implémentation dans `src/communication/wifi_manager.cpp` - Les états typiques sont:
1. IDLE: État initial
2. CONNECTING: Tentative de connexion en cours
3. CONNECTED: Connexion établie
4. RECONNECTING: Tentative de reconnexion après perte de connexion
5. AP_MODE: Basculement en mode point d'accès si la connexion échoue

## FreeRTOS - Bonnes pratiques

### Priorités des tâches

La hiérarchie des priorités dans notre projet suit les principes suivants:

| Priorité | Type de tâche | Exemples |
|----------|---------------|----------|
| Élevée (5-6) | Tâches critiques temps réel | Contrôle des moteurs, communication de sécurité |
| Moyenne (3-4) | Tâches importantes | Interface utilisateur, lecture des capteurs |
| Basse (1-2) | Tâches de fond | Logging, surveillance système, communication non-critique |

Règles importantes:
- Évitez les inversions de priorité en utilisant l'héritage de priorité dans les mutex
- Évitez de bloquer indéfiniment dans les tâches de haute priorité
- Utilisez des timeout pour toutes les opérations bloquantes

### Communication inter-tâches

Utilisez les primitives FreeRTOS appropriées selon le besoin:

| Mécanisme | Utilisation recommandée |
|-----------|-------------------------|
| Queue | Transfert de données entre tâches |
| Sémaphore binaire | Signalisation d'événements |
| Mutex | Protection des ressources partagées |
| Notification de tâche | Alternative légère aux sémaphores binaires |

```cpp
// Exemple d'utilisation d'une queue
QueueHandle_t dataQueue;
dataQueue = xQueueCreate(10, sizeof(SensorData));

// Envoyer des données (avec timeout)
SensorData data = readSensor();
if (xQueueSend(dataQueue, &data, pdMS_TO_TICKS(100)) != pdPASS) {
  LOG_WARNING("QUEUE", "File pleine, données perdues");
}

// Recevoir des données (avec timeout)
SensorData receivedData;
if (xQueueReceive(dataQueue, &receivedData, pdMS_TO_TICKS(100)) == pdPASS) {
  processData(receivedData);
}
```

### Surveillance des tâches

Surveillez régulièrement l'état de vos tâches pour détecter les problèmes:

```cpp
void printTaskStats() {
  TaskStatus_t *taskStatusArray;
  UBaseType_t taskCount;
  uint32_t totalRuntime;
  
  // Allouer de la mémoire pour les statistiques
  taskCount = uxTaskGetNumberOfTasks();
  taskStatusArray = (TaskStatus_t*)pvPortMalloc(taskCount * sizeof(TaskStatus_t));
  
  if (taskStatusArray != NULL) {
    // Obtenir les statistiques
    taskCount = uxTaskGetSystemState(taskStatusArray, taskCount, &totalRuntime);
    
    LOG_INFO("TASKS", "%-20s %-8s %-5s %-10s", "Nom", "État", "Prio", "Stack High");
    LOG_INFO("TASKS", "----------------------------------------");
    
    for (UBaseType_t i = 0; i < taskCount; i++) {
      LOG_INFO("TASKS", "%-20s %-8d %-5d %-10d", 
               taskStatusArray[i].pcTaskName,
               taskStatusArray[i].eCurrentState,
               taskStatusArray[i].uxCurrentPriority,
               taskStatusArray[i].usStackHighWaterMark);
    }
    
    vPortFree(taskStatusArray);
  }
}
```

## Gestion de la mémoire

### Éviter les allocations dynamiques

Préférez:
- Buffers statiques prédimensionnés
- Pools de mémoire préallouée
- Variables statiques locales aux fonctions
- Tableaux de taille fixe

Évitez:
- `new` et `delete`
- `malloc()` et `free()`
- Objets String (utilisez des char[] avec snprintf)

### Utilisation de la mémoire statique pour les FSM

```cpp
// Bon exemple - Variables statiques locales à la fonction
bool connectWifi() {
  static uint8_t state = 0;
  static uint8_t retryCount = 0;
  static unsigned long lastAttemptTime = 0;
  
  // Code de la FSM...
}
```

### Vérification des limites

Toujours vérifier les limites avant d'accéder à un tableau:

```cpp
// Mauvais
buffer[index] = value;

// Bon
if (index < BUFFER_SIZE) {
  buffer[index] = value;
} else {
  LOG_ERROR("BUFFER", "Tentative d'accès hors limites: %d >= %d", index, BUFFER_SIZE);
}
```

### Utilisation de snprintf pour la sécurité

```cpp
// Mauvais - Risque de débordement de buffer
char buffer[50];
sprintf(buffer, "Valeur: %d, Texte: %s", value, text);

// Bon - Taille limitée et valeur de retour vérifiée
char buffer[50];
int written = snprintf(buffer, sizeof(buffer), "Valeur: %d, Texte: %s", value, text);
if (written >= sizeof(buffer)) {
  LOG_WARNING("BUFFER", "Troncature du message (buffer trop petit)");
}
```

## Documentation standardisée

### En-têtes de fichiers

Tous les fichiers doivent inclure un en-tête standardisé avec ces sections:
1. Identification et description du module
2. Version, date et auteurs
3. Section "FONCTIONNEMENT" décrivant en détail le module
4. Principes de fonctionnement (liste numérotée)
5. Interactions avec d'autres modules
6. Aspects techniques notables

Voir le fichier `promptkite.prompt.md` pour le modèle exact.

### Documentation des fonctions

```cpp
/**
 * Initialise la connexion WiFi en mode non-bloquant
 *
 * Cette fonction démarre le processus de connexion WiFi mais retourne
 * immédiatement. L'état de la connexion peut être vérifié via isWifiConnected().
 * Utilise une machine à états finis (FSM) interne pour gérer le processus.
 *
 * @param ssid Nom du réseau WiFi (doit être non-NULL)
 * @param password Mot de passe du réseau (peut être NULL pour un réseau ouvert)
 * @param timeout_ms Timeout en millisecondes (0 = pas de timeout)
 * @return true si le processus a démarré correctement, false sinon
 */
bool beginWifiConnection(const char* ssid, const char* password, unsigned long timeout_ms = 0);
```

### Documentation des FSM

Pour les FSM complexes, ajoutez un diagramme d'états dans les commentaires:

```cpp
/**
 * FSM de gestion du WiFi
 *
 * États:
 * - IDLE: En attente de commande
 * - CONNECTING: Tentative de connexion en cours
 * - CONNECTED: Connexion établie
 * - RECONNECTING: Tentative de reconnexion après perte
 * - AP_MODE: Mode point d'accès
 *
 * Transitions:
 * IDLE -> CONNECTING: Appel à beginWifiConnection()
 * CONNECTING -> CONNECTED: Connexion réussie
 * CONNECTING -> RECONNECTING: Échec mais tentatives restantes
 * CONNECTING -> AP_MODE: Échec après toutes les tentatives
 * CONNECTED -> RECONNECTING: Perte de connexion détectée
 * RECONNECTING -> CONNECTED: Reconnexion réussie
 * RECONNECTING -> AP_MODE: Échec après toutes les tentatives
 * * -> IDLE: Appel à disconnectWifi()
 */
bool updateWifiConnection();
```

## Utilisation du système de logging

Le système de journalisation offre plusieurs niveaux de verbosité et de formatage:

| Niveau | Macro | Utilisation |
|--------|-------|------------|
| ERROR | LOG_ERROR | Erreurs critiques empêchant le fonctionnement |
| WARNING | LOG_WARNING | Problèmes non-critiques mais nécessitant attention |
| INFO | LOG_INFO | Informations importantes sur le fonctionnement normal |
| DEBUG | LOG_DEBUG | Informations détaillées pour le débogage |

```cpp
// Exemples d'utilisation
LOG_ERROR("WIFI", "Impossible de se connecter au réseau %s", ssid);
LOG_WARNING("SERVO", "Position demandée hors limites: %d°", angle);
LOG_INFO("SYSTEM", "Démarrage du système, version %s", VERSION);
LOG_DEBUG("FSM", "Transition d'état: %d -> %d", oldState, newState);
```

### Directives pour le logging

1. **Contexte clair**: Utilisez un tag descriptif pour chaque message
2. **Messages concis**: Informations essentielles uniquement
3. **Niveau approprié**: Utilisez le niveau correspondant à l'importance
4. **Variables importantes**: Incluez les valeurs pertinentes
5. **Pas de logging excessif**: Évitez le spam en production

## Modèles de conception

### Observateur (Observer Pattern)

Utilisé pour les notifications de changements d'état:

```cpp
// Définition de l'interface d'observateur
class SystemStateObserver {
public:
  virtual void onSystemStateChanged(uint8_t newState) = 0;
};

// Classe observable
class System {
private:
  std::vector<SystemStateObserver*> observers;
  uint8_t state;
  
public:
  void addObserver(SystemStateObserver* observer) {
    observers.push_back(observer);
  }
  
  void setState(uint8_t newState) {
    if (state != newState) {
      state = newState;
      notifyObservers();
    }
  }
  
  void notifyObservers() {
    for (auto observer : observers) {
      observer->onSystemStateChanged(state);
    }
  }
};
```

### Singleton

Utilisé pour les gestionnaires globaux uniques:

```cpp
class LogManager {
private:
  // Constructeur privé
  LogManager() {
    // Initialisation
  }
  
  // Instance unique
  static LogManager* instance;
  
public:
  // Méthode d'accès à l'instance
  static LogManager* getInstance() {
    if (instance == nullptr) {
      instance = new LogManager();
    }
    return instance;
  }
  
  void log(const char* tag, const char* message) {
    // Implémentation
  }
};

// Initialisation de l'instance statique
LogManager* LogManager::instance = nullptr;
```

---

Document écrit par: Équipe Kite PiloteV3
Dernière mise à jour: 7 mai 2025