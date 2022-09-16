### Data for train
- Using RTXI and our white noise module:
  - Insert white noise and read the voltage
  - Save the data using RTXI

### Data for test
- Using RTXI and the wave generator (¿included?):
  - Insert a periodic signal and read the voltage
  - Save the data in another trial of the same data file

### Calculating the kernel
```py aec_train.py -p file.h5 -m intra```

### Checking c++ convolution
```g++ -std=c++1y -O3 -Wall -pedantic -pthread convolution_test.cpp && ./a.out```

### Use
- The calculated kernel is in ```aec_kernel.txt```
- Move the kernel file to the AEC module folder
- Open the AEC module in RTXI and load the kernel
- Connect AEC module input and output

### Credits
Brette, R., Piwkowska, Z., Monier, C., Rudolph-Lilith, M., Fournier, J., Levy, M., Frégnac, Y., Bal, T., Destexhe, A. (2008). High-resolution intracellular recordings using a real-time computational model of the electrode. Neuron, 59(3), 379-391.
