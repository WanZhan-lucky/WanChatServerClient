#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <map>
#include <vector>
using namespace std;

string func1() {
  json js;
  js["msg_type"] = 2;
  js["from"] = "wan";
  js["to"] = "liufangmiao";
  js["msg"] = "hello_world";
  //   cout << js << endl;
  // 无序列化

  string sendBuf = js.dump();
  //   cout << sendBuf.c_str() << endl;
  return sendBuf.c_str();
}
string func2() {
  json js;
  js["id"] = {1, 2, 3, 4};
  js["msfd"] = {"fjdsla", "djfsklaf"};
  js["name"]["wan"] = {"zhio"};
  js["name"]["zhi"] = {"jflsdf"};
  js["name"] = {{"wan", "zhi"}, {"zhi", "jfsld"}};
  //   cout << js.dump() << endl;
  return js.dump();
}
string func3() {
  json js;
  vector<int> vec;
  vec.push_back(2);
  vec.push_back(3);
  vec.push_back(4);
  js["vec"] = vec;

  map<int, string> mp;
  mp.insert({1, "dkfjlsadf"});
  mp.insert({2, "klsfjd"});
  mp.insert({3, "fjas;ldjf"});
  js["mp"] = mp;

  string sendbuf = js.dump();
  //   cout << sendbuf.c_str() << endl;
  return sendbuf;
}
json fun4(string sendbuf) {
  json recvbuf = json::parse(sendbuf);
  return recvbuf;
}

int main() {
  string s1 = func1();
  string s2 = func2();
  string s3 = func3();
  json res1 = fun4(s1);
  json res2 = fun4(s2);
  json res3 = fun4(s3);
  cout << res1["from"] << " " << res1["to"] << endl;
  cout << res2["id"][2] << " " << res2["name"]["wan"] << endl;
  cout << res3["mp"][1][0] << " " << res3["vec"][2] << endl;

  vector<int> vec = res3["vec"];

  for (auto& k : vec) {
    cout << k << endl;
  }
  map<int, string> mp = res3["mp"];
  for (auto& t : mp) {
    cout << t.first << " " << t.second << endl;
  }
  return 0;
}