#  Copyright (c) 2019, Xilinx
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#  1. Redistributions of source code must retain the above copyright notice, this
#     list of conditions and the following disclaimer.
#
#  2. Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#
#  3. Neither the name of the copyright holder nor the names of its
#     contributors may be used to endorse or promote products derived from
#     this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
#  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from setuptools import setup, find_packages
from distutils.dir_util import copy_tree
import os
from pynq.utils import build_py as _build_py


__author__ = "Lucian Petrica"
__copyright__ = "Copyright 2019, Xilinx"


# global variables
module_name = "resnet50_pynq"
data_files = []


def extend_package(path):
    if os.path.isdir(path):
        data_files.extend(
            [os.path.join("..", root, f)
             for root, _, files in os.walk(path) for f in files]
        )
    elif os.path.isfile(path):
        data_files.append(os.path.join("..", path))


class build_py(_build_py):
    """Overload the pynq.utils 'build_py' command (that performs overlay
    download)  to also call the function 'copy_notebooks'.
    """
    def copy_notebooks(self):
        cmd = self.get_finalized_command("build_py")
        for package, src_dir, build_dir, _ in cmd.data_files:
            if "." not in package:  # sub-packages are skipped
                src_folder = os.path.join(os.path.dirname(src_dir), "host")
                dst_folder = os.path.join(build_dir, "notebooks")
                if os.path.isdir(src_folder):
                    copy_tree(src_folder, dst_folder)

    def run(self):
        super().run()
        self.copy_notebooks()


with open("README.md", encoding="utf-8") as fh:
    readme_lines = fh.readlines()
    readme_lines = readme_lines[
        readme_lines.index("## PYNQ quick start\n") + 2:
        readme_lines.index("## Author\n"):
    ]

long_description = ("".join(readme_lines))

extend_package(os.path.join(module_name, "notebooks"))
setup(name=module_name,
      version="1.1",
      description="Quantized dataflow implementation of ResNet50 on Alveo",
      long_description=long_description,
      long_description_content_type="text/markdown",
      author="Lucian Petrica",
      url="https://github.com/Xilinx/ResNet50-PYNQ",
      packages=find_packages(),
      download_url="https://github.com/Xilinx/ResNet50-PYNQ",
      package_data={
          "": data_files,
      },
      python_requires=">=3.5.2",
      # keeping 'setup_requires' only for readability - relying on
      # pyproject.toml and PEP 517/518
      setup_requires=[
          "pynq>=2.5.1"
      ],
      install_requires=[
          "pynq>=2.5.1",
          "jupyter",
          "jupyterlab",
          "plotly",
          "opencv-python",
          "wget"
      ],
      extras_require={
          ':python_version<"3.6"': [
              'matplotlib<3.1',
              'ipython==7.9'
          ],
          ':python_version>="3.6"': [
              'matplotlib'
          ]
      },
      entry_points={
          "pynq.notebooks": [
              "ResNet50 = {}.notebooks".format(module_name)
          ]
      },
      cmdclass={"build_py": build_py},
      license="BSD 3-Clause"
      )
