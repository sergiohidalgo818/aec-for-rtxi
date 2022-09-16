#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <valarray>

template<typename T>
std::vector<T>
conv(std::vector<T> const &f, std::vector<T> const &g) {
  int const nf = f.size();
  int const ng = g.size();
  int const n  = nf + ng - 1;
  std::vector<T> out(n, T());
  for(auto i(0); i < n; ++i) {
    int const jmn = (i >= ng - 1)? i - (ng - 1) : 0;
    int const jmx = (i <  nf - 1)? i            : nf - 1;
    for(auto j(jmn); j <= jmx; ++j) {
      out[i] += (f[j] * g[i - j]);
    }
  }
  return out; 
}


int main() {

	std::vector<double> f{1, 2, 3, 4};
	std::vector<double> kernel{};

  /***************/
	/* READ KERNEL */
	/***************/
	std::ifstream kernel_file;
	kernel_file.open("aec_kernel.txt");

	std::string str;
	while (std::getline(kernel_file, str)){
        std::stringstream read_val(str);
        double read_val_2;
        read_val >> read_val_2;
        kernel.push_back(read_val_2);
	}

	/****************/
	/* PRINT KERNEL */
	/****************/
	//for(auto i : kernel) std::cout << i << " ";
	//std::cout << std::endl << std::endl;


  /***************/
	/* CONVOLUCION */
	/***************/
  //std::vector<double> v_cov = conv(f, kernel);
  //for(auto i : v_cov) std::cout << i << " ";
  //std::cout << std::endl << std::endl;

	/*********************************/
	/* CONVOLUCION TAMAÑO CORREGUIDO */
	/*********************************/
	//std::vector<double> v_cov2 = std::vector<double>(v_cov.begin(), v_cov.end()-(f.size()-1));
	//for(auto i : v_cov2) std::cout << i << " ";
  //std::cout << std::endl  << std::endl;

	/************************/
	/* CONVOLUCION UN VALOR */
	/************************/
  double result = 0-conv(f, kernel)[0];
  std::cout << "\nCheck value:\n" << result << std::endl;
} 

