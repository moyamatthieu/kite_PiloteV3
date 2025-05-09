# Discussions et refontes - Kite PiloteV3

## 1. Objectif de la refonte
- Passer d'une gestion statique (booléens de compilation) à une architecture orientée objet, dynamique et modulaire.
- Permettre l'activation/désactivation des modules à l'exécution (menu, web, API REST).
- Centraliser l'état et le contrôle des modules via une base `Module` et un `ModuleRegistry`.
- Améliorer la qualité, la maintenabilité et l'évolutivité du code.

## 2. Actions réalisées
- Création d'une classe abstraite `Module` avec gestion d'état, enable/disable, reporting, etc.
- Mise en place d'un singleton `ModuleRegistry` pour enregistrer et itérer dynamiquement sur tous les modules.
- Refactorisation de tous les modules principaux (WiFi, API, Servo, Autopilot, Webserver, Winch, Sensors, OTA) pour hériter de `Module` et s'enregistrer automatiquement.
- Remplacement de la logique d'activation/désactivation des tâches dans `TaskManager` : démarrage/arrêt dynamique basé sur l'état des modules du `ModuleRegistry`.
- Suppression de la dépendance aux booléens globaux `moduleXEnabled` pour la gestion des tâches.
- Ajout de fonctions utilitaires pour activer/désactiver un module par nom et lister l'état de tous les modules (pour menu ou API web).
- Refactorisation de l'affichage de l'état des modules sur LCD et web pour itérer sur le `ModuleRegistry`.
- Migration OOP complète des modules capteurs (IMU, vent, longueur de ligne, tension) et actionneurs (Servo, Treuil) : FSM, gestion d'erreur, configuration, polymorphisme.
- Migration OOP des modules de communication (WiFi, API, Webserver) avec FSM, gestion d'erreur, configuration, et activation dynamique.

## 3. Prochaines étapes possibles
- Finaliser la migration OOP pour l'autopilote et les modules avancés.
- Intégrer la configuration dynamique (JSON, web, etc.) pour chaque module.
- Ajouter des tests unitaires et d'intégration sur la base OOP.
- Continuer à migrer le code procédural vers des patterns orientés objet.

---
Dernière mise à jour : 9 mai 2025
