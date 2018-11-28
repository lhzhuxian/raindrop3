#include <fstream>
#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <limits.h>
#include <algorithm>
#include <time.h>
#include <sys/time.h>
#include <iomanip>

using namespace std;

struct arg_struct {
  vector<int> & current;
  arg_struct(vector<int> & a):current(a){}
};

int main() {
  vector<int> a;
  a.push_back(1);
  arg_struct tmp(a);
  cout << tmp.current[0] << endl;
  tmp.current[0] = 2;
  cout << a[0] << endl;
}
