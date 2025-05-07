# Règles de Codage

## Bonnes pratiques générales

- **Commentaires** : Toujours ajouter un commentaire descriptif pour chaque élément clé et chaque fonctions du code. Les commentaires doivent expliquer le "pourquoi" et non seulement le "quoi".

- **Bibliothèques externes** : Ne jamais modifier le code des bibliothèques externes. Modifier le code pour qu'il soit compatible avec les librairies. Rechercher la meilleure librairie, surtout si elle est maintenue dans PlatformIO.

- **Compilation** : S'assurer que le code compile sans erreurs avant de le soumettre. Utilisez des outils d'intégration continue pour vérifier automatiquement.

- **Tests de compilation réguliers** : Effectuer des builds fréquents pour détecter et corriger les erreurs de compilation dès leur apparition. Ne pas accumuler trop de modifications non testées.

- **Résolution d'erreurs** : Aborder les erreurs de compilation méthodiquement en comprenant leur cause fondamentale plutôt qu'en appliquant des correctifs superficiels.

- **Performances** : Optimiser le code pour les performances, en particulier sur l'ESP32 qui a une puissance de calcul limitée. Évitez les boucles inutiles et les allocations dynamiques fréquentes.

- **Architecture** : Respecter l'architecture existante du projet et placer les fonctions au bon endroit. Suivez les conventions de nommage et de structure des dossiers.

- **Évolutivité** : Réfléchir en termes de développement futur pour éviter des modifications structurelles plus tard. Utilisez des interfaces et des abstractions pour faciliter les extensions.

## Architecture du projet

- **Objectif principal** : L'objectif lointain est d'arriver à ce projet : `docs-fr/projet_kite_complet.md`. Ce document décrit la vision complète du projet.

- **Architecture MCP** : Utiliser l'architecture MCP (Model-Controller-Presenter) pour structurer le code. Cela permet une séparation claire des responsabilités et facilite la maintenance.

- **Objectifs** : Suivre les objectifs définis dans le fichier `projet_kite_complet.md`. Chaque module doit contribuer à ces objectifs.

- **Multitâche FreeRTOS** : Utiliser le multitâche de FreeRTOS pour les opérations concurrentes. Assurer la synchronisation appropriée entre les tâches via les primitives FreeRTOS (mutex, sémaphores, queues).

- **Mémoire SPIFFS** : Ne pas utiliser la mémoire SPIFFS pour le stockage. Préférez des alternatives comme LittleFS ou des bases de données légères.

- **Organisation** : Respecter la hiérarchie des dossiers et l'organisation logique du code. Chaque module doit rester dans son domaine de responsabilité.

- **Modularité** : Veiller à ce que chaque module soit autonome et réutilisable. Les dépendances entre modules doivent être minimisées.

- **Thread-safety** : Concevoir les fonctions avec la thread-safety en tête quand elles peuvent être appelées de différentes tâches FreeRTOS.

## Méthodologie de travail

- **Réflexion structurée** : Adopter une réflexion organisée et structurée avant de commencer à coder. Utilisez des diagrammes et des spécifications pour planifier.

- **Cycle de développement itératif** : Suivre un cycle de développement itératif: implémentation → compilation → test → correction. Répéter ce cycle pour chaque fonctionnalité.

- **Auto-correction** : Corriger ses propres idées en suivant les instructions générales. Relisez votre code et testez-le avant de demander une revue.

- **Initiative** : Prendre des initiatives pertinentes pour améliorer le projet. Proposez des améliorations mais documentez-les clairement.

- **Anticipation** : Anticiper les besoins futurs lors de l'implémentation de nouvelles fonctionnalités. Pensez à l'évolutivité et à la maintenance.

- **Tests** : Écrire des tests unitaires et d'intégration pour valider le comportement du code. Utilisez des frameworks comme Google Test ou Catch2.

- **Documentation** : Documenter chaque module et chaque fonction. Utilisez des outils comme Doxygen pour générer une documentation lisible.

- **Journalisation des erreurs** : Implémenter une journalisation efficace pour faciliter le diagnostic des problèmes en production.

## Processus de revue de code

- **Critères de qualité** : Vérifiez que le code est lisible, performant, et respecte les conventions du projet.

- **Feedback constructif** : Donnez un feedback constructif et précis. Proposez des solutions aux problèmes identifiés.

- **Validation** : Assurez-vous que tous les tests passent et que la compilation est exempte d'erreurs avant d'approuver une modification.

- **Vérification de la concurrence** : Porter une attention particulière aux problèmes potentiels de concurrence dans un environnement FreeRTOS.

## Gestion des versions

- C'est un projet amateur, push directement les corrections sur la branche main après avoir vérifié la compilation et les tests.

## Vérification du code

- **Compilation fréquente** : Compiler le code régulièrement, idéalement après chaque modification significative, pour détecter rapidement les erreurs.

- **Analyse des warnings** : Traiter les warnings comme des erreurs potentielles et les résoudre systématiquement.

- **Journalisation des builds** : Conserver un historique des erreurs de build pour identifier les problèmes récurrents.

- **Tests de regression** : S'assurer que les corrections n'introduisent pas de nouveaux problèmes dans le code existant.

## Outils recommandés

- **IDE** : Utilisez Visual Studio Code avec les extensions recommandées (PlatformIO, C/C++ IntelliSense, etc.).

- **Linting** : Configurez un linter comme `clang-tidy` ou `cppcheck` pour détecter les erreurs et les mauvaises pratiques.

- **Formatage** : Utilisez un outil de formatage comme `clang-format` pour garantir un style de code cohérent.

- **CI/CD** : Configurez une pipeline CI/CD pour automatiser les tests et les déploiements.

- **Monitoring** : Utilisez des outils comme Serial Monitor ou Logic Analyzer pour déboguer le matériel.

- **Documentation** : Utilisez des outils comme Markdown ou Doxygen pour documenter le projet.

- **Analyseurs statiques** : Utilisez des analyseurs statiques de code pour identifier les problèmes potentiels avant l'exécution.

- **Outils de profiling** : Utilisez des outils de profilage pour identifier les goulots d'étranglement de performance.

- **Débogage** : Maîtrisez les outils de débogage pour ESP32 comme l'ESP-IDF Monitor ou JTag pour les problèmes complexes.