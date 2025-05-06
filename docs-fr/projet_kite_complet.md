# Projet Autopilote de Kite Générateur d'Électricité

**Version : 1.1.0 — Dernière mise à jour : 6 Mai 2025**

## Introduction

Ce projet vise à développer un système d'autopilote optimisé pour un kite générateur d'électricité basé sur ESP32. Le système contrôle automatiquement un cerf-volant de traction pour produire de l'électricité de manière efficace et sécurisée. Il repose sur des algorithmes avancés, une architecture matérielle robuste et une interface utilisateur intuitive.

### Objectifs principaux

- Maximiser la production d'énergie grâce à des trajectoires de vol optimisées.
- Garantir un fonctionnement sécurisé avec détection et gestion des situations à risque.
- Offrir une interface intuitive pour le contrôle et le monitoring.
- Fonctionner de manière autonome dans diverses conditions météorologiques.
- Optimiser l'efficacité énergétique globale du système.

---

## Guide de Navigation Rapide

- **[Vue d'ensemble](#introduction)** : Concept et objectifs du projet.
- **[Matériel](#2-composants-matériels-clés)** : ESP32, capteurs et actionneurs.
- **[Logiciel](#3-fonctionnalités-logicielles-clés)** : Architecture et fonctionnalités.
- **[Sécurité](#4-système-de-sécurité-intégré)** : Protections et mesures d'urgence.
- **[Architecture](#10-architecture-des-fichiers-du-projet)** : Structure du code.
- **[Diagnostic](#11-systèmes-de-journalisation-diagnostic-et-terminal-distant)** : Outils de débogage.

---

## 1. Objectif Global du Projet

Le projet a pour but de développer un système de pilotage automatique basé sur ESP32 pour un kite (cerf-volant de traction) dans le but de générer de l'électricité de manière efficace et sécurisée. Le système doit :

- Maximiser la production d'énergie via des trajectoires de vol optimisées.
- Garantir un fonctionnement sécurisé avec détection et gestion des situations à risque.
- Offrir une interface intuitive pour le contrôle et le monitoring.
- Fonctionner de manière autonome dans diverses conditions météorologiques.
- Optimiser l'efficacité énergétique globale du système.

---

## 2. Composants Matériels Clés

### 2.1 Contrôleur Principal

- **ESP32 principal** :
  - Modèle recommandé : ESP32-WROOM-32D.
  - RAM : Minimum 4MB pour les buffers et l'interface web.
  - Avantages :
    - Puissance de calcul suffisante pour les algorithmes PID complexes.
    - Connectivité Wi-Fi/BT intégrée.
    - Consommation optimisée en mode deep sleep (<10µA).

### 2.2 Actionneurs

#### 2.2.1 Servo 1 : Direction

- **Fonction** : Contrôle la direction gauche/droite du kite.
- **Spécifications optimales** :
  - Type : Servo digital haute vitesse (>0.10s/60°).
  - Couple : 10-15kg/cm (selon taille du kite).

#### 2.2.2 Servo 2 : Trim

- **Fonction** : Ajuste l'angle d'incidence pour la puissance.
- **Spécifications optimales** :
  - Type : Servo digital haute précision.
  - Couple : 12-18kg/cm.

#### 2.2.3 Servo 3 : Générateur/Treuil

- **Fonction double** :
  - Génération d'énergie (résistance contrôlée).
  - Enroulement/déroulement des lignes.

### 2.3 Capteurs

#### 2.3.1 IMU (Unité de Mesure Inertielle)

- **Type recommandé** : BNO055 avec fusion de capteurs.
- **Avantages** :
  - Fusion de capteurs directement en hardware.
  - Calibration automatique.

#### 2.3.2 Communication IMU-ESP32

- **Solution optimale** : ESP32 miniature sur le kite.
  - Modèle : ESP32-S2 Mini ou ESP32-C3 Mini.
  - Communication : ESP-NOW optimisé (faible latence, <5ms).

#### 2.3.3 Capteur de Tension de Ligne

- **Type optimal** : Cellule de charge amplifiée HX711.
- **Optimisations** :
  - Calibration automatique au démarrage.
  - Filtrage numérique passe-bas optimisé.

---

## 3. Fonctionnalités Logicielles Clés

### 3.1 Autopilote

- **Description** :
  - Contrôle automatique des trajectoires de vol.
  - Algorithmes PID pour ajuster la direction et la puissance.

### 3.2 Gestion de la Génération d'Énergie

- **Description** :
  - Optimisation de la production d'énergie en temps réel.
  - Surveillance des conditions de vent et ajustement dynamique.

### 3.3 Interface Web

- **Description** :
  - Serveur web intégré pour le contrôle et le monitoring.
  - Visualisation des données en temps réel.

---

## 4. Système de Sécurité Intégré

### 4.1 Détection des Risques

- **Description** :
  - Surveillance continue des conditions de vol.
  - Détection des anomalies et déclenchement des procédures d'urgence.

### 4.2 Hiérarchie des Réponses

- **Description** :
  - Réponses graduées en fonction de la gravité des situations.
  - Activation automatique des modes de sécurité.

---

## 10. Architecture des Fichiers du Projet

- **Structure Globale** :
  - `src/` : Contient le code source principal.
  - `include/` : Contient les fichiers d'en-tête.
  - `docs-fr/` : Documentation en français.

---

## 11. Systèmes de Journalisation, Diagnostic et Terminal Distant

### 11.1 Système de Journalisation

- **Description** :
  - Enregistrement des événements critiques pour le débogage.

### 11.2 Système de Diagnostic

- **Description** :
  - Vérification de l'état des capteurs et des actionneurs.

### 11.3 Terminal Distant

- **Description** :
  - Interface pour envoyer des commandes et recevoir des données en temps réel.

---

## Conclusion

Ce projet représente une avancée significative dans le domaine des systèmes autonomes pour la production d'énergie renouvelable. Grâce à une architecture robuste et des fonctionnalités avancées, il offre une solution complète pour maximiser l'efficacité et la sécurité des kites générateurs d'électricité.