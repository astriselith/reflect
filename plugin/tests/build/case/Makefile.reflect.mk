# Gerado automaticamente pelo plugin Clang 'reflect'
# Nao edite manualmente.

# @reflect: Behavior.hpp __Behavior.cpp
# @reflect: Player.hpp __Player.cpp
# @reflect: Transform.hpp __Transform.cpp

REFLECT_GENERATED := __Behavior.cpp __Player.cpp __Transform.cpp

__Behavior.cpp: Behavior.hpp
__Player.cpp: Player.hpp
__Transform.cpp: Transform.hpp

.PHONY: reflect-clean
reflect-clean:
	rm -f __Behavior.cpp __Player.cpp __Transform.cpp Makefile.reflect.mk
