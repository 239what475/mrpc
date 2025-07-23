#include "gen_stub.h"
#include <iostream>

namespace mrpc {
namespace generator {

class GoStubGenerator : public StubGeneratorBase {
private:
    // 生成Go类型定义
    std::string generateGoType(const std::string& type) {
        if (type == "string") return "string";
        if (type == "int") return "int";
        if (type == "bool") return "bool";
        if (type == "float") return "float64";
        return "string"; // 默认类型
    }

    // 生成方法名数组
    void generateMethodNames() override {
        output << "var " << service.name << "_method_names = []string{\n";
        for (const auto& method : service.methods) {
            output << "\t\"/" << yaml_filename << "." << service.name << "/" 
                  << method.name << "\",\n";
        }
        output << "}\n\n";
    }

    // 生成请求和响应结构体
    void generateStructs() override {
        for (const auto& method : service.methods) {
            // 生成请求结构体
            output << "type " << method.name << "Request struct {\n";
            for (const auto& param : method.request_params) {
                output << "\t" << capitalize(param.name) << " " << 
                         generateGoType(param.type) << " `json:\"" << 
                         param.name << "\"`\n";
            }
            output << "}\n\n";
            
            // 生成请求ToString方法
            output << "func (r *" << method.name << "Request) ToString() (string, error) {\n";
            output << "\tdata, err := json.Marshal(r)\n";
            output << "\tif err != nil {\n";
            output << "\t\treturn \"\", err\n";
            output << "\t}\n";
            output << "\treturn string(data), nil\n";
            output << "}\n\n";
            
            // 生成响应结构体
            output << "type " << method.name << "Response struct {\n";
            for (const auto& param : method.response_params) {
                output << "\t" << capitalize(param.name) << " " << 
                         generateGoType(param.type) << " `json:\"" << 
                         param.name << "\"`\n";
            }
            output << "}\n\n";
            
            // 生成响应FromString方法
            output << "func (r *" << method.name << "Response) FromString(data string) error {\n";
            output << "\treturn json.Unmarshal([]byte(data), r)\n";
            output << "}\n\n";
        }
    }

    // 生成客户端结构体和方法
    void generateClient() override {
        // 生成客户端结构体
        output << "type " << service.name << "Client struct {\n";
        output << "\tclient *mrpc.Client\n";
        output << "}\n\n";
        
        // 生成构造函数
        output << "func New" << service.name << "Client(s string) *" << service.name << "Client {\n";
        output << "\treturn &" << service.name << "Client{\n";
        output << "\t\tclient: mrpc.NewClient(s),\n";
        output << "\t}\n";
        output << "}\n\n";
        
        // 为每个方法生成同步、异步和回调方法
        for (size_t i = 0; i < service.methods.size(); i++) {
            const auto& method = service.methods[i];
            
            // 同步方法
            output << "func (h *" << service.name << "Client) " << method.name << 
                     "(request *" << method.name << "Request) (";
            
            auto first_response = method.response_params[0];
            output << generateGoType(first_response.type) << ", error) {\n";
            output << "\tresponse := &" << method.name << "Response{}\n";
            output << "\terr := h.client.Send(" << service.name << "_method_names[" << 
                     std::to_string(i) << "], request, response)\n";
            output << "\treturn response." << capitalize(first_response.name) << ", err\n";
            output << "}\n\n";
            
            // 异步方法
            output << "func (h *" << service.name << "Client) Async" << method.name << 
                     "(request *" << method.name << "Request) (string, error) {\n";
            output << "\treturn h.client.AsyncSend(" << service.name << "_method_names[" << 
                     std::to_string(i) << "], request)\n";
            output << "}\n\n";
            
            // 回调方法
            output << "func (h *" << service.name << "Client) Callback" << method.name << 
                     "(request *" << method.name << "Request, callback func(" << 
                     generateGoType(first_response.type) << ", error)) {\n";
            output << "\tresponse := &" << method.name << "Response{}\n";
            output << "\th.client.CallbackSend(" << service.name << "_method_names[" << 
                     std::to_string(i) << "], request, response, func(err error) {\n";
            output << "\t\tcallback(response." << capitalize(first_response.name) << ", err)\n";
            output << "\t})\n";
            output << "}\n\n";
        }
        
        // 生成Receive方法
        if (service.methods.size() > 1) {
            output << "func (h *" << service.name << "Client) Receive(key string, methodIndex int) (string, error) {\n";
            output << "\tswitch methodIndex {\n";
            for (size_t i = 0; i < service.methods.size(); i++) {
                const auto& method = service.methods[i];
                output << "\tcase " << std::to_string(i) << ":\n";
                output << "\t\tresponse := &" << method.name << "Response{}\n";
                output << "\t\terr := h.client.Receive(key, response)\n";
                auto first_response = method.response_params[0];
                output << "\t\treturn response." << capitalize(first_response.name) << ", err\n";
            }
            output << "\tdefault:\n";
            output << "\t\treturn \"\", fmt.Errorf(\"unknown method index: %d\", methodIndex)\n";
            output << "\t}\n";
            output << "}\n\n";
        } else {
            const auto& method = service.methods[0];
            output << "func (h *" << service.name << "Client) Receive(key string) (string, error) {\n";
            output << "\tresponse := &" << method.name << "Response{}\n";
            output << "\terr := h.client.Receive(key, response)\n";
            auto first_response = method.response_params[0];
            output << "\treturn response." << capitalize(first_response.name) << ", err\n";
            output << "}\n\n";
        }
        
        // 生成Close方法
        output << "func (h *" << service.name << "Client) Close() {\n";
        output << "\th.client.Close()\n";
        output << "}\n";
    }

    // 生成服务端抽象基类
    void generateService() override {
        // 生成服务结构体
        output << "type " << service.name << "Service struct {\n";
        output << "\t*mrpc.Service\n";
        output << "}\n\n";
        
        // 生成服务构造函数
        output << "func New" << service.name << "Service() *" << service.name << "Service {\n";
        output << "\tservice := &" << service.name << "Service{\n";
        output << "\t\tService: mrpc.NewService(\"" << yaml_filename << "." << service.name << "\"),\n";
        output << "\t}\n\n";
        
        // 注册所有方法的处理函数
        for (size_t i = 0; i < service.methods.size(); i++) {
            const auto& method = service.methods[i];
            output << "\tservice.RegisterHandler(" << service.name << "_method_names[" << 
                     std::to_string(i) << "], \n\t\tfunc(request *" << method.name << 
                     "Request, response *" << method.name << "Response) error {\n";
            output << "\t\t\treturn service." << method.name << "(request, response)\n";
            output << "\t\t})\n\n";
        }
        
        output << "\treturn service\n";
        output << "}\n\n";
        
        // 为每个方法生成抽象方法
        for (const auto& method : service.methods) {
            output << "func (s *" << service.name << "Service) " << method.name << 
                     "(request *" << method.name << "Request, response *" << method.name << 
                     "Response) error {\n";
            output << "\treturn fmt.Errorf(\"" << method.name << " not implemented\")\n";
            output << "}\n\n";
        }
    }

public:
    GoStubGenerator(const std::string& yaml_path) 
        : StubGeneratorBase(yaml_path) {}

    bool generate() override {
        // 生成包声明和导入
        output << "package " << yaml_filename << "\n\n";
        output << "import (\n";
        output << "\t\"encoding/json\"\n";
        output << "\t\"mrpc\"\n";
        output << ")\n\n";
        
        generateMethodNames();
        generateStructs();
        generateClient();
        output << "\n";
        generateService();
        
        return true;
    }
};

} // namespace generator
} // namespace mrpc

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.yaml> <output.go>" << std::endl;
        return 1;
    }
    
    mrpc::generator::GoStubGenerator generator(argv[1]);
    if (!generator.parseYaml(argv[1])) {
        std::cerr << "Failed to parse YAML file" << std::endl;
        return 1;
    }

    if (!generator.generate()) {
        std::cerr << "Failed to generate stub file" << std::endl;
        return 1;
    }

    std::cout << generator.GetGenFile() << std::endl;
    return 0;
} 