#pragma once
#include<vector>
#include "../include/nlohmann/json.hpp"
using json = nlohmann::json;

struct global_data {
  int numberVariant;
  int numberMethod;
  bool test;
  // stop condition
  int nmax;
  double eps;
  // area
  int n, m;
  double a, b, c, d;
  int** area;
  int area_x, area_y;
  // method
  double param;

  double (*func_m)(double, double);
  double (*func_f)(double, double);
  double (*func_u)(double, double);

  // auxiliary
  std::vector<double> coord_x;
  std::vector<double> coord_y;
};

class answer {
 public:
  int n = -1, m = -1;
  int N = -1;
  int nmax = -1;
  double eps = LDBL_MAX;
  double acc = LDBL_MIN;
  double err = LDBL_MIN;
  double R = LDBL_MIN;
  //double e1;
  double x = LDBL_MIN, y = LDBL_MIN;
  double param = LDBL_MIN;
  std::vector<std::vector<double>> v;
  std::vector<double> coord_x;
  std::vector<double> coord_y;

   std::string to_json() {
    json j;
    j["n"] = n;
    j["m"] = m;
    j["N"] = N;
    j["nmax"] = nmax;
    j["eps"] = eps;
    j["err"] = err;
    j["acc"] = acc;
    j["R"] = R;
    j["x"] = x;
    j["y"] = y;
    j["param"] = param;
    j["v"] = v;
    j["coord_x"] = coord_x;
    j["coord_y"] = coord_y;
    return j.dump();
  }
};