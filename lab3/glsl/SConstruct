import os
env = Environment(ENV = os.environ)
env.Append(CXXFLAGS=["-g", "-std=c++11", "-Wall", "-Werror"])
env.Append(LIBS=["GL","GLU","glut","GLEW"])
# On the department machines, you may need to uncomment these.
# env.Append(CPPPATH=["/usr/csshare/pkgs/glew-1.13.0/include"])
# env.Append(LIBPATH=["/usr/csshare/pkgs/glew-1.13.0/lib64/"])
# env.Append(RPATH=["/usr/csshare/pkgs/glew-1.13.0/lib64/"])
env.Program("shaders",["main.cpp","application.cpp",
                       "obj.cpp","gl_viewer.cpp","gl_arcball.cpp"])
