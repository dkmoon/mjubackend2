#define RAPIDJSON_HAS_STDSTRING 1

#include <iostream>
#include <string>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

class Person {
private:
    int id;
    std::string name;

public:
    Person(int i, const std::string& name);
    Person(const std::string& jsonString);
    std::string toJson() const;
};


Person::Person(int id, const std::string &name)
    : id(id), name(name) {
    // 빈 구현
}


Person::Person(const std::string& jsonString) {
    // TODO: 여기를 구현하라
}


std::string Person::toJson() const {
    Document d;
    d.SetObject();

    // TODO: 여기를 채운다.
    return "";
}

int main() {
    // 1. JSON 문자열을 파싱해서 DOM 으로 구조화 한다.
    const std::string json = "{\"project\":\"rapidjson\",\"stars\":10}";
    Document d;
    d.Parse(json);

    // 2. DOM 내용을 변경한다.
    Value& s = d["stars"];
    s.SetInt(s.GetInt() + 1);
    
    // 3. DOM 내용을 다시 문자열로 변경한다.
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);

    // 문자열을 다시 출력한다. {"project":"rapidjson","stars":11}
    std::cout << buffer.GetString() << std::endl;

    Person p = Person(1000, "Luffy");
    std::cout << p.toJson() << std::endl;

    const std::string namiJsonStr = "{\"id\":1000,\"name\":\"Nami\"}";
    Person nami = Person(namiJsonStr);
    std::cout << nami.toJson() << std::endl;

    return 0;
}