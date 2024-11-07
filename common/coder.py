#!/usr/bin/env python3
# -*- coding:utf-8 -*-

# Licensed under the GNU General Public License v. 3 (the "License")
# You may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.gnu.org/licenses/gpl-3.0.html
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Copyright (C) 2022-2024 xqyjlj<xqyjlj@126.com>
#
# @author      xqyjlj
# @file        coder.py
#
# Change Logs:
# Date           Author       Notes
# ------------   ----------   -----------------------------------------------
# 2024-07-11     xqyjlj       initial version
#

import copy
import glob
import hashlib
import importlib.util
import os
import re
import time
import xml.etree.ElementTree as etree
from pathlib import Path
from types import ModuleType

import jinja2
from PySide6.QtCore import QObject, Signal
from loguru import logger

from .project import PROJECT
from .settings import SETTINGS


class Coder(QObject):
    progressUpdated = Signal(str, int, int)

    def __init__(self, parent=None):
        super().__init__(parent=parent)

        self.__contextTable = {}
        self.__filesTable = {}
        self.__coder = None

        self.__coder = self.__loadCoder()
        self.__filesTable = self.__getFilesTable()

    def generate(self):
        if not self.__checkHalFolder():
            return

        outputDir = os.path.dirname(PROJECT.path())

        for path, context in self.dump().items():
            genMd5 = hashlib.md5(context.encode('utf-8')).hexdigest()
            path = f"{outputDir}/{path}"
            if os.path.isfile(path):
                with open(path, "r", encoding='utf-8') as file:
                    fileContext = file.read()
                    fileMd5 = hashlib.md5(fileContext.encode('utf-8')).hexdigest()
            else:
                Path(path).parent.mkdir(parents=True, exist_ok=True)
                fileMd5 = ""
            if genMd5 != fileMd5:
                with open(path, "w", encoding='utf-8') as file:
                    file.write(context)

    def dump(self) -> dict:
        packageFolder = PROJECT.halFolder()

        if len(self.__filesTable) == 0:
            return {}

        data = {
            "author": "csplink coder",
            "version": SETTINGS.VERSION,
            "project": PROJECT.project().origin,
            "time": time.strftime('%Y-%m-%d %H:%M:%S', time.localtime()),
            "year": time.strftime('%Y', time.localtime())
        }

        env = jinja2.Environment(loader=jinja2.FileSystemLoader([f'{SETTINGS.EXE_FOLDER}/resource/templates',
                                                                 f'{packageFolder}/tools/coder/templates']),
                                 line_comment_prefix="//")

        files = glob.glob(f"{packageFolder}/tools/coder/filters/*.py")
        for file in files:
            spec = importlib.util.spec_from_file_location(Path(file).stem, file)
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            functions = [name for name in dir(module) if callable(getattr(module, name))]
            for fun in functions:
                if not fun.startswith("_"):
                    function = getattr(module, fun)
                    env.filters[fun] = function

        for file, info in self.__filesTable.items():
            suffix = Path(file).suffix
            context = self.__render(file, info.get('brief', 'file automatically-generated by tool: [csp]'), env, data)
            if context is not None:
                self.__contextTable[file] = context
            elif suffix == '.uvprojx' or suffix == '.uvproj':
                pass

            # self.dumped.emit(file)

        return self.__contextTable

    def deploy(self):
        if self.__coder is None:
            return

        data = self.__coder.deploy(copy.deepcopy(PROJECT.project().origin), os.path.dirname(PROJECT.path()))
        return data

    def filesList(self) -> list[str]:
        return list(self.__filesTable.keys())

    def __loadCoder(self) -> ModuleType | None:
        if self.__checkHalFolder():
            spec = importlib.util.spec_from_file_location("coder", f'{PROJECT.halFolder()}/tools/coder/coder.py')
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            return module
        return None

    def __getFilesTable(self) -> dict[str, dict[str, str]]:
        if self.__coder is None:
            return {}
        files = self.__coder.files_table(copy.deepcopy(PROJECT.project().origin))
        return files

    def __checkHalFolder(self) -> bool:
        if not os.path.isfile(f'{PROJECT.halFolder()}/tools/coder/coder.py'):
            logger.error(
                f"{PROJECT.halFolder()} is not directory! maybe package({PROJECT.project().gen.hal}) not yet installed.")
            return False
        return True

    def __matchUser(self, path: str, prefix1: str, suffix1: str, prefix2: str, suffix2: str) -> dict:
        code = {}
        if os.path.isfile(path):
            with open(path, "r", encoding='utf-8') as f:
                data = f.read()
                for s in re.findall(f"{prefix1} user code begin (.*), do not change this comment!{suffix1}", data):
                    matcher = f"{prefix1} user code begin {s}, do not change this comment!{suffix1}\n(.*){prefix2} user code end {s}, do not change this comment!{suffix2}"
                    result = str.rstrip(re.findall(matcher, data, re.S)[0])
                    if result:
                        code[s] = str.rstrip(result)
                        # noinspection PyUnresolvedReferences
                        if code[s] != "":
                            code[s] = code[s] + "\n"
                    else:
                        code[s] = ""
        return code

    def __render(self, path: str, brief: str, env: jinja2.Environment, args: dict) -> str | None:
        absPath = f"{PROJECT.folder()}/{path}"
        template = env.get_template(f'{os.path.basename(path)}.j2')
        suffix = Path(absPath).suffix
        if suffix == '.h' or suffix == '.c':
            args["userCode"] = self.__matchUser(absPath, "/\*\*<", " \*/", "/\*\*>", " \*/")
        elif Path(absPath).name == 'xmake.lua':
            args["userCode"] = self.__matchUser(absPath, "----<", "\n", "----<", "\n")
        elif Path(absPath).name == 'CMakeLists.txt':
            args["userCode"] = self.__matchUser(absPath, "##==<", "\n", "##==>", "\n")
        else:
            return None
        context = template.render(args, file=os.path.basename(path), brief=brief)
        context = context.strip() + "\n"

        timePattern = r'\b\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\b'  # YYYY-MM-DD HH:MM:SS
        genMd5 = hashlib.md5(re.sub(timePattern, '', context).encode('utf-8')).hexdigest()
        fileContext = ''
        if os.path.isfile(absPath):
            with open(absPath, "r", encoding='utf-8') as file:
                fileContext = file.read()
                fileMd5 = hashlib.md5(re.sub(timePattern, '', fileContext).encode('utf-8')).hexdigest()
        else:
            fileMd5 = ""

        if genMd5 == fileMd5:
            context = fileContext

        return context

    def __generateMdkArmProject(self, project: dict, path: str, minVersion: str) -> str:
        """
        generate mdk arm
        """
        spec = importlib.util.spec_from_file_location("coder", f'{package_dir}/tools/coder/gen_mdk_arm.py')
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)

        if os.path.isfile(path):
            tree = etree.parse(path)
        else:
            tree = None

        return module.main(copy.deepcopy(project), copy.deepcopy(minVersion), copy.deepcopy(tree))
