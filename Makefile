# SSAPT Makefile for manual builds
# Requires Visual Studio Build Tools or MinGW

# Compiler settings
CXX = cl
CXXFLAGS = /EHsc /std:c++17 /O2 /W3
LDFLAGS = /link gdi32.lib d3d9.lib d3d11.lib dxgi.lib

# Targets
all: ssapt.dll ssapt_test.exe

ssapt.dll: driver.cpp dx_hooks.cpp
	$(CXX) /LD $(CXXFLAGS) driver.cpp dx_hooks.cpp $(LDFLAGS) /OUT:ssapt.dll

ssapt_test.exe: test_driver.cpp ssapt.lib
	$(CXX) $(CXXFLAGS) test_driver.cpp /link ssapt.lib gdi32.lib /OUT:ssapt_test.exe

clean:
	del /Q *.dll *.exe *.lib *.exp *.obj *.pdb 2>nul

.PHONY: all clean
