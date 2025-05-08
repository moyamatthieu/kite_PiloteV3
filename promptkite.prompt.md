# Règles de Codage pour le Projet Kite Pilote V3

## Principes Généraux

- **Clarté avant optimisation** : Privilégier un code clair et lisible avant d'optimiser les performances.
- **Documentation approfondie** : Chaque module et fonction doit être documenté avec son but, ses paramètres et valeurs de retour.
- **Tests systématiques** : Toute nouvelle fonctionnalité doit être accompagnée de tests unitaires et d'intégration.
- **Gestion des erreurs** : Anticiper et gérer toutes les erreurs possibles de manière robuste et documentée.
- **Approche non-bloquante** : Utiliser des machines à états finis (FSM) et éviter les appels bloquants comme `delay()`.

## Structure des Fichiers

### En-tête Standardisé

Tous les fichiers doivent inclure un en-tête suivant ce format :

```c
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

  Principes de fonctionnement :
  1. [Principe 1]
  2. [Principe 2]

  Interactions avec d'autres modules :
  - [Module A] : [Description de l'interaction]
  - [Module B] : [Description de l'interaction]
*/
```

### Organisation des Dossiers

- **`src/`** : Contient les fichiers source organisés par module (communication, contrôle, matériel, etc.).
- **`include/`** : Contient les fichiers d'en-tête correspondants.
- **`docs-fr/`** : Documentation en français, y compris la vision complète du projet.
- **`test/`** : Tests unitaires et d'intégration.
- **`data/`** : Fichiers statiques pour l'interface web.

## Architecture du Projet

### Modèle MCP (Model-Controller-Presenter)

- **Model** : Gestion des données et logique métier.
- **Controller** : Orchestration des actions et gestion des événements.
- **Presenter** : Interface utilisateur et affichage des données.

### Multitâche avec FreeRTOS

- **Tâches dédiées** : Chaque fonctionnalité critique doit être gérée par une tâche FreeRTOS.
- **Synchronisation** : Utiliser des mutex, sémaphores et queues pour éviter les conflits.
- **Priorités cohérentes** : Les tâches critiques doivent avoir une priorité supérieure à 5 (échelle 0-10).

### Gestion des Capteurs et Actionneurs

- **Initialisation non-bloquante** : Utiliser des FSM pour l'initialisation.
- **Détection des défaillances** : Implémenter des mécanismes de récupération automatique.
- **Calibration documentée** : Inclure les étapes de calibration dans un fichier README spécifique.

## Bonnes Pratiques de Développement

### Compilation et Tests

- **Compilation fréquente** : Compiler après chaque modification significative.
- **Analyse des warnings** : Traiter les warnings comme des erreurs.
- **Tests automatisés** : Utiliser PlatformIO pour exécuter des tests unitaires et d'intégration.

### Gestion des Erreurs

- **Validation des entrées** : Vérifier systématiquement la validité des données entrantes.
- **Journalisation efficace** : Utiliser les niveaux de log (INFO, WARNING, ERROR, DEBUG).
- **Propagation cohérente** : Propager les erreurs au niveau approprié.

### Documentation

- **API publique** : Documenter clairement les interfaces publiques.
- **FSM** : Décrire chaque état et transition dans les commentaires.
- **Modules** : Inclure des diagrammes UML pour les modules complexes.

## Implémentation des FSM

- **Éviter `delay()`** : Utiliser `millis()` ou `vTaskDelayUntil` pour gérer les délais.
- **Timeout systématique** : Implémenter un timeout pour chaque étape.
- **Réinitialisation après succès** : Réinitialiser l'état pour permettre une nouvelle exécution.

## Processus de Développement

1. **Planification** : Définir les objectifs et tâches dans un outil comme Trello.
2. **Conception** : Créer des diagrammes UML et spécifications.
3. **Implémentation** : Respecter les bonnes pratiques et effectuer des commits fréquents.
4. **Compilation et Tests** : Vérifier la compilation et exécuter les tests.
5. **Revue** : Utiliser des pull requests pour les revues de code.
6. **Intégration** : Fusionner dans la branche principale après validation.
7. **Documentation** : Mettre à jour la documentation technique et utilisateur.
8. **Rétrospective** : Analyser les points d'amélioration après chaque itération.

## Outils Recommandés

- **IDE** : Visual Studio Code avec PlatformIO.
- **Linting** : `clang-tidy` ou `cppcheck`.
- **Formatage** : `clang-format` pour un style de code cohérent.
- **CI/CD** : Automatiser les tests avec GitHub Actions.
- **Débogage** : Utiliser ESP-IDF Monitor ou JTag pour les problèmes complexes.
- **Documentation** : Générer des documents avec Doxygen.

## Sécurité et Performances

- **Validation des données** : Vérifier toutes les données externes.
- **Protection des ressources** : Utiliser des mutex pour les sections critiques.
- **Optimisation ciblée** : N'optimiser que les parties identifiées comme critiques après profilage.
- **Surveillance des ressources** : Implémenter des mécanismes pour surveiller l'utilisation CPU/mémoire.

## Gestion des Versions

- **Branche principale** : Pousser directement après validation des tests.
- **Historique des builds** : Conserver un journal des erreurs pour identifier les problèmes récurrents.

## Conclusion

Ces règles visent à garantir un code robuste, maintenable et performant pour le projet Kite Pilote V3. Respectez-les pour assurer la qualité et la pérennité du projet.
