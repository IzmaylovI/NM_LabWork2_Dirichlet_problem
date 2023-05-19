#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "../include/functions.h"
#include "../include/methods.h"


#define CHECK(x, y) if (!(x)) throw std::exception(y)
#define CHECK_ARRAY(arr, cond, exc) for (int i = 0; i < arr.size(); ++i) { auto item = arr[i];if (!(cond)) throw std::exception(exc);}                                      
int precision = 6;

void parse(std::ifstream& f, global_data& data) {
  json j = json::parse(f);
  data.a = j["a"];
  data.b = j["b"];
  data.c = j["c"];
  data.d = j["d"];
  data.n = j["n"];
  data.m = j["m"];
  data.nmax = j["nmax"].get<std::vector<int>>();
  data.eps = j["eps"].get<std::vector<double>>();
  data.test = j["test"];
  data.numberMethod = j["numberMethod"];
  data.numberVariant = j["numberVariant"];
  data.param = j["param"].get<std::vector<double>>();
  data.area_x = j["area_x"];
  data.area_y = j["area_y"];
  std::string str = j["area"];

  CHECK(data.a < data.b, "a >= b");
  CHECK(data.c < data.d, "c >= d");
  CHECK(data.n >= 0, "n < 0");
  CHECK(data.m >= 0, "m < 0");
  CHECK(data.numberMethod >= 0, "numberMethod < 0");
  CHECK(data.numberVariant >= 0, "numberVariant < 0");
  CHECK_ARRAY(data.nmax, item >= 0, "nmax < 0");
  CHECK_ARRAY(data.eps, item >= 0, "eps < 0");
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

  data.initialize();
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

void fill_border_of_array(double** arr, int** mask, const global_data& data) {
    double a = data.a, b = data.b, c = data.c, d = data.d;
    double h = (data.b - data.a) / data.n;
    double k = (data.d - data.c) / data.m;
    double (*_m)(double, double) = data.func_m;
    double x, y;
    double f1, f2;

    for (int j = 0; j <= data.m; ++j) {
        y = c + j * k;
        f1 = _m(a, y), f2 = _m(b, y);
        for (int i = 0; i <= data.n; ++i) {
            x = a + i * h;
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

void fill_arrays_of_coord(global_data& data){
    data.coord_x.resize(data.n + 1);
    data.coord_y.resize(data.m + 1);

    double a = data.a;
    double c = data.c;
    double h = (data.b - data.a) / data.n; 
    double k = (data.d - data.c) / data.m;

    for (int j = 0; j <= data.m; ++j) data.coord_y[j] = c + j * k;
    for (int i = 0; i <= data.n; ++i) data.coord_x[i] = a + i * h;
}

std::pair<double**, int**> create_area(const global_data& data) {
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

void add_result_method(int pos, double** v, result_method& res, global_data& data) {
    if (data.arr_u.size() <= pos) data.arr_u.resize(pos + 1);
    if (data.acc.size() <= pos) data.acc.resize(pos + 1);
    if (data.R.size() <= pos) data.R.resize(pos + 1);
    if (data.N.size() <= pos) data.N.resize(pos + 1);
    copy_array(v, data.arr_u[pos], data.n + 1, data.m + 1);
    data.acc[pos] = res.acc;
    data.R[pos] = res.R;
    data.N[pos] = res.count;
}

template<class T>
void show_array(std::ostream& out, T** arr, int sizeY, int sizeX) {
    for (int j = 0; j < sizeY; ++j) {
        for (int i = 0; i < sizeX; ++i) {
            out << arr[j][i] << " ";
        }
        out << "\n";
    }
}

/* template <class T>
std::pair<int, int> compress_array(T** arr1, int sizeY, int sizeX, T** arr2, int& new_sizeY, int& new_sizeX, int k) {
    new_sizeY = sizeY / k;
    new_sizeX = sizeX / k;
    arr2 = create_array<T>(new_sizeY, new_sizeX);
    for (int j = 0; j < new_sizeY; ++j){
        for (int i = 0; i < new_sizeX; ++i) {
            arr2[j][i] = arr1[j * k][i * k];
        }    
    }
}*/

int main(int argc, char* argv[]) {
    std::cout.precision(precision);

    clock_t start = clock();

    std::ifstream in;
    std::ofstream out;
    global_data data;

    try {
        CHECK(argc == 3, "input argument as [input path] [output path]");

        in.open(argv[1]);
        if (!in.is_open()) throw std::exception("path not open");
        parse(in, data);
        in.close();
        set_function(data.numberVariant, data.test, data);

        double h, k;
        h = (data.b - data.a) / data.n;
        k = (data.d - data.c) / data.m;

        auto p = create_area(data);
        double** v = p.first;
        int** mask = p.second;
        // show_array(std::cout, v, data.m + 1, data.n + 1);
        double** f = create_array<double>(data.m + 1, data.n + 1);
        fill_array_func(f, mask, data.n, data.m, data.a, data.c, h, k,
                        data.func_f);
        // choose_method(data.numberMethod, data);

        auto& coord_x = data.coord_x;
        auto& coord_y = data.coord_y;
        double x, y;

        if (data.test) {
            result_method res =
                method_upper_relaxation(v, f, mask, data.n, data.m, h, k,
                                        data.nmax[0], data.eps[0], data.param);
            double** u = create_array<double>(data.m + 1, data.n + 1);

            fill_array_func(u, mask, data.n, data.m, data.a, data.c, h, k,
                            data.func_u);
            add_result_method(0, v, res, data);
            copy_array(u, data.arr_u[1], data.m + 1, data.n + 1);
            set_error(data.arr_u[0], data.arr_u[1], data.arr_err, mask, data,
                      1);
        } else {
            result_method res =
                method_upper_relaxation(v, f, mask, data.n, data.m, h, k,
                                        data.nmax[0], data.eps[0], data.param);
            add_result_method(0, v, res, data);

            // task 2
            data.n *= 2;
            data.m *= 2;

            auto p2 = create_area(data);
            double** v2 = p2.first;
            int** mask2 = p2.second;
            double** f2 = create_array<double>(data.m + 1, data.n + 1);

            h = (data.b - data.a) / data.n;
            k = (data.d - data.c) / data.m;

            fill_array_func(f2, mask2, data.n, data.m, data.a, data.c, h, k,
                            data.func_f);
            res =
                method_upper_relaxation(v2, f2, mask2, data.n, data.m, h, k,
                                        data.nmax[1], data.eps[1], data.param);
            add_result_method(1, v2, res, data);
            data.m /= 2;
            data.n /= 2;
            //std::cout << "!";
            //std::cout.flush();
            set_error(data.arr_u[0], data.arr_u[1], data.arr_err, mask, data,
                      2);
        }
        out.open(argv[2]);
        out.clear();
        out << data.get_answer() << "\n";
        out.close();

        delete_area(p);
        delete_array(f);
    }

    catch (std::exception e) {
        std::cerr << e.what();
        return -1;
    }

    clock_t end = clock();
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    std::cout << "The time: " << seconds << " seconds\n";
}
     