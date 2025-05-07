# Projet Autopilote de Kite Générateur d'Électricité

*Version: 1.1.0 — Dernière mise à jour: 1 Mai 2025*

## Guide de Navigation Rapide
- **[Vue d'ensemble](#introduction)** - Concept et objectifs du projet
- **[Matériel](#2-composants-matériels-clés)** - ESP32, capteurs et actionneurs
- **[Logiciel](#3-fonctionnalités-logicielles-clés)** - Architecture et fonctionnalités
- **[Sécurité](#4-système-de-sécurité-intégré)** - Protections et mesures d'urgence
- **[Architecture](#10-architecture-des-fichiers-du-projet)** - Structure du code
- **[Diagnostic](#11-systèmes-de-journalisation-diagnostic-et-terminal-distant)** - Outils de débogage


## Introduction

Ce projet développe un système d'autopilote optimisé pour kite générateur d'électricité basé sur ESP32. Le système contrôle automatiquement un cerf-volant de traction pour produire de l'électricité de manière efficace et sécurisée, en utilisant des algorithmes sophistiqués et une architecture matérielle robuste.
```mermaid
graph TD
  A[Direction du Vent] -->|^ ^ ^| B((Kite))
  B -->|Ligne Gauche| C[Station au Sol]
  B -->|Ligne Droite| C

  subgraph "Kite (avec IMU + ESP32)"
    B
  end

  subgraph "Station au Sol"
    C --> D[Capteurs]
    C --> E[Contrôle ESP32 Autopilot]
    C --> F[Interface Web/LCD]
    C --> G[Actionneurs]
    C --> H[Système d'Alimentation]

    D[Capteurs]
    D -->|Tension| D1
    D -->|Longueur| D2
    D -->|Vent| D3

    G[Actionneurs]
    G -->|Direction (Servo1)| G1
    G -->|Trim (Servo2)| G2
    G -->|Générateur (Servo3)| G3

    H[Système d'Alimentation]
    H -->|Batterie| H1
    H -->|Générateur| H2
    H -->|Gestion| H3
  end
```

## 1. Objectif Global du Projet

Développer un système de pilotage automatique basé sur ESP32 pour un kite (cerf-volant de traction) dans le but de générer de l'électricité de manière efficace et sécurisée. Le système doit:

- Maximiser la production d'énergie via des trajectoires de vol optimisées
- Garantir un fonctionnement sécurisé avec détection et gestion des situations à risque
- Offrir une interface intuitive pour le contrôle et le monitoring
- Fonctionner de manière autonome dans diverses conditions météorologiques
- Optimiser l'efficacité énergétique globale du système

## Gestion des Variables et Configuration dans le Projet

### Variables Globales de Configuration

#### Quand centraliser dans un fichier `config.h` :

- **Paramètres communs et configurables** : Si vous avez des paramètres qui affectent plusieurs modules (par exemple, les pins utilisés, les constantes de calibrage, ou des flags de débogage), il est pratique de les centraliser dans un seul fichier. Cela facilite la modification et la maintenance, surtout si vous devez réajuster la configuration matérielle ou logicielle sans plonger dans chaque module.
- **Cohérence** : Avoir un point d’accès unique aux paramètres réduit les risques de divergence ou d’incohérences entre les modules.
- **Facilité de portabilité** : Un fichier de configuration centralisé permet de réutiliser facilement votre code dans un autre projet en adaptant uniquement ce fichier.

#### Attention :

- **Risque de surcharge** : Si vous placez trop de variables dans `config.h`, cela peut vite devenir un monolithe difficile à gérer et à comprendre.
- **Encapsulation** : Certaines variables n’ont d’intérêt que pour un module particulier. Les centraliser peut nuire à la clarté en exposant des détails qui devraient rester internes au module.

### Variables Définies dans chaque Module

#### Quand définir localement dans chaque module (ex. `display_manager.h`, `button_ui.h`, `potentiometre_manager.h`) :

- **Encapsulation et Modularité** : Pour maintenir un découplage fort entre les modules, il peut être préférable que chaque module gère ses propres variables internes. Cela permet de masquer les détails d’implémentation et de limiter l’impact de modifications internes.
- **Code plus lisible et autonome** : Chaque module expose uniquement ce qui est nécessaire (via des interfaces publiques) sans révéler ses variables internes, ce qui rend le code plus maintenable et réutilisable.
- **Réduction des dépendances globales** : Limiter l’accès aux variables globales aide à éviter des effets de bord indésirables lorsqu’un module modifie une valeur à laquelle d’autres dépendent.

#### Attention :

- **Duplication éventuelle** : S’il y a des constantes ou des paramètres communs à plusieurs modules, définir ces variables localement peut entraîner une duplication. Dans ce cas, il vaut mieux les centraliser pour garder la cohérence.

### Conclusion Recommandée

Adoptez une approche mixte :

- **Centralisez dans `config.h`** :
  - Les paramètres globaux comme les pins, constantes de timing, seuils de détection, flags de débogage et autres éléments partagés entre plusieurs modules.

- **Localisez dans chaque module** :
  - Les variables internes ou les états propres à la logique et à la gestion d’un module (par exemple, l’état d’un afficheur ou d’un bouton), qui ne concernent pas l’ensemble du projet.

Cette approche permet de tirer le meilleur parti des avantages de la centralisation (facilité de configuration et de modification globale) tout en conservant une bonne encapsulation et modularité. Ainsi, vous facilitez la maintenance et la compréhension du code par d’autres développeurs ou par vous-même plus tard.

### Pour aller plus loin :

- **Utiliser des namespaces ou des classes** : Regroupez les variables et fonctions qui appartiennent à un même composant, surtout si vous travaillez avec du C++ sur Arduino.
- **Documenter clairement votre `config.h`** : Assurez-vous que les utilisateurs (ou vous-même) sachent précisément l’impact de chaque paramètre.
- **Automatiser une vérification lors des compilations** : Décelez les conflits ou les incohérences potentielles entre modules.

### Transition vers des Configurations Dynamiques

En résumé, à mesure que vos projets Arduino se complexifient, il devient judicieux d'adopter des outils de configuration plus dynamiques pour :

- **Accélérer le prototypage et le débogage** : Ajustez rapidement des paramètres sans avoir à recompiler le code.
- **Améliorer l'ergonomie du système** : Permettez une adaptation en temps réel aux conditions d'utilisation.
- **Faciliter la maintenance et la scalabilité** : Centralisez et déportez une partie du contrôle de la configuration hors du code compilé.

Cette transition illustre l'évolution naturelle d'un projet, passant d'une configuration statique à une configuration adaptative, capable de répondre aux besoins changeants de l'application. Cela offre une meilleure expérience, tant pour les développeurs que pour les utilisateurs finaux.

### Explorations Futures

- **Bibliothèques dédiées** : Utilisez des bibliothèques spécialisées pour gérer des configurations dynamiques sur Arduino.
- **Interfaces web** : Implémentez des interfaces web pour des projets IoT, permettant une gestion intuitive et à distance des paramètres.

Quelles autres fonctionnalités aimeriez-vous voir évoluer dans une telle gestion de configuration ?