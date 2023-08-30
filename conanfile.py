from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout

class EnigineConan(ConanFile):
    name = "enigine"
    version = "0.0.1"
    generators = "CMakeDeps", "CMakeToolchain"

    # Optional metadata
    license = "<Put the package license here>"
    author = "Furkan Sarihan fusarihan@gmail.com"
    url = "https://github.com/furkansarihan/enigine"
    description = "3D game engine"

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*"

    def requirements(self):
        self.requires("glfw/3.3.6")
        self.requires("glew/2.2.0")
        self.requires("glm/cci.20220420")
        self.requires("imgui/1.89.4")
        self.requires("assimp/5.2.2")
        self.requires("openal/1.22.2")
        self.requires("bullet3/3.25")
        self.requires("libsndfile/1.2.0")
        self.requires("boost/1.82.0")
        self.requires("eigen/3.4.0")

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["enigine"]
