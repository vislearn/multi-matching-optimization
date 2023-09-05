#include <random>

namespace mgm {

class RandomSingleton {

    private:
        // Make the default constructor private.
        RandomSingleton () : random_engine(std::random_device()) {};

    public:
        RandomSingleton (const RandomSingleton&) = delete;
        RandomSingleton& operator = (const RandomSingleton&) = delete;

        static RandomSingleton& get() {
            static RandomSingleton instance;
            return instance;
        }

        template<typename T>
        void shuffle(std::vector<T>& vec)
            { std::shuffle(vec.begin(), vec.end(), random_engine); }

        std::random_device random_engine;
};
    
}