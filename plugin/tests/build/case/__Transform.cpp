// Gerado automaticamente pelo plugin Clang 'reflect'
// Origem: Transform.hpp

#include "Transform.hpp"
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

class __Transform {
public:
    static void __field_get_0(void* object, void* outValue) {
        auto* instance = reinterpret_cast<com::engine::Transform*>(object);
        *static_cast<decltype(com::engine::Transform::x)*>(outValue) = instance->x;
    }

    static void __field_set_0(void* object, void* valuePtr) {
        auto* instance = reinterpret_cast<com::engine::Transform*>(object);
        if (valuePtr == nullptr) throw std::invalid_argument("Null value for non-pointer field");
        instance->x = *static_cast<decltype(com::engine::Transform::x)*>(valuePtr);
    }

    static void __field_get_1(void* object, void* outValue) {
        auto* instance = reinterpret_cast<com::engine::Transform*>(object);
        *static_cast<decltype(com::engine::Transform::y)*>(outValue) = instance->y;
    }

    static void __field_set_1(void* object, void* valuePtr) {
        auto* instance = reinterpret_cast<com::engine::Transform*>(object);
        if (valuePtr == nullptr) throw std::invalid_argument("Null value for non-pointer field");
        instance->y = *static_cast<decltype(com::engine::Transform::y)*>(valuePtr);
    }

    static void __field_get_2(void* object, void* outValue) {
        auto* instance = reinterpret_cast<com::engine::Transform*>(object);
        *static_cast<decltype(com::engine::Transform::z)*>(outValue) = instance->z;
    }

    static void __field_set_2(void* object, void* valuePtr) {
        auto* instance = reinterpret_cast<com::engine::Transform*>(object);
        if (valuePtr == nullptr) throw std::invalid_argument("Null value for non-pointer field");
        instance->z = *static_cast<decltype(com::engine::Transform::z)*>(valuePtr);
    }

    static void __destructor_0(void* object) {
        delete reinterpret_cast<com::engine::Transform*>(object);
    }

    static void* __constructor_0(const std::vector<void*>& args) {
        if (args.size() != 3) throw std::invalid_argument("Wrong argument count");
        if (args[0] == nullptr) throw std::invalid_argument("Null argument");
        using __ctor_param_0 = int;
        using __ctor_arg_0 = typename std::remove_reference<__ctor_param_0>::type;
        auto& arg_0 = *static_cast<__ctor_arg_0*>(args[0]);
        if (args[1] == nullptr) throw std::invalid_argument("Null argument");
        using __ctor_param_1 = int;
        using __ctor_arg_1 = typename std::remove_reference<__ctor_param_1>::type;
        auto& arg_1 = *static_cast<__ctor_arg_1*>(args[1]);
        if (args[2] == nullptr) throw std::invalid_argument("Null argument");
        using __ctor_param_2 = int;
        using __ctor_arg_2 = typename std::remove_reference<__ctor_param_2>::type;
        auto& arg_2 = *static_cast<__ctor_arg_2*>(args[2]);
        com::engine::Transform* obj = new com::engine::Transform(arg_0, arg_1, arg_2);
        return static_cast<void*>(obj);
    }

};

} // namespace com::engine

extern "C" __attribute__((constructor)) void __register_com_engine_Transform() {
    if (!Class::hasClass("com::engine::Transform")) {
        Class::putClass("com::engine::Transform", Class("com::engine::Transform"));
    }
    const Class &self = Class::getClass("com::engine::Transform");

    std::vector<const Class*> superclasses = {
        &(Class::hasClass("com::engine::Behavior") ? Class::getClass("com::engine::Behavior") : Class::putClass("com::engine::Behavior", Class("com::engine::Behavior"))),
    };

    std::vector<Field> fields = {
        Field(&self, Modifier(Modifier::PRIVATE), &(Class::hasClass("int") ? Class::getClass("int") : Class::putClass("int", Class("int"))), "x", reinterpret_cast<uintptr_t>(&com::engine::__Transform::__field_get_0), reinterpret_cast<uintptr_t>(&com::engine::__Transform::__field_set_0)),
        Field(&self, Modifier(Modifier::PRIVATE), &(Class::hasClass("int") ? Class::getClass("int") : Class::putClass("int", Class("int"))), "y", reinterpret_cast<uintptr_t>(&com::engine::__Transform::__field_get_1), reinterpret_cast<uintptr_t>(&com::engine::__Transform::__field_set_1)),
        Field(&self, Modifier(Modifier::PRIVATE), &(Class::hasClass("int") ? Class::getClass("int") : Class::putClass("int", Class("int"))), "z", reinterpret_cast<uintptr_t>(&com::engine::__Transform::__field_get_2), reinterpret_cast<uintptr_t>(&com::engine::__Transform::__field_set_2)),
    };

    std::vector<Method> methods = {
    };

    std::vector<Constructor> constructors = {
        Constructor(&self, Modifier(Modifier::PRIVATE), {
            Parameter("x_", &(Class::hasClass("int") ? Class::getClass("int") : Class::putClass("int", Class("int"))), Modifier(Modifier::NONE)),
            Parameter("y_", &(Class::hasClass("int") ? Class::getClass("int") : Class::putClass("int", Class("int"))), Modifier(Modifier::NONE)),
            Parameter("z_", &(Class::hasClass("int") ? Class::getClass("int") : Class::putClass("int", Class("int"))), Modifier(Modifier::NONE)),
        }, reinterpret_cast<uintptr_t>(&com::engine::__Transform::__constructor_0)),
    };

    std::vector<Destructor> destructors = {
        Destructor(reinterpret_cast<uintptr_t>(&com::engine::__Transform::__destructor_0)),
    };

    Class::putClass("com::engine::Transform",
        Class("Transform", Modifier(Modifier::PUBLIC), superclasses, fields, methods, constructors, "com::engine", destructors));
}

