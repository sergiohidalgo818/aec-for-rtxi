
# Active Electrode Compensation (AEC) module for RTXI

This module implemets the AEC method from Brette et al. (2008)

- A RTXI module to inject white noise in the system and obtain the data for the calibration is also included 
- The calibration is done in python, with a code also included. This calibration creates a kernel file
- Finally, the AEC module can be used with the calculated electrode kernel

## How to use it

### Data for train
- Install our white noise module:

```cd white-noise-module-rtxi && sudo make install```

- Insert white noise & read the voltage
- Save the data (recorded voltage & white noise injection)
- Our RTXI workspace is included (white_generator.set)

### Calculating the kernel
- Generate the electrode kernel (**check line 86** to define the trial of the h5 file):

```py aec_train.py -p file.h5 -m intra```

- The c++ convolution code used in RTXI can be tested standalone using:

```g++ -std=c++1y -O3 -Wall -pedantic -pthread convolution_test.cpp && ./a.out```

### Using the AEC module
- The calculated kernel is in:

```aec_kernel.txt```

- **Check line 151** to define your kernel path:

```aec-module-rtxi/aec-module-rtxi.cpp```

- Install our AEC module:

```cd aec-module-rtxi && sudo make install```

- Open the AEC module in RTXI
- Connect AEC module inputs (recorded voltage & current injection) and output (clean voltage)
- Our RTXI workspace is included (aec_test.set)

## Credits
Brette, R., Piwkowska, Z., Monier, C., Rudolph-Lilith, M., Fournier, J., Levy, M., Frégnac, Y., Bal, T., Destexhe, A. (2008). High-resolution intracellular recordings using a real-time computational model of the electrode. Neuron, 59(3), 379-391.
