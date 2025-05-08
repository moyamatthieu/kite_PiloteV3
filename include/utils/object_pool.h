#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include <Arduino.h>
#include <array>
#include <bitset>

/**
 * Pool d'objets pré-alloués pour éviter les allocations dynamiques fréquentes.
 * Cette classe permet de réutiliser efficacement des objets dans les boucles critiques
 * et d'éviter la fragmentation de la mémoire.
 * 
 * @tparam T Type d'objets à stocker dans le pool
 * @tparam Size Nombre d'objets dans le pool
 */
template<typename T, size_t Size>
class ObjectPool {
private:
    // Tableau d'objets pré-alloués
    std::array<T, Size> objects;
    
    // Bitset pour suivre quels objets sont utilisés (1 = utilisé, 0 = disponible)
    std::bitset<Size> used;
    
    // Mutex pour assurer un accès thread-safe
    SemaphoreHandle_t mutex;

public:
    /**
     * Constructeur
     */
    ObjectPool() {
        // Initialisation du mutex
        mutex = xSemaphoreCreateMutex();
        
        // Initialisation des flags d'utilisation
        used.reset();  // Tous les objets sont disponibles
    }
    
    /**
     * Destructeur
     */
    ~ObjectPool() {
        // Libération du mutex
        if (mutex != nullptr) {
            vSemaphoreDelete(mutex);
            mutex = nullptr;
        }
    }
    
    /**
     * Acquiert un objet du pool
     * @return Pointeur vers un objet disponible, ou nullptr si le pool est plein
     */
    T* acquire() {
        // Protection thread-safe
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
            return nullptr;
        }
        
        // Recherche d'un objet disponible
        for (size_t i = 0; i < Size; ++i) {
            if (!used[i]) {
                used[i] = true;
                xSemaphoreGive(mutex);
                return &objects[i];
            }
        }
        
        // Aucun objet disponible
        xSemaphoreGive(mutex);
        return nullptr;
    }
    
    /**
     * Libère un objet précédemment acquis
     * @param object Pointeur vers l'objet à libérer
     * @return true si l'objet a été libéré avec succès, false sinon
     */
    bool release(T* object) {
        // Vérification que l'objet appartient au pool
        if (object < &objects[0] || object > &objects[Size - 1]) {
            return false;
        }
        
        // Protection thread-safe
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
            return false;
        }
        
        // Calcul de l'index de l'objet dans le pool
        size_t index = object - &objects[0];
        
        // Vérification que l'objet était bien marqué comme utilisé
        if (!used[index]) {
            xSemaphoreGive(mutex);
            return false;
        }
        
        // Libération de l'objet
        used[index] = false;
        
        xSemaphoreGive(mutex);
        return true;
    }
    
    /**
     * Vérifie si un objet est actuellement utilisé
     * @param object Pointeur vers l'objet à vérifier
     * @return true si l'objet est actuellement utilisé, false sinon
     */
    bool isUsed(T* object) {
        // Vérification que l'objet appartient au pool
        if (object < &objects[0] || object > &objects[Size - 1]) {
            return false;
        }
        
        // Protection thread-safe
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
            return false;
        }
        
        // Calcul de l'index de l'objet dans le pool
        size_t index = object - &objects[0];
        bool status = used[index];
        
        xSemaphoreGive(mutex);
        return status;
    }
    
    /**
     * Retourne le nombre d'objets disponibles dans le pool
     * @return Nombre d'objets disponibles
     */
    size_t available() const {
        // Protection thread-safe
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
            return 0;
        }
        
        size_t count = Size - used.count();
        
        xSemaphoreGive(mutex);
        return count;
    }
    
    /**
     * Retourne la capacité totale du pool
     * @return Nombre total d'objets dans le pool
     */
    constexpr size_t capacity() const {
        return Size;
    }
    
    /**
     * Réinitialise le pool, libérant tous les objets
     * Attention: ne doit être utilisé que si aucun objet du pool n'est utilisé
     * @return true si réinitialisation réussie, false sinon
     */
    bool reset() {
        // Protection thread-safe
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
            return false;
        }
        
        used.reset();
        
        xSemaphoreGive(mutex);
        return true;
    }
};

#endif // OBJECT_POOL_H