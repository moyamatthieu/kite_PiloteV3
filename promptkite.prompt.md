# Règles de Codage pour le Projet Kite Pilote V3
   PARLE EN FRANçAIS !!!!


## Principes généraux

- **Clarté avant optimisation**: Privilégier un code clair et lisible avant d'optimiser les performances
- **Documentation approfondie**: Chaque module et fonction doit être documenté avec son but, ses paramètres et valeurs de retour
- **Tests systématiques**: Toute nouvelle fonctionnalité doit être accompagnée de tests appropriés
- **Gestion des erreurs**: Anticiper et gérer toutes les erreurs possibles de manière gracieuse

## Vérification de la compilation

- **Compilation régulière**: Lancer des compilations régulièrement pendant le développement pour détecter les erreurs au plus tôt
- **Correction immédiate**: Tout problème de compilation doit être résolu avant de continuer le développement
- **Analyse des warnings**: Traiter les warnings comme des erreurs et les corriger systématiquement
- **Suivi de l'utilisation mémoire**: Surveiller l'utilisation de la RAM et de la Flash à chaque compilation
- **Tests unitaires**: Exécuter les tests unitaires après chaque compilation réussie

## Structure du code

- **Architecture MVC**: Séparer les modèles, les vues et les contrôleurs
- **Modules découplés**: Minimiser les dépendances entre modules
- **Interface claire**: Définir des interfaces stables entre les composants
- **Cohésion maximale**: Chaque classe ne doit avoir qu'une seule responsabilité

## Nommage et formatage

- **CamelCase pour les fonctions et méthodes**: `maFonction()`, `calculerPosition()`
- **PascalCase pour les classes**: `SensorManager`, `TaskController`
- **snake_case pour les variables**: `line_length`, `tension_value`
- **UPPER_CASE pour les constantes**: `MAX_TENSION`, `DEFAULT_FREQUENCY`
- **Indentation de 4 espaces**: Ne pas utiliser de tabulations
- **Accolades sur la même ligne**: `if (condition) {`

## Commentaires et documentation

- **En-tête de fichier**: Auteur, date, licence, description
- **Documentation des fonctions**: But, paramètres, valeurs de retour, effets secondaires
- **Commentaires pour le code complexe**: Expliquer les algorithmes et les choix d'implémentation
- **Éviter les commentaires évidents**: Ne pas commenter ce qui est évident dans le code
- **Documentation des API**: Documenter clairement les API publiques

## Gestion des erreurs

- **Vérification des valeurs de retour**: Toujours vérifier les valeurs de retour des fonctions
- **Logging approprié**: Utiliser les différents niveaux de log (INFO, WARNING, ERROR, DEBUG)
- **Propagation cohérente**: Propager les erreurs au niveau approprié
- **Récupération gracieuse**: Prévoir des stratégies de récupération après erreur

## Communication et protocoles

- **Protocoles documentés**: Documenter tous les formats de message
- **Vérification des données**: Valider toutes les données reçues avant traitement
- **Timeout systématiques**: Implémenter des timeouts pour toutes les communications
- **Gestion des déconnexions**: Prévoir des stratégies en cas de perte de connexion

## Performances et ressources

- **Optimisation ciblée**: N'optimiser que les parties critiques après profilage
- **Économie de mémoire**: Minimiser l'allocation dynamique de mémoire
- **Gestion des tâches**: Respecter les priorités et ne pas bloquer les tâches critiques
- **Surveillance des ressources**: Implémenter des mécanismes de surveillance de l'utilisation CPU/mémoire

## Sécurité

- **Validation des entrées**: Valider toutes les entrées, même celles supposées sûres
- **Protection contre les débordements**: Vérifier les limites des tableaux et des buffers
- **Sécurisation des communications**: Utiliser le chiffrement quand c'est possible
- **Gestion des accès concurrents**: Protéger les ressources partagées avec des mutex appropriés

## Processus de développement

- **Branching stratégique**: Utiliser les branches pour les nouvelles fonctionnalités et corrections
- **Revue de code systématique**: Faire réviser tout nouveau code par un autre développeur
- **Tests avant fusion**: S'assurer que tous les tests passent avant de fusionner du code
- **Intégration continue**: Automatiser les tests et les déploiements

## Spécificités FreeRTOS

- **Priorités cohérentes**: Attribuer des priorités cohérentes aux tâches
- **Délais non-bloquants**: Utiliser vTaskDelay au lieu de délais bloquants
- **Protection des ressources**: Utiliser les sémaphores et mutex appropriés
- **Surveillance de la stack**: Surveiller l'utilisation de la stack de chaque tâche
- **Notification plutôt que sémaphore**: Privilégier les notifications pour les signaux simples

## Capteurs et actuateurs

- **Calibration documentée**: Documenter les procédures de calibration
- **Plages de valeurs**: Définir et respecter les plages de valeurs acceptables
- **Filtrage approprié**: Implémenter un filtrage adapté à chaque type de capteur
- **Détection des défaillances**: Mettre en place des mécanismes de détection de défaillance
- **Modes dégradés**: Prévoir des modes de fonctionnement dégradés en cas de défaillance

## Itération de développement

1. **Planification**: Définir clairement les objectifs de l'itération
2. **Conception**: Concevoir les nouvelles fonctionnalités ou modifications
3. **Implémentation**: Coder en respectant les règles ci-dessus
4. **Compilation et tests**: Vérifier régulièrement que le code compile et passe les tests
5. **Revue**: Faire réviser le code par un autre développeur
6. **Intégration**: Fusionner le code dans la branche principale
7. **Validation**: Valider le fonctionnement sur le matériel cible
8. **Documentation**: Mettre à jour la documentation technique et utilisateur
9. **Rétrospective**: Analyser ce qui a bien fonctionné et ce qui peut être amélioré