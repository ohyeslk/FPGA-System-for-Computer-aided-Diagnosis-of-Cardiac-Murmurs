FPGA-System-for-Computer-aided-Diagnosis-of-Cardiac-Murmurs
===========================================================
A computer-aided diagnosis (CAD) algorithm was designed and implemented
on an Altera Cyclone II FPGA to detect cardiac murmurs from recorded heart signals.
Training and evaluation data sets were obtained from the online “Classifying Heart
Sounds Challenge” sponsored by PASCAL. The detection algorithm calculates the Low
Energy Rate (LER) from a recorded heart signal and performs a binary classification of
the sample as either normal or murmur. The FPGA system interfaces with a commercial
digital stethoscope to acquire real time data as well as a VGA-compatible monitor for
visualization and metric reporting. In addition, basic I/O was developed for user input to
the system. The result is a complete embedded system capable of acquiring and
analyzing cardiac data in real time to detect heart murmurs.
