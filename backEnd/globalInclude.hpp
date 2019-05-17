#ifndef globalInclude__hpp
#define globalInclude__hpp

#include <map>
#include <set>
#include <vector>
#include <string>
#include <list>
#include <mutex>
#include <iostream>
#include <cstdint>
#include <thread>
#include <chrono>
#include <functional>
#include "rapidjson/fwd.h"
enum DataBindingsType{ guiToOpcua, opcuaToGui, guiToGui };
enum GuiElementType{ button, label, numeric, textInput, selector};//only examples there have to by an custom variable for every type
namespace rj = rapidjson;
#include "util.hpp"
#endif
