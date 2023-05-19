#pragma once
template<class T>
T** create_array(size_t sizeY, size_t sizeX) {
  T* arr = new T[sizeX * sizeY];
  T** arr2 = new T*[sizeY];
  for (int i = 0; i < sizeY; ++i) {
    arr2[i] = &arr[i * sizeX];
  }
  return arr2;
}

template <class T>
void delete_array(T**& arr) {
  delete (*arr);
  delete arr;
}

template <class T>
void copy_array(T** out, T** in, int sizeY, int sizeX) {
  for (int j = 0; j < sizeY; ++j)
    for (int i = 0; i < sizeX; ++i) in[j][i] = out[j][i];
}

template <class T>
void copy_array(T** out, std::vector<std::vector<T>>& in, int sizeY, int sizeX) {
  in.resize(sizeY);
  for (int j = 0; j < sizeY; ++j) {
    in[j].resize(sizeX);
    for (int i = 0; i < sizeX; ++i) in[j][i] = out[j][i];
  }
}