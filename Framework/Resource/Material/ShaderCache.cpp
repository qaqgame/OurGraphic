//
// Created by Glodxy on 2021/12/18.
//

#include "ShaderCache.h"
#include <fstream>
#include <sstream>
#include "Utils/OGLogging.h"

#define SHADER_ROOT_PATH GRAPHIC_ROOT_PATH/shaders
#define INLINE_STR(R) #R
#define STR(R) INLINE_STR(R)

namespace {
static const char* kGetterFile = "Shared/getter.hdr";
static const char* kInputVsFile = "Shared/input_vs.hdr";
static const char* kInputFsFile = "Shared/input_fs.hdr";
static const char* kInputMaterialVsFile = "Shared/material_inputs_vs.hdr";
static const char* kInputMaterialFsFile = "Shared/material_inputs_fs.hdr";
static const char* kBasePassVsFile = "Shared/base_pass.vs";
static const char* kBasePassFsFile = "Shared/base_pass.fs";
const static const char* kFilePath[] = {
    // todo
    "Shared/deferred_light.hdr", // deferred light
    "Shared/directional_light.hdr", // direction light
    "Shared/dynamic_light.hdr", // dynamic light
    "Shared/brdf.fs", // brdf
};
}


namespace our_graph {

std::map<uint32_t, std::string> ShaderCache::shader_variant_data_;
std::map<std::string, std::string> ShaderCache::shader_file_data_;
std::map<std::pair<std::string, uint32_t>, std::vector<uint32_t>> ShaderCache::shader_compiled_data_;

ShaderCache::ShaderType ShaderCache::GetShaderKind(const std::string &file_path) {
  auto pos = file_path.find_last_of('.');
  std::string type = file_path.substr(pos + 1);
  if (type == "vs") {
    return ShaderType::VERTEX;
  } else if (type == "fs") {
    return ShaderType::FRAGMENT;
  }
}

std::vector<uint32_t> ShaderCache::GetCompiledData(const std::string &file_name,
                                                   uint32_t module_key) {
  auto key = std::make_pair(file_name, module_key);
  if (shader_compiled_data_.find(key) != shader_compiled_data_.end()) {
    return shader_compiled_data_[key];
  }
  std::string source = GetDataFromFile(file_name, module_key);
  ShaderType shader_type = GetShaderKind(file_name);

  std::vector<uint32_t> data = CompileFile(file_name, shader_type,
                                           source, false);
  shader_compiled_data_[key] = data;
  return data;
}

std::vector<uint32_t> ShaderCache::CompileFile(const std::string &source_name,
                                               ShaderType kind,
                                               const std::string &source,
                                               bool optimize) {
  shaderc::Compiler compiler;
  shaderc::CompileOptions options;
  // Like -DMY_DEFINE=1
  if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);

  shaderc_shader_kind shaderc_kind;
  switch (kind) {
    case ShaderType::VERTEX: {
      shaderc_kind = shaderc_glsl_vertex_shader;
      break;
    }
    case ShaderType::FRAGMENT: {
      shaderc_kind = shaderc_glsl_fragment_shader;
      break;
    }
  }
  shaderc::SpvCompilationResult module =
      compiler.CompileGlslToSpv(source, shaderc_kind, source_name.c_str(), options);

  if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
    std::ofstream os;
    os.open("out_" + source_name);
    if (os.is_open()) {
      os << source;
    }
    os.close();
    std::cerr << module.GetErrorMessage();
    return std::vector<uint32_t>();
  }

  return {module.cbegin(), module.cend()};
}

std::string ShaderCache::GetDataFromFile(const std::string &file_path, uint32_t module) {
  std::stringstream res;
  if (module != 0) {
    std::string module_content = GetModuleContent(module);
    res << module_content;
  }
  std::string text;
  if (shader_file_data_.find(file_path) != shader_file_data_.end()) {
    text = shader_file_data_[file_path];
  } else {
    text = LoadFromFile(file_path);
    shader_file_data_[file_path] = text;
  }
  res << text;
  return res.str();
}

std::string ShaderCache::LoadFromFile(const std::string &file_path) {
  std::ifstream file;
  std::string path = STR(SHADER_ROOT_PATH);
  path = path + "/" + file_path;
  file.open(path, std::ios::in);
  if (!file.is_open()) {
    LOG_ERROR("ShaderCache", "Open {} Failed!", path);
    return "";
  }
  std::istreambuf_iterator<char> begin(file), end;
  std::string text(begin, end);
  file.close();
  return text;
}

std::string ShaderCache::GetModuleContent(uint32_t module_key) {

  std::stringstream ss;
  for (uint32_t idx = 0; idx < ShaderVariantBit::MAX_BIT; ++idx) {
    if (((1 << idx) & module_key) != 0) {
      ss << shader_variant_data_[idx];
      ss << "\n";
    }
  }
  return ss.str();
}

std::string ShaderCache::GetGetterData() {
  return GetDataFromFile(kGetterFile);
}

std::string ShaderCache::GetVsInputData() {
  return GetDataFromFile(kInputVsFile);
}

std::string ShaderCache::GetFsInputData() {
  return GetDataFromFile(kInputFsFile);
}

std::string ShaderCache::GetMaterialInputVsData() {
  return GetDataFromFile(kInputMaterialVsFile);
}
std::string ShaderCache::GetMaterialInputFsData() {
  return GetDataFromFile(kInputMaterialFsFile);
}

std::string ShaderCache::GetBasePassVsData() {
  return GetDataFromFile(kBasePassVsFile);
}
std::string ShaderCache::GetBasePassFsData() {
  return GetDataFromFile(kBasePassFsFile);
}


void ShaderCache::Init() {
   // 加载内置模块的shader
  size_t module_file_size = sizeof(kFilePath) / sizeof(const char*);
  for (size_t idx = 0; idx < module_file_size; ++idx) {
    shader_variant_data_[idx] = LoadFromFile(kFilePath[idx]);
  }
}



}