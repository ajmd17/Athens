#include <ace-c/Configuration.hpp>

const int ace::compiler::Config::MAX_DATA_MEMBERS = 255;
const std::string ace::compiler::Config::GLOBAL_MODULE_NAME = "global";

bool ace::compiler::Config::lazy_declarations = true;
bool ace::compiler::Config::use_static_objects = true;
bool ace::compiler::Config::cull_unused_objects = false;
int ace::compiler::Config::max_registers = 1;
