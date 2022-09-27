#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>

using namespace rapidjson;

int main() {
    // 1. JSON 문자열을 파싱해서 DOM 으로 구조화 한다.
    const char* json = "{\"project\":\"rapidjson\",\"stars\":10}";
    Document d;
    d.Parse(json);

    // 2. DOM 내용을 변경한다.
    Value& s = d["stars"];
    s.SetInt(s.GetInt() + 1);
    
    Value a(kArrayType);
    a.PushBack("Luffy", d.GetAllocator());
    a.PushBack("Nami", d.GetAllocator());

    d.AddMember("students", a, d.GetAllocator());

    // 3. DOM 내용을 다시 문자열로 변경한다.
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);

    // 문자열을 다시 출력한다. {"project":"rapidjson","stars":11}
    std::cout << buffer.GetString() << std::endl;
    return 0;
}