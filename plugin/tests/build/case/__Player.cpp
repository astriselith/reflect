// Gerado automaticamente pelo plugin Clang 'reflect'
// Origem: Player.hpp

#include "Player.hpp"
#include "Class.hpp"
#include "Field.hpp"
#include "Method.hpp"
#include "Constructor.hpp"
#include "Destructor.hpp"
#include "Modifier.hpp"
#include "Parameter.hpp"
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

namespace com::engine {

class __Player {
public:
    static void __field_get_0(void* object, void* outValue) {
        auto* instance = reinterpret_cast<com::engine::Player*>(object);
        *static_cast<decltype(com::engine::Player::life)*>(outValue) = instance->life;
    }

    static void __field_set_0(void* object, void* valuePtr) {
        auto* instance = reinterpret_cast<com::engine::Player*>(object);
        if (valuePtr == nullptr) throw std::invalid_argument("Null value for non-pointer field");
        instance->life = *static_cast<decltype(com::engine::Player::life)*>(valuePtr);
    }

    static void __field_get_1(void* object, void* outValue) {
        auto* instance = reinterpret_cast<com::engine::Player*>(object);
        *static_cast<decltype(com::engine::Player::attack)*>(outValue) = instance->attack;
    }

    static void __field_set_1(void* object, void* valuePtr) {
        auto* instance = reinterpret_cast<com::engine::Player*>(object);
        if (valuePtr == nullptr) throw std::invalid_argument("Null value for non-pointer field");
        instance->attack = *static_cast<decltype(com::engine::Player::attack)*>(valuePtr);
    }

    static void __destructor_0(void* object) {
        delete reinterpret_cast<com::engine::Player*>(object);
    }

    static void* __constructor_0(const std::vector<void*>& args) {
        if (args.size() != 1) throw std::invalid_argument("Wrong argument count");
        if (args[0] == nullptr) throw std::invalid_argument("Null argument");
        using __ctor_param_0 = int;
        using __ctor_arg_0 = typename std::remove_reference<__ctor_param_0>::type;
        auto& arg_0 = *static_cast<__ctor_arg_0*>(args[0]);
        com::engine::Player* obj = new com::engine::Player(arg_0);
        return static_cast<void*>(obj);
    }

};

} // namespace com::engine

extern "C" __attribute__((constructor)) void __register_com_engine_Player() {
    if (!Class::hasClass("com::engine::Player")) {
        Class::putClass("com::engine::Player", Class("com::engine::Player"));
    }
    const Class &self = Class::getClass("com::engine::Player");

    std::vector<const Class*> superclasses = {
        &(Class::hasClass("com::engine::Behavior") ? Class::getClass("com::engine::Behavior") : Class::putClass("com::engine::Behavior", Class("com::engine::Behavior"))),
    };

    std::vector<Field> fields = {
        Field(&self, Modifier(Modifier::PUBLIC), &(Class::hasClass("int") ? Class::getClass("int") : Class::putClass("int", Class("int"))), "life", reinterpret_cast<uintptr_t>(&com::engine::__Player::__field_get_0), reinterpret_cast<uintptr_t>(&com::engine::__Player::__field_set_0)),
        Field(&self, Modifier(Modifier::PUBLIC), &(Class::hasClass("int") ? Class::getClass("int") : Class::putClass("int", Class("int"))), "attack", reinterpret_cast<uintptr_t>(&com::engine::__Player::__field_get_1), reinterpret_cast<uintptr_t>(&com::engine::__Player::__field_set_1)),
    };

    std::vector<Method> methods = {
    };

    std::vector<Constructor> constructors = {
        Constructor(&self, Modifier(Modifier::PUBLIC), {
            Parameter("life_", &(Class::hasClass("int") ? Class::getClass("int") : Class::putClass("int", Class("int"))), Modifier(Modifier::NONE)),
        }, reinterpret_cast<uintptr_t>(&com::engine::__Player::__constructor_0)),
    };

    std::vector<Destructor> destructors = {
        Destructor(reinterpret_cast<uintptr_t>(&com::engine::__Player::__destructor_0)),
    };

    Class::putClass("com::engine::Player",
        Class("Player", Modifier(Modifier::PUBLIC), superclasses, fields, methods, constructors, "com::engine", destructors));
}

