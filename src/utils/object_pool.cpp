#include <array>

class ObjectPool {
private:
    std::array<int, 10> pool;
    std::array<bool, 10> used;

public:
    ObjectPool() {
        used.fill(false);
    }

    int* acquire() {
        for (size_t i = 0; i < pool.size(); ++i) {
            if (!used[i]) {
                used[i] = true;
                return &pool[i];
            }
        }
        return nullptr; // Aucun objet disponible
    }

    void release(int* object) {
        for (size_t i = 0; i < pool.size(); ++i) {
            if (&pool[i] == object) {
                used[i] = false;
                break;
            }
        }
    }
};

void exampleObjectPoolUsage() {
    ObjectPool pool;
    int* obj = pool.acquire();
    if (obj) {
        *obj = 42; // Utilisation de l'objet
        pool.release(obj); // Libération de l'objet
    }
}