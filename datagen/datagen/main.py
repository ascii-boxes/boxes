#
# boxes - Command line filter to draw/remove ASCII boxes around text
# Copyright (c) 1999-2024 Thomas Jensen and the boxes contributors
#
# This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
# License, version 3, as published by the Free Software Foundation.
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
# You should have received a copy of the GNU General Public License along with this program.
# If not, see <https://www.gnu.org/licenses/>.
#

from ansi2html import Ansi2HTMLConverter
from typing import Optional
import click
import itertools
import re
import subprocess
import sys
import yaml


class Quoted(str):
    pass


class Sample(str):
    pass


class Design:
    name: str
    alias: Optional[list[str]]
    designer: Optional[Quoted]
    coder: Optional[Quoted]
    tags: Optional[list[str]]
    sample: Optional[Sample]

    def __init__(self, name: str):
        self.name = name
        self.alias = None
        self.designer = None
        self.coder = None
        self.tags = None
        self.sample = None
    
    def add_alias(self, alias: str) -> None:
        if self.alias is None:
            self.alias = []
        self.alias.append(alias)
    
    def add_tag(self, tag: str) -> None:
        if self.tags is None:
            self.tags = []
        self.tags.append(tag)



def read_boxes_version(executable: str) -> str:
    process = subprocess.run([executable, '-v'], stdout=subprocess.PIPE)
    output = process.stdout.decode('ascii')
    p = re.compile('boxes version ([^ ]+) \(')
    m = p.match(output)
    if m:
        version = m.group(1)
        click.echo('Running boxes v' + version)
        return version
    raise Exception('ERROR: Failed to read boxes version')


def read_design_names(config: str, executable: str) -> list[Design]:
    process = subprocess.run([executable, '-f', config, '-q', '(all)'], stdout=subprocess.PIPE)
    lines = process.stdout.decode('ascii').splitlines()
    result = []
    current = Design('ERROR')
    for line in lines:
        if line.endswith(' (alias)'):
            current.add_alias(line[:-8])
        else:
            current = Design(line)
            result.append(current)
    click.echo(f"Config file contains {len(result)} designs")
    return result


def detect_indent(sample_lines: list[str]) -> int:
    min_indent = sys.maxsize
    for line in sample_lines:
        indent = sum(1 for _ in itertools.takewhile(str.isspace, line))
        if len(line) > 0 and indent < min_indent:
            min_indent = indent
    if min_indent == sys.maxsize:
        min_indent = 0
    return min_indent


def remove_indent(sample_lines: list[str]) -> list[str]:
    indent = detect_indent(sample_lines)
    result = []
    killblank = True
    for line in sample_lines:
        if killblank:
            if len(line) == 0:
                continue
            killblank = False
        if len(line) >= indent:
            result.append(line[indent:])
    return result


def remove_email(attribution: str) -> str:
    return re.sub(re.compile(' <[^>]+>$'), '', attribution)


def color2html(design: Design) -> None:
    if design.sample:
        converter = Ansi2HTMLConverter(
            inline=True,     # do not use templates or CSS, just inline the color styles
            line_wrap=True,  # wrap lines as in input
            output_encoding='utf-8',
            scheme='xterm')
        ansi = design.sample
        html = converter.convert(ansi, full=False)
        design.sample = Sample(html)


def add_design_details(config: str, executable: str, design: Design) -> None:
    process = subprocess.run([executable, '-f', config, '-q', '(undoc)', '-d', design.name, '-l'], stdout=subprocess.PIPE)
    lines = process.stdout.decode('utf-8').splitlines()
    p_orgd = re.compile('^Original Designer:\\s+(.*)')
    p_auth = re.compile('^Author:\\s+(.*)')
    p_tags = re.compile('^Tags:\\s+(.*)')
    sample_reached = False
    sample_lines = []
    for line in lines:
        m = p_orgd.match(line)
        if m:
            design.designer = Quoted(remove_email(m.group(1)))
            continue
        m = p_auth.match(line)
        if m:
            design.coder = Quoted(remove_email(m.group(1)))
            continue
        m = p_tags.match(line)
        if m:
            tags = m.group(1).split(', ')
            tags.sort()
            for tag in tags:
                if tag != 'none':
                    design.add_tag(tag)
            continue
        if line == 'Sample:':
            sample_reached = True
            continue
        if sample_reached:
            sample_lines.append(line.rstrip())
    if len(sample_lines) > 0:
        sample = '\n'.join(remove_indent(sample_lines)).rstrip()
        design.sample = Sample(sample)
    
    if design.tags and 'color' in design.tags:
        color2html(design)


def write_yaml(design_data: list[Design], output: str) -> None:
    def quoted_presenter(dumper, data):
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='\'')

    def block_presenter(dumper, data):
        return dumper.represent_scalar("tag:yaml.org,2002:str", data, style="|")

    yaml.add_representer(Quoted, quoted_presenter)
    yaml.add_representer(Sample, block_presenter)

    yaml_str = yaml.dump(design_data, allow_unicode=True, sort_keys=False)
    yaml_str = yaml_str.replace('!!python/object:datagen.main.Design\n  ', '')
    yaml_str = yaml_str.replace('\n  -', '\n    -')
    yaml_str = re.sub(re.compile('\n  [a-z]+: null\n'), '\n', yaml_str)

    with open(output, 'w', encoding='utf-8') as outfile:
        outfile.write(yaml_str)
    click.echo('Output file created: ' + output)


@click.command()
@click.option('--config', '-f', required=False, default='boxes-config', help='boxes config file to use')
@click.option('--executable', '-x', required=False, default='boxes.exe', help='boxes executable to use')
@click.option('--output', '-o', required=False, default='designs-vx_y_z.yml', help='output file to generate')
@click.version_option()
def main(config: str, executable: str, output: str) -> None:
    bx_version = read_boxes_version(executable)
    design_data = read_design_names(config, executable)
    for design in design_data:
        add_design_details(config, executable, design)
    write_yaml(design_data, output.replace('x_y_z', bx_version.replace('.', '_')))
    click.echo('Done.')
