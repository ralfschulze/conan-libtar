#!/usr/bin/env python
# -*- coding: utf-8 -*-

from conans import ConanFile, tools, AutoToolsBuildEnvironment
import os


class LibtarConan(ConanFile):
    name = "libtar"
    version = "1.2.20"
    homepage = "https://github.com/tklauser/libtar"
    description = "C library for manipulating tar files."
    url = "https://github.com/fogo/conan-libtar"
    author = "fogo <tochaman@gmail.com>"
    license = "COPYRIGHT"
    exports = ['LICENSE.md']
    exports_sources = ['CMakeLists.txt']
    generators = 'cmake'
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False], "withzlib": [True, False]}
    default_options = "shared=False", "fPIC=True", "withzlib=True"
    checksum = "3152fc61cf03c82efbf99645596efdadba297eac3e85a52ae189902a072c9a16"

    def requirements(self):
        if self.options.withzlib:
            self.requires.add("zlib/1.2.11")

    def config_options(self):
        if self.settings.os == "Windows":
            raise OSError("not actively supported for this platform")

    def source(self):
        targz_name = "libtar-v{version}.tar.gz".format(version=self.version)
        tools.download("https://github.com/tklauser/libtar/archive/v{version}.tar.gz".format(version=self.version), targz_name)
        tools.check_sha256(targz_name, self.checksum)
        tools.untargz(targz_name)
        os.unlink(targz_name)

    def build(self):
        name = "libtar-{version}".format(version=self.version)
        install_folder = os.path.join(name, "install")
        with tools.chdir(name):
            args = ["--prefix={}".format(os.path.abspath(install_folder))]
            if not self.options.withzlib:
                args.append("--without-zlib")

            self.run("autoreconf --force --install")
            autotools = AutoToolsBuildEnvironment(self)
            autotools.fpic = self.options.fPIC
            autotools.configure(args=args)
            autotools.make()
            autotools.install()

    def package(self):
        name = "libtar-{version}".format(version=self.version)
        install_folder = os.path.join(name, name, "install")
        self.copy(
            "*.h",
            dst="include",
            src="{basedir}/include".format(basedir=install_folder))
        if not self.options.shared:
            self.copy(
                "*.a",
                dst="lib",
                src="{basedir}/lib".format(basedir=install_folder))
        else:
            self.copy(
                "*.so",
                dst="lib",
                src="{basedir}/lib".format(basedir=install_folder),
                symlinks=True,
            )
            self.copy(
                "*.so.0",
                dst="lib",
                src="{basedir}/lib".format(basedir=install_folder),
                symlinks=True,
            )
            self.copy(
                "*.so.0.0.0",
                dst="lib",
                src="{basedir}/lib".format(basedir=install_folder))

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
