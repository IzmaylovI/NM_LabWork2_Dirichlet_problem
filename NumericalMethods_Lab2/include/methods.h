#pragma once
#include "../include/structures.h"
#include "../include/array_methods.h"

/* void updateErr(double err, double x, double y, answer& ans) {
  if (err > ans.err) {
    ans.x = x;
    ans.y = y;
    ans.err = err;
  }
}*/

double get_R(double** u, double** f, int** mask, int n, int m, double H,
             double K, double A) {
  double R = LDBL_MIN;
  for (int j = 0; j <= m; ++j) {
    for (int i = 0; i <= n; ++i) {
      if (mask[j][i] == 2) {
        R = std::max(R, abs(H * (u[j][i + 1] + u[j][i - 1]) +
                            K * (u[j + 1][i] + u[j - 1][i]) + A * u[j][i]) +
                            f[j][i]);  //?
      }
    }
  }
  return R;
}

void fill_array_func(double** arr, int** mask, int n, int m, double a, double c,
                    double h, double k, double (*func)(double, double)) {
  double x, y;
  for (int j = 0; j <= m; ++j) {
    y = c + j * k;
    for (int i = 0; i <= n; ++i) {
      if (mask[j][i] == 2 || mask[j][i] == 1) {
        x = a + h * i;
        arr[j][i] = func(x, y);
      }
    }
  }
}

void set_error(std::vector<std::vector<double>>& arr1,
               std::vector<std::vector<double>>& arr2,
               std::vector<std::vector<double>>& arr_out,
               int** mask, global_data& data, int step) {
  double& err = data.err;
  err = LDBL_MIN;
  int _i, _j;
  _i = _j = -1;
  double tmp;
  arr_out.resize(data.m + 1);
  for (int j = 0; j <= data.m; ++j) {
    arr_out[j].resize(data.n + 1);
    for (int i = 0; i <= data.n; ++i) {
      if (mask[j][i] == 2 || mask[j][i] == 1) {
        tmp = abs(arr2[j * step][i * step] - arr1[j][i]);
        arr_out[j][i] = tmp;
        if (err < tmp) {
          err = tmp;
          _i = i;
          _j = j;
        }
      }
    }
  }
  data.x_err = data.a + _i * (data.b - data.a) / data.n;
  data.y_err = data.c + _j * (data.d - data.c) / data.m;
}

result_method method_upper_relaxation(double** v, double** f, int** mask,
                                      int n, int m,
                             double h, double k, int nmax, double eps,
                             const std::vector<double>& param)
{
  //double** _u = create_array<double>(data.m + 1, data.n + 1);
  //copy_array<double>(u, _u, data.m + 1, data.n + 1);

  double w = param[0];

  double H = 1.0 / (h * h);
  double K = 1.0 / (k * k);
  double A = -2.0 * (H + K);

  double R = LDBL_MIN;
  double acc = LDBL_MAX;
  double x, y, tmp;
  int count = 0;

  while (count < nmax && acc > eps) {
    acc = LDBL_MIN;
    R = LDBL_MIN;
    for (int j = 1; j < m; ++j) {
      for (int i = 1; i < n; ++i) {
        if (mask[j][i] == 2) {
          tmp = -(w * (H * (v[j][i + 1] + v[j][i - 1]) +
                       K * (v[j + 1][i] + v[j - 1][i]) + f[j][i]) +
                  (w - 1.0) * A * v[j][i]) /
                A;
          acc = std::max(acc, abs(v[j][i] - tmp));
          v[j][i] = tmp;
        }
      }
    }
    ++count;
  }
  result_method res;
  res.count = count;
  res.R = get_R(v, f, mask, n, m, H, K, A);
  res.acc = acc;
  return res;
}