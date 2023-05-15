#pragma once
#include "../include/structures.h"
#include "../include/array_methods.h"

void updateErr(double err, double x, double y, answer& ans) {
  if (err > ans.err) {
    ans.x = x;
    ans.y = y;
    ans.err = err;
  }
}

answer method_upper_relaxation(double** u, int** mask, global_data& data) {
  answer ans;

  double** _u = create_array<double>(data.m + 1, data.n + 1);
  copy_array<double>(u, _u, data.m + 1, data.n + 1);

  int nmax = data.nmax;
  double (*_f)(double, double) = data.func_f;
  double w = data.param;
  double eps = data.eps;

  double h = (data.b - data.a) / data.n;
  double k = (data.d - data.c) / data.m;

  double H = 1.0 / (h * h);
  double K = 1.0 / (k * k);
  double A = -2.0 * (H + K);

  double R = LDBL_MIN;
  double acc = LDBL_MAX;
  double x, y, tmp;
  int count = 0;

  auto& coord_x = data.coord_x;
  auto& coord_y = data.coord_y;

  while (count < nmax && acc > eps) {
    acc = LDBL_MIN;
    R = LDBL_MIN;
    for (int j = 1; j < data.m; ++j) {
      y = coord_y[j];
      for (int i = 1; i < data.n; ++i) {
        if (mask[j][i] == 2) {
          x = coord_x[i];
          tmp = -(w * (H * (_u[j][i + 1] + _u[j][i - 1]) +
                       K * (_u[j + 1][i] + _u[j - 1][i]) + _f(x, y)) +
                  (w - 1.0) * A * _u[j][i]) /
                A;
          acc = std::max(acc, abs(_u[j][i] - tmp));
          _u[j][i] = tmp;
        }
      }
    }
    ++count;
    /* if ((count * 100) / nmax != ((count - 1) * 100) / nmax) {
      std::cout << setprecision(2) << std::fixed << ((float)count / nmax) << "\n";
      std::cout.flush();
    }*/
  }

  R = LDBL_MIN;
  for (int j = 0; j <= data.m; ++j) {
    y = coord_y[j];
    for (int i = 0; i <= data.n; ++i) {
      if (mask[j][i] == 2) {
        x = coord_x[i];
        R = std::max(R, abs(H * (_u[j][i + 1] + _u[j][i - 1]) +
                            K * (_u[j + 1][i] + _u[j - 1][i]) + A * _u[j][i]) +
                            _f(x, y));
      }
    }
  }

  ans.N = count;
  ans.R = R;
  ans.acc = acc;

  ans.nmax = data.nmax;
  ans.eps = data.eps;
  ans.n = data.n;
  ans.m = data.m;
  ans.param = data.param;

  auto& v = ans.v;
  v.resize(data.m + 1);
  for (int j = 0; j <= data.m; ++j) {
    v[j].resize(data.n + 1);
    for (int i = 0; i <= data.n; ++i) {
      switch (mask[j][i]) {
        case 0:
          v[j][i] = 0.0;
          break;
        case 1:
        case 2:
          v[j][i] = _u[j][i];
          break;
      }
    }
  }

  ans.coord_x = data.coord_x;
  ans.coord_y = data.coord_y;

  delete_array(_u);
  return ans;
}

/* answer method_upper_relaxation(double** u, int** mask, global_data& data) {
  auto& coord_x = data.coord_x;
  auto& coord_y = data.coord_y;
  double x, y;
  answer ans;
  // if (test) {
  /* auto func_u = data.func_u;

  auto& err = ans.err;
  for (int j = 0; j <= data.m; ++j) {
      y = coord_y[j];
      for (int i = 0; i <= data.n; ++i) {
      if (mask[j][i] == 2 || mask[j][i] == 1) {
          x = coord_x[i];
          err = max(err, abs(ans.v[j][i] - func_u(x, y)));
      }
      }
  }*/
  // } else {
  /* answer ans1 =
      _method_upper_relaxation_(u, mask, data.n, data.m, data);
  answer ans2 =
      _method_upper_relaxation_(u, mask, data.n * 2, data.m * 2, data);

  double x, y;
  auto& err = ans1.err;
  for (int j = 0; j <= data.m; ++j) {
      y = coord_y[j];
      for (int i = 0; i <= data.n; ++i) {
      if (mask[j][i] == 2 || mask[j][i] == 1) {
          x = coord_x[i];
          err = max(err, abs(ans1.v[j][i] - ans2.v[j * 2][i * 2]));
      }
      }
  }
  //}
  return ans;
}*/

/* void method_simple_iteration_test(double** u, int** mask,
                                       global_data& data) {
    double a = data.a, b = data.b, c = data.c, d = data.d;
    int nmax = data.nmax;
    double (*_f)(double, double) = data.func_f;
    double (*_u)(double, double) = data.func_u;
    double t = data.param;
    double eps = data.eps;

    double h = (b - a) / data.n;
    double k = (d - c) / data.m;

    double H = 1.0 / (h * h);
    double K = 1.0 / (k * k);
    double A = -2.0 * (H + K);

    t = 0.00001;

    double R = LDBL_MIN;
    double x, y;

    int count = 0;
    double ma = LDBL_MAX;

    nmax = 1;

    double** u2 = create_array<double>(data.m + 1, data.n + 1);

    copy_array(u, u2, data.m + 1, data.n + 1);

    while (count < nmax && ma > eps) {
        ma = LDBL_MIN;
        for (int j = 0; j <= data.m; ++j) {
            y = c + j * k;
            for (int i = 0; i <= data.n; ++i) {
                x = a + i * h;
                if (mask[j][i] == 2) {
                    double rs = K * (u[j - 1][i] + u[j + 1][i]) + H * (u[j][i -
1] + u[j][i + 1]) + A * u[j][i] + _f(x, y); u2[j][i] = u[j][i] - t * rs; ma =
max(ma, abs(u2[j][i] - _u(x, y)));
                }
            }
        }
        swap(u2, u);
        ++count;
    }
    swap(u2, u);

    for (int j = 0; j <= data.m; ++j) {
        y = data.c + j * k;
        for (int i = 0; i <= data.n; ++i) {
            cout << u[j][i] << "\t";
            if (mask[j][i] == 2) {
                x = data.a + i * h;
                //ma = max(ma, abs(u[j][i] - data.func_u(x, y)));
            }
        }
        cout << "\n";
    }
    int _i , _j;
    _i = _j = 0;

    swap(u2, u);
    ma = LDBL_MIN;
    //cout << "eps: " << ma << "\n";
    cout << "\n";
    for (int j = 0; j <= data.m; ++j) {
        y = data.c + j * k;
        for (int i = 0; i <= data.n; ++i) {
            cout << u[j][i] << "\t";
            if (mask[j][i] == 2) {
                x = data.a + i * h;
                double tmp = abs(u[j][i] - data.func_u(x, y));
                if (tmp > ma) {
                    ma = tmp;
                    _i = i;
                    _j = j;
                }
            }
        }
        cout << "\n";
    }

    cout << "eps: " << ma << "\n";
    cout << "i: " << _i << " j: " << _j << "\n";
    //delete_array(u2);
}*/