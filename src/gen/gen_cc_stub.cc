#include "gen_stub.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace mrpc {
namespace generator {

class CppStubGenerator : public StubGeneratorBase {
private:
  std::string namespace_name;

  // 生成头文件保护和包含声明
  void generateHeader() {
    output << "#pragma once\n\n";
    output << "#include \"mrpcpp/server.h\"\n";
    output << "#include \"mrpcpp/client.h\"\n";
    output << "#include <string>\n\n";
    output << "using json = nlohmann::json;\n\n";
  }

  // 生成命名空间开始
  void generateNamespaceStart() {
    output << "namespace " << namespace_name << " {\n\n";
  }

  // 生成方法名数组
  void generateMethodNames() override {
    output << "static const char *" << service.name << "_method_names[] = {\n";
    for (const auto &method : service.methods) {
      output << "    \"/" << namespace_name << "." << service.name << "/"
             << method.name << "\",\n";
    }
    output << "};\n\n";
  }

  // 生成参数的JSON处理代码
  std::string generateJsonCode(const std::vector<Parameter> &params,
                               bool isToJson) {
    std::stringstream ss;
    if (isToJson) {
      ss << "return json{";
      for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0)
          ss << ",";
        ss << "{\"" << params[i].name << "\", " << params[i].name << "}";
      }
      ss << "};";
    } else {
      for (const auto &param : params) {
        std::string defaultValue;
        if (param.type == "string")
          defaultValue = "\"\"";
        else if (param.type == "int")
          defaultValue = "0";
        else if (param.type == "float")
          defaultValue = "0.0f";
        else if (param.type == "bool")
          defaultValue = "false";

        ss << param.name << " = j.value(\"" << param.name << "\", "
           << defaultValue << "); ";
      }
    }
    return ss.str();
  }

  // 生成构造函数参数列表
  std::string generateConstructorParams(const std::vector<Parameter> &params) {
    std::stringstream ss;
    for (size_t i = 0; i < params.size(); ++i) {
      if (i > 0)
        ss << ", ";
      if (params[i].type == "string")
        ss << "std::string " << params[i].name;
      else
        ss << params[i].type << " " << params[i].name;
    }
    return ss.str();
  }

  // 生成初始化列表
  std::string generateInitList(const std::vector<Parameter> &params) {
    std::stringstream ss;
    for (size_t i = 0; i < params.size(); ++i) {
      if (i > 0)
        ss << ", ";
      ss << params[i].name << "(" << params[i].name << ")";
    }
    return ss.str();
  }

  // 生成请求/响应类
  void generateStructs() override {
    for (const auto &method : service.methods) {
      // 请求类
      output << "class " << method.name << "Request : public mrpc::Parser {\n";
      output << "public:\n";
      output << "  " << method.name << "Request() {}\n";
      output << "  " << method.name << "Request("
             << generateConstructorParams(method.request_params)
             << ") : " << generateInitList(method.request_params) << " {}\n\n";

      output << "private:\n";
      output << "  json toJson() const override { "
             << generateJsonCode(method.request_params, true) << " }\n";
      output << "  void fromJson(const json &j) override { "
             << generateJsonCode(method.request_params, false) << "}\n\n";

      output << "public:\n";
      for (const auto &param : method.request_params) {
        if (param.type == "string")
          output << "  std::string " << param.name << ";\n";
        else
          output << "  " << param.type << " " << param.name << ";\n";
      }
      output << "};\n\n";

      // 响应类
      output << "class " << method.name << "Response : public mrpc::Parser {\n";
      output << "public:\n";
      output << "  " << method.name << "Response() {}\n";
      output << "  " << method.name << "Response("
             << generateConstructorParams(method.response_params)
             << ") : " << generateInitList(method.response_params) << " {}\n\n";

      output << "private:\n";
      output << "  json toJson() const override { "
             << generateJsonCode(method.response_params, true) << " }\n";
      output << "  void fromJson(const json &j) override { "
             << generateJsonCode(method.response_params, false) << "}\n\n";

      output << "public:\n";
      for (const auto &param : method.response_params) {
        if (param.type == "string")
          output << "  std::string " << param.name << ";\n";
        else
          output << "  " << param.type << " " << param.name << ";\n";
      }
      output << "};\n\n";
    }
  }

  // 生成Stub类
  void generateClient() override {
    output << "class " << service.name << "Stub : mrpc::client::MrpcClient {\n";
    output << "public:\n";
    output << "  " << service.name << "Stub(const std::string &addr) : "
           << "mrpc::client::MrpcClient(addr) {}\n\n";

    // 为每个方法生成三种调用方式
    for (size_t i = 0; i < service.methods.size(); ++i) {
      const auto &method = service.methods[i];

      // 同步调用
      output << "  mrpc::Status " << method.name << "(" << method.name
             << "Request &request, " << method.name
             << "Response &response) {\n";
      output << "    return Send(" << service.name << "_method_names[" << i
             << "], request, response);\n  }\n\n";

      // 异步调用
      output << "  mrpc::Status Async" << method.name << "(" << method.name
             << "Request &request, std::string &key) {\n";
      output << "    return AsyncSend(" << service.name << "_method_names[" << i
             << "], request, key);\n  }\n\n";

      // 回调方式
      output << "  void Callback" << method.name << "(" << method.name
             << "Request &request, " << method.name << "Response &response,\n"
             << "                        std::function<void(mrpc::Status)> "
                "callback) {\n";
      output << "    CallbackSend(" << service.name << "_method_names[" << i
             << "], request, response, callback);\n  }\n\n";
    }

    // Receive方法
    output << "  mrpc::Status Receive(const std::string &key, mrpc::Parser "
              "&response) {\n";
    output << "    return mrpc::client::MrpcClient::Receive(key, response);\n";
    output << "  }\n";
    output << "};\n\n";
  }

  // 生成Service类
  void generateService() override {
    output << "class " << service.name
           << "Service : public mrpc::server::MrpcService {\n";
    output << "public:\n";
    output << "  " << service.name << "Service() : mrpc::server::MrpcService(\""
           << namespace_name << "." << service.name << "\") {\n";

    for (size_t i = 0; i < service.methods.size(); ++i) {
      const auto &method = service.methods[i];
      output << "    AddHandler<" << method.name << "Request, " << method.name
             << "Response>(\n";
      output << "        " << service.name << "_method_names[" << i << "],\n";
      output << "        [this](const " << method.name << "Request &request, "
             << method.name << "Response &response) {\n";
      output << "          return this->" << method.name
             << "(request, response);\n        });\n";
    }
    output << "  }\n\n";

    // 纯虚函数声明
    for (const auto &method : service.methods) {
      output << "  virtual mrpc::Status " << method.name << "(const "
             << method.name << "Request &request,\n"
             << "                                " << method.name
             << "Response &response) = 0;\n";
    }
    output << "};\n\n";
  }

  // 生成命名空间结束
  void generateNamespaceEnd() {
    output << "} // namespace " << namespace_name << "\n";
  }

public:
  CppStubGenerator(const std::string &yaml_path)
      : StubGeneratorBase(yaml_path) {
    namespace_name = yaml_filename;
  }

  bool generate(const std::string &output_dir) override {
    generateHeader();
    generateNamespaceStart();
    generateMethodNames();
    generateStructs();
    generateClient();
    generateService();
    generateNamespaceEnd();

    std::string h_file(output_dir + "/" + yaml_filename + ".mrpc.h");
    std::ofstream out_file_1(h_file);
    if (!out_file_1) {
      std::cerr << "Failed to open output file: " << h_file << std::endl;
      return false;
    }

    out_file_1 << output.str();
    out_file_1.close();

    std::string cc_file(output_dir + "/" + yaml_filename + ".mrpc.cc");
    std::ofstream out_file_2(cc_file);
    if (!out_file_2) {
      std::cerr << "Failed to open output file: " << cc_file << std::endl;
      return false;
    }

    out_file_2 << "// Empty implementation file for " << yaml_filename;
    out_file_2.close();

    return true;
  }
};

} // namespace generator
} // namespace mrpc

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <input.yaml>" << std::endl;
    return 1;
  }

  mrpc::generator::CppStubGenerator generator(argv[1]);
  if (!generator.parseYaml(argv[1])) {
    std::cerr << "Failed to parse YAML file" << std::endl;
    return 1;
  }

  if (!generator.generate(argv[2])) {
    std::cerr << "Failed to generate stub file" << std::endl;
    return 1;
  }

  std::cout << "Successfully generated CC stub at: " << argv[2] << std::endl;

  return 0;
}