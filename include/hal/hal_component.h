/*
  -----------------------
  Kite PiloteV3 - Composant HAL (Hardware Abstraction Layer)
  -----------------------
  
  Classe de base pour tous les composants matériels.
  
  Date: 15 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#pragma once
#include "core/component.h"

/**
 * Classe de base pour tous les composants d'abstraction matérielle (HAL)
 * Hérite de ManagedComponent et ajoute des fonctionnalités spécifiques au matériel.
 */
class HALComponent : public ManagedComponent {
public:
    HALComponent(const char* name, bool enabledByDefault = true) 
        : ManagedComponent(name, enabledByDefault) {}
    
    virtual ~HALComponent() = default;
    
    // Les composants HAL peuvent avoir besoin d'une méthode reset spécifique
    virtual ErrorCode reset() {
        return ErrorCode::OK;
    }
    
    // Les composants HAL peuvent avoir besoin d'un mode diagnostic
    virtual bool runDiagnostic() {
        return true;
    }
    
    // Méthode pour obtenir le nom du type de périphérique
    virtual const char* getDeviceTypeName() const {
        return "HALDevice";
    }
};

/**
 * Classe de base pour les composants de capteurs
 */
class SensorComponent : public HALComponent {
public:
    SensorComponent(const char* name, bool enabledByDefault = true) 
        : HALComponent(name, enabledByDefault) {}
    
    virtual ~SensorComponent() = default;
    
    // Méthode virtuelle pure pour lire le capteur (à implémenter dans les classes dérivées)
    virtual bool read() = 0;
    
    // Type de capteur
    virtual const char* getDeviceTypeName() const override {
        return "Sensor";
    }
};

/**
 * Classe de base pour les composants d'actionneurs
 */
class ActuatorComponent : public HALComponent {
public:
    ActuatorComponent(const char* name, bool enabledByDefault = true) 
        : HALComponent(name, enabledByDefault) {}
    
    virtual ~ActuatorComponent() = default;
    
    // Méthode virtuelle pure pour actionner l'actionneur (à implémenter dans les classes dérivées)
    virtual void actuate() = 0;
    
    // Type d'actionneur
    virtual const char* getDeviceTypeName() const override {
        return "Actuator";
    }
};

/**
 * Classe de base pour les composants d'entrée (boutons, potentiomètres, etc.)
 */
class InputComponent : public HALComponent {
public:
    InputComponent(const char* name, bool enabledByDefault = true) 
        : HALComponent(name, enabledByDefault) {}
    
    virtual ~InputComponent() = default;
    
    // Type de composant d'entrée
    virtual const char* getDeviceTypeName() const override {
        return "Input";
    }
};

/**
 * Classe de base pour les composants de sortie (affichage, LED, etc.)
 */
class OutputComponent : public HALComponent {
public:
    OutputComponent(const char* name, bool enabledByDefault = true) 
        : HALComponent(name, enabledByDefault) {}
    
    virtual ~OutputComponent() = default;
    
    // Type de composant de sortie
    virtual const char* getDeviceTypeName() const override {
        return "Output";
    }
};
