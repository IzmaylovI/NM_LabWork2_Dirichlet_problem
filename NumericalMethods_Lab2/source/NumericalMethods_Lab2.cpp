#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "../include/functions.h"
#include "../include/methods.h"


#define CHECK(x, y) if (!(x)) throw std::exception(y)

int precision = 6;

void parse(std::ifstream& f, global_data& data) {
  json j = json::parse(f);
  data.a = j["a"];
  data.b = j["b"];
  data.c = j["c"];
  data.d = j["d"];
  data.n = j["n"];
  data.m = j["m"];
  data.nmax = j["nmax"];
  data.eps = j["eps"];
  data.test = j["test"];
  data.numberMethod = j["numberMethod"];
  data.numberVariant = j["numberVariant"];
  data.param = j["param"];
  data.area_x = j["area_x"];
  data.area_y = j["area_y"];
  std::string str = j["area"];

  CHECK(data.a < data.b, "a >= b");
  CHECK(data.c < data.d, "c >= d");
  CHECK(data.n >= 0, "n < 0");
  CHECK(data.m >= 0, "m < 0");
  CHECK(data.numberMethod >= 0, "numberMethod < 0");
  CHECK(data.numberVariant >= 0, "numberVariant < 0");
  CHECK(data.nmax >= 0, "nmax < 0");
  CHECK(data.eps >= 0, "eps < 0");
  CHECK(data.area_x > 0, "area_x <= 0");
  CHECK(data.area_y > 0, "area_y <= 0");
  CHECK(data.n % data.area_x == 0, "n % area_x != 0");
  CHECK(data.m % data.area_y == 0, "m % area_y != 0");


  data.area = create_array<int>(data.area_y, data.area_x);
  for (int j = 0; j < data.area_y; ++j) {
    for (int i = 0; i < data.area_x; ++i) {
      data.area[j][i] = str[j * data.area_x + i];
    }
  }
}

int** create_mask_area(int n, int m, int area_x, int area_y, int** area) {
    int** arr2 = create_array<int>(m + 1, n + 1);

    int dx = n / area_x;
    int dy = m / area_y;

    for (int j = 0; j < m + 1; ++j)
        for (int i = 0; i < n + 1; ++i) arr2[j][i] = 0;
    
    for (int sh_x = 0; sh_x <= area_x; sh_x++) {
        int st_x = sh_x * dx;
        for (int sh_y = 0; sh_y < area_y; sh_y++) {
            int st_y = sh_y * dy;
            bool z,l;
            z = !((sh_x - 1 < 0) || area[sh_y][sh_x - 1] == '0');
            l = !((sh_x >= area_x) || area[sh_y][sh_x] == '0');
            bool flag = z xor l;
            if (flag) {
                for (int b = st_y; b <= st_y + dy; ++b) {
                    arr2[b][st_x] = 1;
                }
            }
        }
    }
    for (int sh_y = 0; sh_y <= area_y; sh_y++) {
        int st_y = sh_y * dy;
        for (int sh_x = 0; sh_x < area_x; sh_x++) {
            int st_x = sh_x * dx;
            bool z, l;
            z = !((sh_y - 1 < 0) || area[sh_y - 1][sh_x] == '0');
            l = !((sh_y >= area_y) || area[sh_y][sh_x] == '0');
            bool flag = z xor l;
            if (flag) {
                for (int b = st_x; b <= st_x + dx; ++b) {
                    arr2[st_y][b] = 1;
                }
            }
        }
    }
    int tmp;
    for (int y = 0; y <= m; ++y) {
        int flag = 0;
        for (int x = 0; x <= n; ++x) {
            tmp = arr2[y][x];
            switch (tmp) {
            case 1: {
                while (x <= n && arr2[y][x] == 1)++x;
                if (x <= n) {
                    int _x = x / dx;
                    int _y = y / dy;
                    if (_x >= area_x || _y >= area_y)flag = 0;
                    else {
                        flag = area[_y][_x] == '0' ? 0 : 2;
                    }
                    arr2[y][x] = flag;
                }
                break;
            }
            case 0: {
                arr2[y][x] = flag;
                break;
            }
            }
        }
    }
    return arr2;
}

void fill_border_of_array(double** arr, int** mask, global_data& data) {
    double a = data.a, b = data.b, c = data.c, d = data.d;
    double h = (data.b - data.a) / data.n;
    double k = (data.d - data.c) / data.m;
    double (*_m)(double, double) = data.func_m;
    double x, y;
    double f1, f2;

    data.coord_x.resize(data.n + 1);
    data.coord_y.resize(data.m + 1);

    for (int j = 0; j <= data.m; ++j) data.coord_y[j] = c + j * k;
    for (int i = 0; i <= data.n; ++i) data.coord_x[i] = a + i * h;

    for (int j = 0; j <= data.m; ++j) {
        y = data.coord_y[j];
        f1 = _m(a, y), f2 = _m(b, y);
        for (int i = 0; i <= data.n; ++i) {
            x = data.coord_x[i];
            switch (mask[j][i]) {
            case 0:
                arr[j][i] = 0;
                break;
            case 1:
                arr[j][i] = data.func_m(x, y);
                break;
            case 2:
                //линейная интерпляция значений по оси X
                arr[j][i] = (f2 * (x - a) - f1 * (x - b)) / (b - a);
                break;
            }
        }
    }
}

std::pair<double**, int**> create_area(global_data& data) {
    std::pair<double**, int**> ans;
    double** arr = create_array<double>(data.m + 1, data.n + 1);
    int** mask =
        create_mask_area(data.n, data.m, data.area_x, data.area_y, data.area);
    fill_border_of_array(arr, mask, data);

    ans.first = arr;
    ans.second = mask;
    return ans;
}

void delete_area(std::pair<double**, int**> area) {
    delete_array(area.first);
    delete_array(area.second);
}

int main(int argc, char* argv[])
{
    std::cout.precision(precision);
    clock_t start = clock();

    std::ifstream in;
    std::ofstream out;
    global_data data;
    try {
        CHECK(argc == 3, "input argument as [input path] [output path]");

        in.open(argv[1]);
        parse(in, data);
        in.close();
        set_function(data.numberVariant, data.test, data);

        auto p = create_area(data);
        answer ans;

        //choose_method(data.numberMethod, data);

        auto& coord_x = data.coord_x;
        auto& coord_y = data.coord_y;
        double x, y;


        if (data.test) {
            ans = method_upper_relaxation(p.first, p.second, data);
            double& err = ans.err;

            auto func_u = data.func_u;
            double** arr = p.first;
            int** mask = p.second;

            for (int j = 0; j <= data.m; ++j) {
            y = coord_y[j];
            for (int i = 0; i <= data.n; ++i) {
                if (mask[j][i] == 2 || mask[j][i] == 1) {
                    x = coord_x[i];
                    updateErr(abs(ans.v[j][i] - func_u(x, y)), x, y, ans);
                }
            }
            }
        } else {
            double** arr1 = p.first;
            int** mask1 = p.second;
            ans = method_upper_relaxation(p.first, p.second, data);
            double& err = ans.err;

            global_data data2 = data;
            data2.n *= 2;
            data2.m *= 2;
            auto p2 = create_area(data2);
            answer ans2 = method_upper_relaxation(p2.first, p2.second, data2);

            for (int j = 0; j <= data.m; ++j) {
            y = coord_y[j];
            for (int i = 0; i <= data.n; ++i) {
                if (mask1[j][i] == 2 || mask1[j][i] == 1) {
                    x = coord_x[i];
                    updateErr(abs(ans.v[j][i] - ans2.v[j * 2][i * 2]), x, y,
                              ans);
                }
            }
            }
            delete_area(p2);
        }

        delete_area(p);

        out.open(argv[2]);
        out.clear();
        out << ans.to_json() << "\n";
        out.close();
    } catch (std::exception e) {
        std::cerr << e.what();
        return -1;
    }
    //clock_t end = clock();
    //double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    //cout << "The time: " << seconds << " seconds\n";
}