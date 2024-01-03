@rem boxes - Command line filter to draw/remove ASCII boxes around text
@rem Copyright (c) 1999-2024 Thomas Jensen and the boxes contributors
@rem
@rem This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
@rem License, version 3, as published by the Free Software Foundation.
@rem This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
@rem warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
@rem details.
@rem You should have received a copy of the GNU General Public License along with this program.
@rem If not, see <https://www.gnu.org/licenses/>.
@rem

call bundle exec jekyll clean
bundle exec jekyll serve --watch --incremental --trace
