#pragma once

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace mrpc {
namespace generator {

// 用于存储参数信息的结构体
struct Parameter {
  std::string name;
  std::string type;
};

// 用于存储方法信息的结构体
struct Method {
  std::string name;
  std::vector<Parameter> request_params;
  std::vector<Parameter> response_params;
};

// 用于存储服务信息的结构体
struct Service {
  std::string name;
  std::vector<Method> methods;
};

// 存根生成器基类
class StubGeneratorBase {
protected:
  std::string yaml_filename; // 不含扩展名的yaml文件名
  Service service;
  std::stringstream output;

  // 辅助函数：将字符串首字母大写
  std::string capitalize(const std::string &str) {
    if (str.empty())
      return str;
    std::string result = str;
    result[0] = std::toupper(result[0]);
    return result;
  }

  // 从路径中提取yaml文件名（不含扩展名）
  void extractYamlFilename(const std::string &yaml_path) {
    size_t lastSlash = yaml_path.find_last_of("/\\");
    size_t lastDot = yaml_path.find_last_of(".");

    if (lastSlash == std::string::npos) {
      lastSlash = 0;
    } else {
      lastSlash++;
    }

    if (lastDot == std::string::npos || lastDot < lastSlash) {
      yaml_filename = yaml_path.substr(lastSlash);
    } else {
      yaml_filename = yaml_path.substr(lastSlash, lastDot - lastSlash);
    }
  }

  // 纯虚函数：生成方法名数组
  virtual void generateMethodNames() = 0;

  // 纯虚函数：生成请求和响应结构体
  virtual void generateStructs() = 0;

  // 纯虚函数：生成客户端类
  virtual void generateClient() = 0;

  // 纯虚函数：生成服务端抽象基类
  virtual void generateService() = 0;

public:
  StubGeneratorBase(const std::string &yaml_path) {
    extractYamlFilename(yaml_path);
  }

  virtual ~StubGeneratorBase() = default;

  const std::string GetGenFile() const { return output.str(); }

  // 解析yaml文件
  bool parseYaml(const std::string &yaml_path) {
    try {
      YAML::Node config = YAML::LoadFile(yaml_path);

      service.name = config["service"]["name"].as<std::string>();

      const YAML::Node &methods = config["service"]["methods"];
      for (const auto &method : methods) {
        Method m;
        m.name = method.first.as<std::string>();

        // 解析请求参数
        auto request = method.second["request"];
        for (const auto &param : request) {
          Parameter p;
          p.name = param.first.as<std::string>();
          p.type = param.second.as<std::string>();
          m.request_params.push_back(p);
        }

        // 解析响应参数
        auto response = method.second["response"];
        for (const auto &param : response) {
          Parameter p;
          p.name = param.first.as<std::string>();
          p.type = param.second.as<std::string>();
          m.response_params.push_back(p);
        }

        service.methods.push_back(m);
      }
      return true;
    } catch (const YAML::Exception &e) {
      std::cerr << "Error parsing YAML file: " << e.what() << std::endl;
      return false;
    }
  }

  // 生成存根文件
  virtual bool generate() = 0;
};

} // namespace generator
} // namespace mrpc