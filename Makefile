OPENSIM_INSTALL ?= ~/Desktop/opensim-core/opensim-core-install
RUNTIME_ENV = "LD_LIBRARY_PATH=${OPENSIM_INSTALL}/lib"

.PHONY: object_sizes

object_sizes:
	$(CXX) size_of_objects.cpp \
	-o size_of_objects \
	-I ${OPENSIM_INSTALL}/include \
	-I ${OPENSIM_INSTALL}/include/simbody/ \
	-I ${OPENSIM_INSTALL}/include/OpenSim/ \
	-L ${OPENSIM_INSTALL}/lib/ \
	-losimTools \
	-losimAnalyses \
	-losimActuators \
	-losimSimulation \
	-losimCommon

	LD_LIBRARY_PATH=${OPENSIM_INSTALL}/lib ./size_of_objects
