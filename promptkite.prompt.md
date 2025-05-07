# Règles de Codage pour le Projet Kite Pilote V3
PARLE EN FRANÇAIS !!!!


## Principes généraux

- **Clarté avant optimisation**: Privilégier un code clair et lisible avant d'optimiser les performances
- **Documentation approfondie**: Chaque module et fonction doit être documenté avec son but, ses paramètres et valeurs de retour
- **Tests systématiques**: Toute nouvelle fonctionnalité doit être accompagnée de tests appropriés
- **Gestion des erreurs**: Anticiper et gérer toutes les erreurs possibles de manière gracieuse
- **Approche non-bloquante**: Utiliser des machines à états finis (FSM) au lieu de delay() pour toutes les opérations asynchrones

## Structure des en-têtes de fichiers

Tous les fichiers doivent suivre ce format d'en-tête standardisé:

```
/*
  -----------------------
  Kite PiloteV3 - [Nom du Module] (Implémentation/Interface)
  -----------------------
  
  [Brève description du module]
  
  Version: 3.0.0
  Date: [Date actuelle]
  Auteurs: Équipe Kite PiloteV3
  
  ===== FONCTIONNEMENT =====
  [Description détaillée du fonctionnement du module]
  
  Principes de fonctionnement:
  1. [Principe 1]
  2. [Principe 2]
  3. [Principe 3]
  
  Interactions avec d'autres modules:
  - [Module A]: [Description de l'interaction]
  - [Module B]: [Description de l'interaction]
  
  Aspects techniques notables:
  - [Point technique 1]
  - [Point technique 2]
*/
```

## Vérification de la compilation

- **Compilation régulière**: Lancer des compilations régulièrement pendant le développement pour détecter les erreurs au plus tôt
- **Correction immédiate**: Tout problème de compilation doit être résolu avant de continuer le développement
- **Analyse des warnings**: Traiter les warnings comme des erreurs et les corriger systématiquement
- **Suivi de l'utilisation mémoire**: Surveiller l'utilisation de la RAM et de la Flash à chaque compilation
- **Tests unitaires**: Exécuter les tests unitaires après chaque compilation réussie

## Structure du code

- **Architecture MCP**: Séparer les modèles, les contrôleurs et les présentateurs
- **Modules découplés**: Minimiser les dépendances entre modules
- **Interface claire**: Définir des interfaces stables entre les composants
- **Cohésion maximale**: Chaque classe ne doit avoir qu'une seule responsabilité
- **Interdépendances documentées**: Documenter clairement les relations entre modules

## Nommage et formatage

- **CamelCase pour les fonctions et méthodes**: `maFonction()`, `calculerPosition()`
- **PascalCase pour les classes**: `SensorManager`, `TaskController`
- **snake_case pour les variables**: `line_length`, `tension_value`
- **UPPER_CASE pour les constantes**: `MAX_TENSION`, `DEFAULT_FREQUENCY`
- **Indentation de 2 espaces**: Ne pas utiliser de tabulations
- **Accolades sur la même ligne**: `if (condition) {`
- **Préfixes pour les états FSM**: Utiliser des préfixes clairs pour les états d'une FSM, ex: `STATE_INIT`, `STATE_CONNECTING`

## Commentaires et documentation

- **En-tête de fichier**: Auteur, date, licence, description et fonctionnement
- **Documentation des fonctions**: But, paramètres, valeurs de retour, effets secondaires
- **Commentaires pour le code complexe**: Expliquer les algorithmes et les choix d'implémentation
- **Éviter les commentaires évidents**: Ne pas commenter ce qui est évident dans le code
- **Documentation des API**: Documenter clairement les API publiques
- **Documentation des FSM**: Décrire clairement chaque état et les transitions possibles

## Implémentation de machines à états finis (FSM)

- **Éviter delay()**: Ne jamais utiliser delay() dans le code principal, utiliser des FSM à la place
- **Variables d'état statiques**: Utiliser des variables static pour conserver l'état entre les appels
- **Timestamps pour le timing**: Utiliser millis() pour gérer les délais sans bloquer
- **Valeurs de retour claires**: Retourner true quand la FSM est terminée, false quand elle est en cours
- **Réinitialisation après succès**: Réinitialiser l'état à la fin pour permettre une nouvelle exécution
- **Gestion des erreurs**: Prévoir des états d'erreur et des mécanismes de récupération
- **Timeout systématique**: Implémenter un timeout pour chaque étape de la FSM
- **Nommage explicite des états**: Utiliser des constantes ou enum pour nommer les états

## Gestion des erreurs

- **Vérification des valeurs de retour**: Toujours vérifier les valeurs de retour des fonctions
- **Logging approprié**: Utiliser les différents niveaux de log (INFO, WARNING, ERROR, DEBUG)
- **Propagation cohérente**: Propager les erreurs au niveau approprié
- **Récupération gracieuse**: Prévoir des stratégies de récupération après erreur
- **Validation des entrées**: Vérifier systématiquement la validité des données entrantes

## Communication et protocoles

- **Protocoles documentés**: Documenter tous les formats de message
- **Vérification des données**: Valider toutes les données reçues avant traitement
- **Timeout systématiques**: Implémenter des timeouts pour toutes les communications
- **Gestion des déconnexions**: Prévoir des stratégies en cas de perte de connexion
- **Communication non-bloquante**: Implémenter toutes les communications avec des FSM

## Performances et ressources

- **Optimisation ciblée**: N'optimiser que les parties critiques après profilage
- **Économie de mémoire**: Minimiser l'allocation dynamique de mémoire
- **Gestion des tâches**: Respecter les priorités et ne pas bloquer les tâches critiques
- **Surveillance des ressources**: Implémenter des mécanismes de surveillance de l'utilisation CPU/mémoire
- **Tailles de pile appropriées**: Définir les tailles de pile FreeRTOS en fonction des besoins réels

## Sécurité

- **Validation des entrées**: Valider toutes les entrées, même celles supposées sûres
- **Protection contre les débordements**: Vérifier les limites des tableaux et des buffers
- **Sécurisation des communications**: Utiliser le chiffrement quand c'est possible
- **Gestion des accès concurrents**: Protéger les ressources partagées avec des mutex appropriés
- **Validation des données externes**: Valider et sanitiser toutes les données provenant de l'extérieur

## Processus de développement

- **Documentation à jour**: Mettre à jour la documentation en même temps que le code.
  - **Correction**: Ajout d'un exemple pour clarifier ce point : "Exemple : Si une nouvelle fonction est ajoutée, documentez son but, ses paramètres et ses valeurs de retour immédiatement."

## Spécificités FreeRTOS

- **Priorités cohérentes**: Attribuer des priorités cohérentes aux tâches.
  - **Correction**: Ajout d'une note : "Les tâches critiques doivent avoir une priorité supérieure à 5 dans une échelle de 0 à 10."
- **Délais non-bloquants**: Utiliser vTaskDelay ou vTaskDelayUntil au lieu de délais bloquants.
  - **Correction**: Ajout d'un exemple : "Exemple : Utilisez vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(100)) pour des tâches périodiques."
- **Protection des ressources**: Utiliser les sémaphores et mutex appropriés.
  - **Correction**: Ajout d'une précision : "Utilisez xSemaphoreTake et xSemaphoreGive pour protéger les sections critiques."

## Capteurs et actuateurs

- **Calibration documentée**: Documenter les procédures de calibration.
  - **Correction**: Ajout d'une précision : "Inclure les étapes de calibration dans un fichier README spécifique au capteur."
- **Plages de valeurs**: Définir et respecter les plages de valeurs acceptables
- **Filtrage approprié**: Implémenter un filtrage adapté à chaque type de capteur
- **Détection des défaillances**: Mettre en place des mécanismes de détection de défaillance.
  - **Correction**: Ajout d'un exemple : "Exemple : Implémentez un watchdog pour surveiller les capteurs critiques."
- **Modes dégradés**: Prévoir des modes de fonctionnement dégradés en cas de défaillance
- **Initialisation non-bloquante**: Utiliser des FSM pour l'initialisation des capteurs et actuateurs

## Itération de développement

1. **Planification**: Définir clairement les objectifs de l'itération.
   - **Correction**: Ajout d'une précision : "Utilisez un outil de gestion de projet comme Trello ou Jira pour suivre les tâches."
2. **Conception**: Concevoir les nouvelles fonctionnalités ou modifications.
   - **Correction**: Ajout d'une note : "Inclure des diagrammes UML pour les modules complexes."
3. **Implémentation**: Coder en respectant les règles ci-dessus, en particulier l'approche non-bloquante.
   - **Correction**: Ajout d'une précision : "Effectuez des commits fréquents avec des messages clairs."
4. **Compilation et tests**: Vérifier régulièrement que le code compile et passe les tests.
   - **Correction**: Ajout d'une note : "Automatisez les tests avec un outil comme PlatformIO."
5. **Revue**: Faire réviser le code par un autre développeur.
   - **Correction**: Ajout d'une précision : "Utilisez des pull requests pour faciliter les revues."
6. **Intégration**: Fusionner le code dans la branche principale.
   - **Correction**: Ajout d'une note : "Effectuez des tests d'intégration avant de fusionner."
7. **Validation**: Valider le fonctionnement sur le matériel cible.
   - **Correction**: Ajout d'une précision : "Documentez les résultats des tests dans un fichier dédié."
8. **Documentation**: Mettre à jour la documentation technique et utilisateur.
   - **Correction**: Ajout d'une note : "Inclure des captures d'écran ou des vidéos pour les guides utilisateur."
9. **Rétrospective**: Analyser ce qui a bien fonctionné et ce qui peut être amélioré.
   - **Correction**: Ajout d'une précision : "Organisez une réunion de rétrospective avec l'équipe."