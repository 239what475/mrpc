#include "gen_stub.h"
#include <algorithm>
#include <fstream>
#include <iostream>

namespace mrpc {
namespace generator {

class PythonStubGenerator : public StubGeneratorBase {
private:
  std::vector<std::string> messages;

  // 获取参数的Python类型和默认值
  std::pair<std::string, std::string>
  getPythonTypeAndDefault(const std::string &type) {
    if (type == "string")
      return {"str", "\"\""};
    if (type == "int")
      return {"int", "0"};
    if (type == "float")
      return {"float", "0.0"};
    if (type == "bool")
      return {"bool", "False"};
    return {"str", "\"\""}; // 默认情况
  }

  // 生成固定的导入语句
  void generateImports() {
    output << "import mrpc\n";
    output << "import json\n";
    output << "from typing import Callable, Optional\n\n"; // 添加了 Optional
    output << "Callback = Callable[[str, Exception | None], None]\n\n\n";
  }

  // 生成方法名数组
  void generateMethodNames() override {
    output << service.name << "_METHOD_NAMES = [\n";
    for (const auto &method : service.methods) {
      output << "    \"/" << yaml_filename << "." << service.name << "/"
             << method.name << "\",\n";
    }
    output << "]\n\n\n";
  }

  // 生成请求和响应结构体
  void generateStructs() override {
    for (const auto &method : service.methods) {
      messages.push_back(method.name + "Request");
      messages.push_back(method.name + "Response");

      // 生成请求类
      output << "class " << method.name << "Request(mrpc.Parser):\n";

      // 构造函数
      output << "    def __init__(self";
      for (const auto &param : method.request_params) {
        auto [type_str, _] = getPythonTypeAndDefault(param.type);
        output << ", " << param.name << ": Optional[" << type_str << "] = None";
      }
      output << "):\n";

      // 初始化参数
      for (const auto &param : method.request_params) {
        output << "        self." << param.name << " = " << param.name << "\n";
      }
      output << "\n";

      // toString方法
      output << "    def toString(self) -> str:\n";
      output << "        return json.dumps({";
      for (size_t i = 0; i < method.request_params.size(); ++i) {
        if (i > 0)
          output << ", ";
        const auto &param = method.request_params[i];
        output << "\"" << param.name << "\": self." << param.name;
      }
      output << "})\n\n";

      // fromString方法
      output << "    def fromString(self, data: str):\n";
      output << "        obj = json.loads(data)\n";
      for (const auto &param : method.request_params) {
        auto [_, default_value] = getPythonTypeAndDefault(param.type);
        output << "        self." << param.name << " = obj.get(\"" << param.name
               << "\", " << default_value << ")\n";
      }
      output << "\n\n";

      // 生成响应类
      output << "class " << method.name << "Response(mrpc.Parser):\n";

      // 构造函数
      output << "    def __init__(self):\n";
      for (const auto &param : method.response_params) {
        auto [_, default_value] = getPythonTypeAndDefault(param.type);
        output << "        self." << param.name << " = " << default_value
               << "\n";
      }
      output << "\n";

      // toString方法
      output << "    def toString(self) -> str:\n";
      output << "        return json.dumps({";
      for (size_t i = 0; i < method.response_params.size(); ++i) {
        if (i > 0)
          output << ", ";
        const auto &param = method.response_params[i];
        output << "\"" << param.name << "\": self." << param.name;
      }
      output << "})\n\n";

      // fromString方法
      output << "    def fromString(self, data: str):\n";
      output << "        obj = json.loads(data)\n";
      for (const auto &param : method.response_params) {
        auto [_, default_value] = getPythonTypeAndDefault(param.type);
        output << "        self." << param.name << " = obj.get(\"" << param.name
               << "\", " << default_value << ")\n";
      }
      output << "\n\n";
    }
  }

  // 生成客户端类
  void generateClient() override {
    output << "class " << service.name << "Client(mrpc.Client):\n";
    output << "    def __init__(self, server_address: str):\n";
    output << "        super().__init__(server_address)\n\n";

    // 为每个方法生成四个相关函数
    for (size_t i = 0; i < service.methods.size(); ++i) {
      const auto &method = service.methods[i];

      // 生成主方法
      output << "    def " << method.name << "(self, request: " << method.name
             << "Request) -> tuple[";

      if (method.response_params.size() == 1) {
        auto [type_str, _] =
            getPythonTypeAndDefault(method.response_params[0].type);
        output << type_str;
      } else {
        output << "tuple[";
        for (size_t j = 0; j < method.response_params.size(); ++j) {
          if (j > 0)
            output << ", ";
          auto [type_str, _] =
              getPythonTypeAndDefault(method.response_params[j].type);
          output << type_str;
        }
        output << "]";
      }
      output << ", Exception | None]:\n";
      output << "        response = " << method.name << "Response()\n";
      output << "        err = super().Send(" << service.name
             << "_METHOD_NAMES[" << i << "], request, response)\n";

      if (method.response_params.size() == 1) {
        output << "        return response." << method.response_params[0].name
               << ", err\n\n";
      } else {
        output << "        return (";
        for (size_t j = 0; j < method.response_params.size(); ++j) {
          if (j > 0)
            output << ", ";
          output << "response." << method.response_params[j].name;
        }
        output << "), err\n\n";
      }

      // 生成异步方法
      output << "    def Async" << method.name
             << "(self, request: " << method.name
             << "Request) -> tuple[str, Exception | None]:\n";
      output << "        return super().AsyncSend(" << service.name
             << "_METHOD_NAMES[" << i << "], request)\n\n";

      // 生成回调方法
      output << "    def Callback" << method.name
             << "(self, request: " << method.name << "Request, callback: ";

      // 生成回调函数类型
      output << "Callable[[";
      for (const auto &param : method.response_params) {
        auto [type_str, _] = getPythonTypeAndDefault(param.type);
        output << type_str << ", ";
      }
      output << "Exception | None], None]):\n";

      output << "        response = " << method.name << "Response()\n";
      output << "        super().CallbackSend(\n";
      output << "            " << service.name << "_METHOD_NAMES[" << i
             << "],\n";
      output << "            request,\n";
      output << "            response,\n";
      output << "            lambda err: callback(";
      for (size_t j = 0; j < method.response_params.size(); ++j) {
        if (j > 0)
          output << ", ";
        output << "response." << method.response_params[j].name;
      }
      output << ", err),\n";
      output << "        )\n\n";

      // 生成接收方法
      output << "    def Receive" << method.name
             << "(self, key: str) -> tuple[";
      if (method.response_params.size() == 1) {
        auto [type_str, _] =
            getPythonTypeAndDefault(method.response_params[0].type);
        output << type_str;
      } else {
        output << "tuple[";
        for (size_t j = 0; j < method.response_params.size(); ++j) {
          if (j > 0)
            output << ", ";
          auto [type_str, _] =
              getPythonTypeAndDefault(method.response_params[j].type);
          output << type_str;
        }
        output << "]";
      }
      output << ", Exception | None]:\n";
      output << "        response = " << method.name << "Response()\n";
      output << "        err = super().Receive(key, response)\n";

      if (method.response_params.size() == 1) {
        output << "        return response." << method.response_params[0].name
               << ", err\n";
      } else {
        output << "        return (";
        for (size_t j = 0; j < method.response_params.size(); ++j) {
          if (j > 0)
            output << ", ";
          output << "response." << method.response_params[j].name;
        }
        output << "), err\n";
      }
    }
  }

  // 生成服务端抽象基类
  void generateService() override {
    // 生成服务类定义
    output << "class " << service.name << "Service(mrpc.MrpcService):\n";

    // 生成构造函数
    output << "    def __init__(self):\n";
    output << "        super().__init__(\"" << yaml_filename << "."
           << service.name << "\")\n";

    // 注册所有方法的处理函数
    for (size_t i = 0; i < service.methods.size(); ++i) {
      const auto &method = service.methods[i]; // 获取当前方法的引用
      output << "        self.AddHandler(\n";
      output << "            " << service.name << "_METHOD_NAMES[" << i << "], "
             << method.name << "Request, " << method.name << "Response,\n";
      output << "            lambda request, response: self." << method.name
             << "(request, response)\n";
      output << "        )\n";
    }
    output << "\n";

    // 为每个方法生成抽象方法
    for (const auto &method : service.methods) {
      output << "    def " << method.name << "(self, request: '" << method.name
             << "Request', response: '" << method.name
             << "Response') -> mrpc.MrpcError | None:\n";
      output << "        pass\n";
    }

    // 生成服务器类
    output << "\n\nclass " << service.name << "Server(mrpc.Server):\n";
    output << "    def __init__(self, server_address: str):\n";
    output << "        super().__init__(server_address)";
  }

  // 辅助函数：将字符串转换为小写
  std::string toLower(const std::string &str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
  }

public:
  PythonStubGenerator(const std::string &yaml_path)
      : StubGeneratorBase(yaml_path) {}

  bool generate(const std::string &output_dir) override {
    generateImports();
    generateMethodNames();
    generateStructs();
    generateClient();
    output << "\n\n";
    generateService();

    std::string py_file(output_dir + "/" + yaml_filename + "_mrpc.py");
    std::ofstream out_file_1(py_file);
    if (!out_file_1) {
      std::cerr << "Failed to open output file: " << py_file << std::endl;
      return false;
    }

    out_file_1 << output.str();
    out_file_1.close();

    std::string init_file(output_dir + "/__init__.py");
    std::ofstream out_file_2(init_file);
    if (!out_file_2) {
      std::cerr << "Failed to open output file: " << init_file << std::endl;
      return false;
    }

    std::ostringstream import_str;
    import_str << "from ." << yaml_filename << "_mrpc import ";
    import_str << service.name << "Client, ";
    import_str << service.name << "Server, ";
    for (auto message : messages) {
      import_str << message << ", ";
    }
    import_str << service.name << "Service ";
    out_file_2 << import_str.str();
    out_file_2.close();

    return true;
  }
};

} // namespace generator
} // namespace mrpc

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <input.yaml> output_dir"
              << std::endl;
    return 1;
  }

  mrpc::generator::PythonStubGenerator generator(argv[1]);
  if (!generator.parseYaml(argv[1])) {
    std::cerr << "Failed to parse YAML file" << std::endl;
    return 1;
  }

  if (!generator.generate(argv[2])) {
    std::cerr << "Failed to generate stub files" << std::endl;
    return 1;
  }

  std::cout << "Successfully generated Python stub at: " << argv[2]
            << std::endl;
  return 0;
}